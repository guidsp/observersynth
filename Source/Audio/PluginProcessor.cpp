#include "PluginProcessor.h"
#include "../gui/PluginEditor.h"

//==============================================================================
ObserverAudioProcessor::ObserverAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    oscA(),
    oscB(),
    filter(),
    treestate(*this, nullptr, "Parameters", makeParams())
#endif
{
    setParamPointers();
    setModDestPointers();

    //Set up queue
    setVoices(maximumVoices);

    voiceQueue.reserve(maximumVoices);

    //Set voices
    Voice defaultVoice = { Voice::VoiceState::free, 69 };
    for (int i = 0; i < maximumVoices; i++)
        voice[i] = defaultVoice;

    activeVoices = 0; activeEnvelopes = 0;

    for (int mod = 0; mod < nOfModulators; mod++)
        for (int v = 0; v < maximumVoices; v++)
            _voiceOpacity_[mod][v].set(0.0f);

    setInitSettings();
}

ObserverAudioProcessor::~ObserverAudioProcessor() {
};

//==============================================================================
const juce::String ObserverAudioProcessor::getName() const {
    return JucePlugin_Name;
};

bool ObserverAudioProcessor::acceptsMidi() const {
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
};

bool ObserverAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
};

bool ObserverAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
};

double ObserverAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ObserverAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ObserverAudioProcessor::getCurrentProgram() {
    return 0;
}

void ObserverAudioProcessor::setCurrentProgram (int index) {
}

const juce::String ObserverAudioProcessor::getProgramName (int index) {
    return {};
}

void ObserverAudioProcessor::changeProgramName (int index, const juce::String& newName) {
}

//==============================================================================
void ObserverAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    //TODO Need to move knobs as well when this happens
    //Set up oscs
    WT_Osc::setSamplerate(sampleRate);
    
    //Set up modulators and poles and link them
    for (int i = 0; i < nOfModulators; i++) {
        modulator[i].setup(sampleRate, envelopeTransitTimeInMs);
        pole[i].setup(sampleRate, poleUpdateFreq);
        modulator[i].pole = &pole[i];
    }

    SVF::setup(sampleRate);
    
    //Glide slide
    glideSpeed = glide->get() + envelopeTransitTimeInMs + 0.01f;

    LineSlide::setSampleRate(sampleRate);

    for (int i = 0; i < maximumVoices; i++) {
        filter[i].setMode(filterState.value(i));
        notePitchSlide[i].setSampleRate(sampleRate);
        notePitchSlide[i].setTime(glideSpeed);
    }

    //Set up slide length for parameters
    for (int i = 0; i < nOfModulationDestinations; i++) {
        modDest[i]->slidingParamVal.setSampleRate(sampleRate);

        if (modDest[i] != nullptr)
            modDest[i]->slidingParamVal.nSteps = samplesPerBlock; //No need for duplicating nSteps because value() function is being called 
        }                                                         //once per processBlock (both channels are processed at once)

    for (int i = 0; i < nOfSmoothedParams; i++) {
        parameter[i]->slidingParamVal.setSampleRate(sampleRate);

        if (parameter[i] != nullptr)
            parameter[i]->slidingParamVal.nSteps = samplesPerBlock;
    }

    setVoices(voices->get());

    //Updates on sampling rate change
    for (int v = 0; v < maximumVoices; v++) {
        oscA.setNote(v, prevNoteA[v]);
        oscB.setNote(v, prevNoteB[v]);
        filter[v].setParams(prevCutoff[v], prevQ[v]);
        filter[v].setMode(prevState[v]);
    }
}

void ObserverAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ObserverAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ObserverAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    buffer.clear();
    int currentSample = 0;

    //Get atomic pole position that may have changed from GUI
    for (int i = 0; i < nOfModulators; i++) {
        modulator[i].pole->polePos.setXY(_polePosX_[i].get(), _polePosY_[i].get());
    }

    glideSpeed = glide->get() + envelopeTransitTimeInMs + 0.01f;

    voices_ = voices->get(); 
    setVoices(voices_);

    octaveAoffset = 12 * Octave_A->get();
    octaveBoffset = 12 * Octave_B->get();

    for (int i = 0; i < nOfModulators; i++) {
        modulator[i].setParams(M_Attack[i]->get(), M_Decay[i]->get(), M_Sustain[i]->get(), M_Release[i]->get());
        modulator[i].veloAmt = M_Velocity[i]->get();
    }

    for (const juce::MidiMessageMetadata metadata : midiMessages) {
        const juce::MidiMessage m = metadata.getMessage();
        //const int messagePosition = static_cast<int>(m.getTimeStamp());
        const int messagePosition = static_cast<int>(m.getTimeStamp());

        render(buffer, currentSample, messagePosition);
        currentSample = messagePosition;
        processMidi(m);
    }

    render(buffer, currentSample, buffer.getNumSamples());

    buffer.applyGainRamp(0, buffer.getNumSamples(), prevGain, output->get());
    prevGain = output->get();

    //Set atomics
    for (int i = 0; i < nOfModulators; i++) {
        _polePosX_[i].set(pole[i].polePos.getX()); _polePosY_[i].set(pole[i].polePos.getY());

        if (modulator[i].depth != 0.0f) {
            for (int j = 0; j < maximumVoices; j++) {
                _voicePosX_[i][j].set(pole[i].voicePos[j].getX()); _voicePosY_[i][j].set(pole[i].voicePos[j].getY());

                _voiceOpacity_[i][j].set(modulator[i].value_[j]);
            }
        }
        else {
            for (int j = 0; j < activeEnvelopes; j++) {
                auto v_ = voiceQueue[j];
                _voiceOpacity_[i][v_].set(0.0f);
            }
        }
    }
};

void ObserverAudioProcessor::render(juce::AudioBuffer<float>& audiobuffer, int start, int end) {
    float* channelData[2] = { audiobuffer.getWritePointer(0) , audiobuffer.getWritePointer(1) };

    for (int sample = start; sample < end; sample++) {
        for (int v_ = 0; v_ < activeEnvelopes; v_++) {
            auto v = voiceQueue[v_];

            updateVoice(v);

            auto shape1val = Shape1.value(v); auto shape2val = Shape2.value(v); auto phaseval = Phase.value(v);

            oscA.shape[v] = juce::jlimit(0.0f, 3.0f, Shape1_A.value(v) + shape1val);
            oscA.shape2[v] = juce::jlimit(0.0f, 1.0f, Shape2_A.value(v) + shape2val);
            oscA.phaseOffset[v] = juce::jlimit(0.0f, 1.0f, Phase_A.value(v) + phaseval);

            oscB.shape[v] = juce::jlimit(0.0f, 3.0f, Shape1_B.value(v) + shape1val);
            oscB.shape2[v] = juce::jlimit(0.0f, 1.0f, Shape2_B.value(v) + shape2val);
            oscB.phaseOffset[v] = juce::jlimit(0.0f, 1.0f, Phase_B.value(v) + phaseval);

            audioSignal = { 0.0f, 0.0f };

            auto oscMix_val = OscMix.value(v);
            auto VCA_ = VCA.value(v);
            auto pan = Pan.value(v);

            {
                auto panA = Pan_A.value(v);
                auto VCAA = VCA_A.value(v);

                if (oscMix_val < 1.0f) {
                    auto oscout = oscA.output(v);
                    auto oscPanVal = jlimit(0.0f, 1.0f, panA + pan);
                    auto Amix = OscMix.max - oscMix_val;

                    SampleData panStereo = { 1.0f - oscPanVal, oscPanVal };

                    auto VCA_osc_val = VCA_ * VCAA;
                    auto osc = oscout * VCA_osc_val * Amix;
                    audioSignal.l += osc * panStereo.l; audioSignal.r += osc * panStereo.r;
                }
            }

            {
                auto panB = Pan_B.value(v);
                auto VCAB = VCA_B.value(v);

                if (oscMix_val > 0.0f) {
                    auto oscout = oscB.output(v);
                    auto oscPanVal = jlimit(0.0f, 1.0f, panB + pan);
                    auto Bmix = oscMix_val;

                    SampleData panStereo = { 1.0f - oscPanVal, oscPanVal };

                    auto VCA_osc_val = VCA_ * VCAB;
                    auto osc = oscout * VCA_osc_val * Bmix;
                    audioSignal.l += osc * panStereo.l; audioSignal.r += osc * panStereo.r;
                }
            }
           
            auto cutFreq = filterCutoff.value(v);
            auto q = filterQ.value(v);

            if (cutFreq != prevCutoff[v] || q != prevQ[v]) {
                filter[v].setParams(cutFreq, q);
                prevCutoff[v] = cutFreq;
                prevQ[v] = q;
            }

            auto filterState_ = filterState.value(v);

            if (filterState_ != prevState[v]) {
                filter[v].setMode(filterState_);
                prevState[v] = filterState_;
            }
            
            filter[v].process(audioSignal);
            
            channelData[0][sample] += audioSignal.l; channelData[1][sample] += audioSignal.r;
        }
    }
};

void ObserverAudioProcessor::processMidi(const juce::MidiMessage& message) {
    if (message.isNoteOn()) {
        if (activeEnvelopes < voices_) {
            //Take free voice
            auto index = voiceQueue[activeEnvelopes];

            for (int mod = 0; mod < nOfModulators; mod++) {
                //Trigger envelope
                modulator[mod].envelope[index].noteOn(modulator[mod].veloAmt * (message.getFloatVelocity() - 1.0f) + 1.0f);

                //Reset pole voice
                pole[mod].clock[index] = 0.0f;
                pole[mod].voiceV[index].setXY(0.0f, 0.0f);
                pole[mod].voicePos[index].setXY(pole[mod].polePos.getX(), pole[mod].polePos.getY());
            }

            //Relocate voice to start
            voiceQueue.erase(voiceQueue.begin() + activeEnvelopes); voiceQueue.insert(voiceQueue.begin(), index);
            voice[voiceQueue[0]].state = Voice::VoiceState::busy;

            //Set note
            voice[index].note = message.getNoteNumber(); 
            notePitchSlide[index].setTime(glideSpeed);
            notePitchSlide[index].set(voice[index].note);

            //Reset oscillator phase
            oscA.phase[index] = 0.0f; oscB.phase[index] = 0.0f;

            activeVoices++; activeEnvelopes++;
        }
        else if (activeVoices < voices_) {
            //Steal a releasing voice
            auto index = voiceQueue[voices_ - 1];

            for (int mod = 0; mod < nOfModulators; mod++) {
                modulator[mod].envelope[index].noteOn(modulator[mod].veloAmt * (message.getFloatVelocity() - 1.0f) + 1.0f);

                //Reset pole voice if retrigger is on
                if (static_cast<int>(M_Retrigger[mod]->get())) {
                    pole[mod].clock[index] = 0.0f;
                    pole[mod].voiceV[index].setXY(0.0f, 0.0f);
                    pole[mod].voicePos[index].setXY(pole[mod].polePos.getX(), pole[mod].polePos.getY());
                }
            }

            voiceQueue.erase(voiceQueue.begin() + (voices_ - 1));
            voiceQueue.insert(voiceQueue.begin(), index);

            voice[voiceQueue[0]].state = Voice::VoiceState::busy;

            voice[index].note = message.getNoteNumber();
            notePitchSlide[index].setTime(glideSpeed);
            notePitchSlide[index].set(voice[index].note);

            activeVoices++;
        }
        else {
            //Steal oldest voice
            int oldestIndex = voiceQueue[voices_ - 1];

            for (int mod = 0; mod < nOfModulators; mod++) {
                modulator[mod].envelope[oldestIndex].noteOn(modulator[mod].veloAmt * (message.getFloatVelocity() - 1.0f) + 1.0f);

                if (static_cast<int>(M_Retrigger[mod]->get())) {
                    pole[mod].clock[oldestIndex] = 0.0f;
                    pole[mod].voiceV[oldestIndex].setXY(0.0f, 0.0f);
                    pole[mod].voicePos[oldestIndex].setXY(pole[mod].polePos.getX(), pole[mod].polePos.getY());
                }
            }

            voice[oldestIndex].note = message.getNoteNumber();

            voiceQueue.erase(voiceQueue.begin() + (voices_ - 1));
            voiceQueue.insert(voiceQueue.begin(), oldestIndex);

            notePitchSlide[oldestIndex].setTime(glideSpeed);
            notePitchSlide[oldestIndex].set(voice[oldestIndex].note);
        }
    }
    else if (message.isNoteOff()) {
        for (int v = 0; v < voices_; v++) {
            auto index = voiceQueue[v];
            //Look for the note that was released
            if (voice[index].note == message.getNoteNumber() && voice[index].state == Voice::VoiceState::busy) {
                for (int env = 0; env < nOfModulators; env++)
                    modulator[env].envelope[index].noteOff();

                activeVoices -= 1;

                voiceQueue.erase(voiceQueue.begin() + v); //Erase released voice and place at start of releasing voices group
                voiceQueue.insert(voiceQueue.begin() + activeVoices, index);

                voice[index].state = Voice::VoiceState::releasing;
            }
        }
    }
    else if (message.isPitchWheel()) {
        //0 - 8192 - 16383
        pitchWheel = pitchWheelRange * ((static_cast<float>(message.getPitchWheelValue()) - 8192.0f) / 16383.0f);
    }
    //else if (message.isController() && message.getControllerNumber() == 1) {
        //0 - 127
        //mod = message.getControllerValue();
    //}
};

void ObserverAudioProcessor::updateVoice(const int& v) {
    auto freeEnvelope = false;

    if ((voice[v].state != Voice::VoiceState::free)  && (vcaModsSize > 0)) {
        auto isVCAmodulatorOff = 0; 

        for (int mod = 0; mod < vcaModsSize; mod++) { //Free voice
            isVCAmodulatorOff += static_cast<int>(VCA.modSources[mod]->envelope[v].state);

            if (VCA.modSources[mod]->envelope[v].state == Obs_Envelope::State::_start) { //NOTE: This only needs to happen once
                oscA.phase[v] = oscB.phase[v] = 0.0f;
            }
        }

        if (isVCAmodulatorOff == 0) {
            for (int mod = 0; mod < nOfModulators; mod++) //Kill envelopes for voices that have just turned off
                modulator[mod].envelope[v].kill();

            voice[v].state = Voice::VoiceState::free;
            freeEnvelope = true;
        }
    }

    activeEnvelopes -= freeEnvelope;

    auto pitch_ = Pitch.value(v);
    auto pitchA_ = notePitchSlide[v].getVal() + pitch_ + pitchWheel + octaveAoffset + Tune_A.getValue() + Pitch_A.value(v);
    auto pitchB_ = notePitchSlide[v].getVal() + pitch_ + pitchWheel + octaveBoffset + Tune_B.getValue() + Pitch_B.value(v);

    if (pitchA_ != prevNoteA[v]) {
        prevNoteA[v] = pitchA_;
        oscA.setNote(v, pitchA_);
    }

    if (pitchB_ != prevNoteB[v]) {
        prevNoteB[v] = pitchB_;
        oscB.setNote(v, pitchB_);
    }

    for (int i = 0; i < nOfModulators; i++) {
        modulator[i].depth = M_Depth[i].getValue();

        if (modulator[i].depth != 0.0f) {
            modulator[i].pole->clockSpeed[v] = M_Speed[i].value(v);
            modulator[i].pole->radius[v] = M_Radius[i].value(v);
            modulator[i].pole->output[v].setTime(M_Soft[i].value(v));
        }
    }
}

//==============================================================================
bool ObserverAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
};

juce::AudioProcessorEditor* ObserverAudioProcessor::createEditor() {
    return new ObserverAudioProcessorEditor(*this);
};

//==============================================================================
void ObserverAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    //Set params
    auto state = treestate.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    juce::MemoryBlock audioData;

    MemoryOutputStream tempStream;
    auto temp = imageData.toBase64Encoding();
    //String temp = ""; //Reset to default image
    
    xml->setAttribute("Image", temp);
    xml->setAttribute("Pole1x", _polePosX_[0].get());
    xml->setAttribute("Pole1y", _polePosY_[0].get());
    xml->setAttribute("Pole2x", _polePosX_[1].get());
    xml->setAttribute("Pole2y", _polePosY_[1].get());
    xml->setAttribute("Pole3x", _polePosX_[2].get());
    xml->setAttribute("Pole3y", _polePosY_[2].get());
    xml->setAttribute("Pole4x", _polePosX_[3].get());
    xml->setAttribute("Pole4y", _polePosY_[3].get());
    copyXmlToBinary(*xml, destData);
};

void ObserverAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    //Get params
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml.get() != nullptr) {
        if (xml->hasTagName(treestate.state.getType())) {
            treestate.replaceState(juce::ValueTree::fromXml(*xml));
            if (xml->hasAttribute("Image")) {
                imageData.fromBase64Encoding(xml->getStringAttribute("Image"));

                if (!imageData.isEmpty()) {
                    MemoryInputStream tempStream(imageData, false);
                    observerImg = ImageFileFormat::loadFrom(tempStream);
                }
                else {
                    observerImg = juce::ImageCache::getFromMemory(BinaryData::defaultObserverImg_jpg, BinaryData::defaultObserverImg_jpgSize);
                }

                if constexpr(resetPoles) {
                    _polePosX_[0].set(0.2f); _polePosY_[0].set(0.2f);
                    _polePosX_[1].set(0.8f); _polePosY_[1].set(0.2f);
                    _polePosX_[2].set(0.2f); _polePosY_[2].set(0.8f);
                    _polePosX_[3].set(0.8f); _polePosY_[3].set(0.8f);
                }
                else {
                    _polePosX_[0].set(xml->getStringAttribute("Pole1x").getFloatValue()); _polePosY_[0].set(xml->getStringAttribute("Pole1y").getFloatValue());
                    _polePosX_[1].set(xml->getStringAttribute("Pole2x").getFloatValue()); _polePosY_[1].set(xml->getStringAttribute("Pole2y").getFloatValue());
                    _polePosX_[2].set(xml->getStringAttribute("Pole3x").getFloatValue()); _polePosY_[2].set(xml->getStringAttribute("Pole3y").getFloatValue());
                    _polePosX_[3].set(xml->getStringAttribute("Pole4x").getFloatValue()); _polePosY_[3].set(xml->getStringAttribute("Pole4y").getFloatValue());
                }
            }
            else {
                observerImg = juce::ImageCache::getFromMemory(BinaryData::defaultObserverImg_jpg, BinaryData::defaultObserverImg_jpgSize);

                _polePosX_[0].set(0.2f); _polePosY_[0].set(0.2f);
                _polePosX_[1].set(0.8f); _polePosY_[1].set(0.2f);
                _polePosX_[2].set(0.2f); _polePosY_[2].set(0.8f);
                _polePosX_[3].set(0.8f); _polePosY_[3].set(0.8f);
            }

            writeImgToArray(observerImg);
        }
    }
}

void ObserverAudioProcessor::setInitSettings() {
    observerImg = juce::ImageCache::getFromMemory(BinaryData::defaultObserverImg_jpg, BinaryData::defaultObserverImg_jpgSize);
    writeImgToArray(observerImg);

    _polePosX_[0].set(0.2f); _polePosY_[0].set(0.2f);
    _polePosX_[1].set(0.8f); _polePosY_[1].set(0.2f);
    _polePosX_[2].set(0.2f); _polePosY_[2].set(0.8f);
    _polePosX_[3].set(0.8f); _polePosY_[3].set(0.8f);

    M_Depth[0].parameter->setValueNotifyingHost(1.0f);
}

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new ObserverAudioProcessor();
};

void ObserverAudioProcessor::setVoices(const int& nvoices) {
    if (nvoices < pastnVoices) { //If new polyphony is lower
        for (int v = pastnVoices - 1; v >= 0; v--) {
            auto index = voiceQueue[v];

            if (index >= nvoices) { //Get rid of highest index voices
                voiceQueue.erase(voiceQueue.begin() + v);

                if (voice[index].state != Voice::VoiceState::free) { //Reduce activeVoices if an active voice was deleted
                    if (voice[index].state == Voice::VoiceState::busy) {
                        activeVoices--;
                    }

                    voice[index].state = Voice::VoiceState::free;
                }

                for (int env = 0; env < nOfModulators; env++) {
                    if (modulator[env].envelope[index].state != Obs_Envelope::State::_idle) { //If any envelope is active
                        //Kill envelope of that voice in every modulator, set atomic opacity to 0 and reduce active envelope count
                        for (int env_ = env; env_ < nOfModulators; env_++) { 
                            modulator[env_].envelope[index].kill();
                            _voiceOpacity_[env_][index].set(modulator[env_].value(index));
                        }
                     
                        //activeEnvelopes = juce::jlimit(0, voices_, activeEnvelopes - 1);
                        activeEnvelopes = jmax(0, activeEnvelopes - 1);
                        break;
                    }
                }
            }
        }
    }
    else if (nvoices > pastnVoices) { //If new polyphony is higher
        for (int v = pastnVoices; v < nvoices; v++) //Add voices
            voiceQueue.push_back(v);
    }

    pastnVoices = nvoices;
};

void ObserverAudioProcessor::writeImgToArray(const juce::Image& imageToWrite) {
    this->suspendProcessing(true);

    juce::Image imageToWrite_ = imageToWrite.rescaled(517, 335);

    for (int x = 0; x < observerWidth; x++) {
        for (int y = 0; y < observerHeight; y++) {
            for (int mod = 0; mod < nOfModulators; mod++)
                pole[mod].imgVal[x][y] = (imageToWrite_.getPixelAt(x, y)).getPerceivedBrightness();
        }
    }

    this->suspendProcessing(false);
}

juce::AudioProcessorValueTreeState::ParameterLayout ObserverAudioProcessor::makeParams() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    //Settings
    params.push_back(std::make_unique<juce::AudioParameterInt>("VOICES", "Voices", 1, maximumVoices, 4));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("GLIDE", "Glide", juce::NormalisableRange<float>(0.0f, 8000.f, 1.0f, 0.4f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OUTPUT", "Output", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0f), 0.65f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSCMIX", "OscMix", 0.0f, 1.0f, 0.5f));

    //OSC A
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC_A_TUNE", "OscA_Tune", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterInt>("OSC_A_OCTAVE", "OscA_Octave", -3, 3, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC_A_SHAPE1", "OscA_Shape1", juce::NormalisableRange<float>(0.0f, 3.0f, 0.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC_A_SHAPE2", "OscA_Shape2", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC_A_PHASE", "OscA_Phase", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC_A_PAN", "OscA_Pan", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0f), 0.5f));

    //OSC B
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC_B_TUNE", "OscB_Tune", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterInt>("OSC_B_OCTAVE", "OscB_Octave", -3, 3, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC_B_SHAPE1", "OscB_Shape1", juce::NormalisableRange<float>(0.0f, 3.0f, 0.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC_B_SHAPE2", "OscB_Shape2", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC_B_PHASE", "OscB_Phase", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC_B_PAN", "OscB_Pan", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0f), 0.5f));

    //FILTER
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTER_CUTOFF", "Filter1_Cutoff", juce::NormalisableRange<float>(20.0f, 20000.0f, 0.0f, 0.3f), 10000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTER_Q", "Filter1_Q", juce::NormalisableRange<float>(0.1f, 15.0f, 0.0f, 0.5f), 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTER_STATE", "Filter1_State", 0.0f, 2.0f, 0.0f));

    //OBSERVERS
    //1
    params.push_back(std::make_unique<juce::AudioParameterInt>("M1_DESTINATION", "M1_Destination", 0, nOfModulationDestinations - 1, 4));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("M1_RADIUS", "M1_Radius", juce::NormalisableRange<float>(0.0f, 0.5f, 0.0f, 0.6f), 0.05f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M1_SOFT", "M1_Soft", juce::NormalisableRange<float>(1.0f, 2000.0f, 0.0f, 0.5f), 60.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M1_SPEED", "M1_Speed", juce::NormalisableRange<float>(-8.0f, 8.0f, 0.0f, 0.5f, true), 0.2f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("M1_RETRIGGER", "M1_Retrigger", false));
    //Envelope
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M1_VELOCITY", "M1_Velocity", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M1_DEPTH", "M1_Depth", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.0f, 0.3f, true), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M1_ATTACK", "M1_Attack", juce::NormalisableRange<float>(0.0f, 10.0f, 0.0f, 0.25f), 0.01f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M1_DECAY", "M1_Decay", juce::NormalisableRange<float>(0.0f, 10.0f, 0.0f, 0.25f), 0.2f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M1_SUSTAIN", "M1_Sustain", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0f), 0.8f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M1_RELEASE", "M1_Release", juce::NormalisableRange<float>(0.0f, 25.0f, 0.0f, 0.4f), 1.0f));

    //2
    params.push_back(std::make_unique<juce::AudioParameterInt>("M2_DESTINATION", "M2_Destination", 0, nOfModulationDestinations - 1, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("M2_RADIUS", "M2_Radius", juce::NormalisableRange<float>(0.0f, 0.5f, 0.0f, 0.6f), 0.05f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M2_SOFT", "M2_Soft", juce::NormalisableRange<float>(1.0f, 2000.0f, 0.0f, 0.5f), 60.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M2_SPEED", "M2_Speed", juce::NormalisableRange<float>(-8.0f, 8.0f, 0.0f, 0.5f, true), -0.2f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("M2_RETRIGGER", "M2_Retrigger", false));
    //Envelope
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M2_VELOCITY", "M2_Velocity", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M2_DEPTH", "M2_Depth", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.0f, 0.3f, true), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M2_ATTACK", "M2_Attack", juce::NormalisableRange<float>(0.0f, 10.0f, 0.0f, 0.25f), 0.01f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M2_DECAY", "M2_Decay", juce::NormalisableRange<float>(0.0f, 10.0f, 0.0f, 0.25f), 0.2f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M2_SUSTAIN", "M2_Sustain", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0f), 0.8f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M2_RELEASE", "M2_Release", juce::NormalisableRange<float>(0.0f, 25.0f, 0.0f, 0.4f), 1.0f));

    //3
    params.push_back(std::make_unique<juce::AudioParameterInt>("M3_DESTINATION", "M3_Destination", 0, nOfModulationDestinations - 1, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("M3_RADIUS", "M3_Radius", juce::NormalisableRange<float>(0.0f, 0.5f, 0.0f, 0.6f), 0.05f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M3_SOFT", "M3_Soft", juce::NormalisableRange<float>(1.0f, 2000.0f, 0.0f, 0.5f), 60.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M3_SPEED", "M3_Speed", juce::NormalisableRange<float>(-8.0f, 8.0f, 0.0f, 0.5f, true), 0.2f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("M3_RETRIGGER", "M3_Retrigger", false));
    //Envelope
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M3_VELOCITY", "M3_Velocity", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M3_DEPTH", "M3_Depth", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.0f, 0.3f, true), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M3_ATTACK", "M3_Attack", juce::NormalisableRange<float>(0.0f, 10.0f, 0.0f, 0.25f), 0.01f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M3_DECAY", "M3_Decay", juce::NormalisableRange<float>(0.0f, 10.0f, 0.0f, 0.25f), 0.2f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M3_SUSTAIN", "M3_Sustain", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0f), 0.8f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M3_RELEASE", "M3_Release", juce::NormalisableRange<float>(0.0f, 25.0f, 0.0f, 0.4f), 1.0f));

    //4
    params.push_back(std::make_unique<juce::AudioParameterInt>("M4_DESTINATION", "M4_Destination", 0, nOfModulationDestinations - 1, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("M4_RADIUS", "M4_Radius", juce::NormalisableRange<float>(0.0f, 0.5f, 0.0f, 0.6f), 0.05f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M4_SOFT", "M4_Soft", juce::NormalisableRange<float>(1.0f, 2000.0f, 0.0f, 0.5f), 60.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M4_SPEED", "M4_Speed", juce::NormalisableRange<float>(-8.0f, 8.0f, 0.0f, 0.5f, true), -0.2f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("M4_RETRIGGER", "M4_Retrigger", false));
    //Envelope
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M4_VELOCITY", "M4_Velocity", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M4_DEPTH", "M4_Depth", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.0f, 0.3f, true), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M4_ATTACK", "M4_Attack", juce::NormalisableRange<float>(0.0f, 10.0f, 0.0f, 0.25f), 0.01f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M4_DECAY", "M4_Decay", juce::NormalisableRange<float>(0.0f, 10.0f, 0.0f, 0.25f), 0.2f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M4_SUSTAIN", "M4_Sustain", juce::NormalisableRange<float>(0.0f, 1.0f, 0.0f), 0.8f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("M4_RELEASE", "M4_Release", juce::NormalisableRange<float>(0.0f, 25.0f, 0.0f, 0.4f), 1.0f));

    return { params.begin(), params.end() };
};

void ObserverAudioProcessor::setParamPointers() {
    voices = (dynamic_cast<juce::AudioParameterInt*>(treestate.getParameter("VOICES")));

    glide = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("GLIDE")));

    output = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("OUTPUT")));

    Tune_A.parameter = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("OSC_A_TUNE")));
    parameter[0] = &Tune_A;

    Octave_A = (dynamic_cast<juce::AudioParameterInt*>(treestate.getParameter("OSC_A_OCTAVE")));

    Tune_B.parameter = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("OSC_B_TUNE")));
    parameter[1] = &Tune_B;

    Octave_B = (dynamic_cast<juce::AudioParameterInt*>(treestate.getParameter("OSC_B_OCTAVE")));

    M_Destination[0] = (dynamic_cast<juce::AudioParameterInt*>(treestate.getParameter("M1_DESTINATION")));

    M_Velocity[0] = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M1_VELOCITY")));

    M_Depth[0].parameter = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M1_DEPTH")));
    parameter[2] = &M_Depth[0];

    M_Retrigger[0] = (dynamic_cast<juce::AudioParameterBool*>(treestate.getParameter("M1_RETRIGGER")));

    M_Attack[0] = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M1_ATTACK")));

    M_Decay[0] = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M1_DECAY")));

    M_Sustain[0] = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M1_SUSTAIN")));

    M_Release[0] = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M1_RELEASE")));

    M_Destination[1] = (dynamic_cast<juce::AudioParameterInt*>(treestate.getParameter("M2_DESTINATION")));

    M_Velocity[1] = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M2_VELOCITY")));

    M_Depth[1].parameter = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M2_DEPTH")));
    parameter[3] = &M_Depth[1];

    M_Retrigger[1] = (dynamic_cast<juce::AudioParameterBool*>(treestate.getParameter("M2_RETRIGGER")));

    M_Attack[1] = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M2_ATTACK")));

    M_Decay[1] = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M2_DECAY")));

    M_Sustain[1] = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M2_SUSTAIN")));

    M_Release[1] = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M2_RELEASE")));

    M_Destination[2] = (dynamic_cast<juce::AudioParameterInt*>(treestate.getParameter("M3_DESTINATION")));

    M_Velocity[2] = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M3_VELOCITY")));

    M_Depth[2].parameter = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M3_DEPTH")));
    parameter[4] = &M_Depth[2];

    M_Retrigger[2] = (dynamic_cast<juce::AudioParameterBool*>(treestate.getParameter("M3_RETRIGGER")));

    M_Attack[2] = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M3_ATTACK")));

    M_Decay[2] = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M3_DECAY")));

    M_Sustain[2] = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M3_SUSTAIN")));

    M_Release[2] = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M3_RELEASE")));

    M_Destination[3] = (dynamic_cast<juce::AudioParameterInt*>(treestate.getParameter("M4_DESTINATION")));

    M_Velocity[3] = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M4_VELOCITY")));

    M_Depth[3].parameter = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M4_DEPTH")));
    parameter[5] = &M_Depth[3];

    M_Retrigger[3] = (dynamic_cast<juce::AudioParameterBool*>(treestate.getParameter("M4_RETRIGGER")));

    M_Attack[3] = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M4_ATTACK")));

    M_Decay[3] = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M4_DECAY")));

    M_Sustain[3] = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M4_SUSTAIN")));

    M_Release[3] = (dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M4_RELEASE")));
};

void ObserverAudioProcessor::setModDestPointers() {
    modDest[0] = nullptr; //First option is none - no modulation destination

    filterCutoff.parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("FILTER_CUTOFF"));
    filterCutoff.setRange(treestate.getParameter("FILTER_CUTOFF")->getNormalisableRange().start, treestate.getParameter("FILTER_CUTOFF")->getNormalisableRange().end);
    modDest[1] = &filterCutoff;

    filterQ.parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("FILTER_Q"));
    filterQ.setRange(treestate.getParameter("FILTER_Q")->getNormalisableRange().start, treestate.getParameter("FILTER_Q")->getNormalisableRange().end);
    modDest[2] = &filterQ;

    filterState.parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("FILTER_STATE"));
    filterState.setRange(treestate.getParameter("FILTER_STATE")->getNormalisableRange().start, treestate.getParameter("FILTER_STATE")->getNormalisableRange().end);
    modDest[3] = &filterState;

    VCA.setRange(0.0f, 1.0f);
    VCA.defaultValue = 1.0f;
    modDest[4] = &VCA;

    VCA_A.setRange(0.0f, 1.0f);
    VCA_A.defaultValue = 1.0f;
    modDest[5] = &VCA_A;

    VCA_B.setRange(0.0f, 1.0f);
    VCA_B.defaultValue = 1.0f;
    modDest[6] = &VCA_B;

    Shape1.setRange(treestate.getParameter("OSC_A_SHAPE1")->getNormalisableRange().start, treestate.getParameter("OSC_A_SHAPE1")->getNormalisableRange().end);
    modDest[7] = &Shape1;

    Shape1_A.parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("OSC_A_SHAPE1"));
    Shape1_A.setRange(treestate.getParameter("OSC_A_SHAPE1")->getNormalisableRange().start, treestate.getParameter("OSC_A_SHAPE1")->getNormalisableRange().end);
    modDest[8] = &Shape1_A;

    Shape1_B.parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("OSC_B_SHAPE1"));
    Shape1_B.setRange(treestate.getParameter("OSC_B_SHAPE1")->getNormalisableRange().start, treestate.getParameter("OSC_B_SHAPE1")->getNormalisableRange().end);
    modDest[9] = &Shape1_B;

    Shape2.setRange(treestate.getParameter("OSC_A_SHAPE2")->getNormalisableRange().start, treestate.getParameter("OSC_A_SHAPE2")->getNormalisableRange().end);
    modDest[10] = &Shape2;

    Shape2_A.parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("OSC_A_SHAPE2"));
    Shape2_A.setRange(treestate.getParameter("OSC_A_SHAPE2")->getNormalisableRange().start, treestate.getParameter("OSC_A_SHAPE2")->getNormalisableRange().end);
    modDest[11] = &Shape2_A;

    Shape2_B.parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("OSC_B_SHAPE2"));
    Shape2_B.setRange(treestate.getParameter("OSC_B_SHAPE2")->getNormalisableRange().start, treestate.getParameter("OSC_B_SHAPE2")->getNormalisableRange().end);
    modDest[12] = &Shape2_B;

    Pan.setRange(treestate.getParameter("OSC_A_PAN")->getNormalisableRange().start, treestate.getParameter("OSC_A_PAN")->getNormalisableRange().end);
    modDest[13] = &Pan;

    Pan_A.parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("OSC_A_PAN"));
    Pan_A.setRange(treestate.getParameter("OSC_A_PAN")->getNormalisableRange().start, treestate.getParameter("OSC_A_PAN")->getNormalisableRange().end);
    modDest[14] = &Pan_A;

    Pan_B.parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("OSC_B_PAN"));
    Pan_B.setRange(treestate.getParameter("OSC_B_PAN")->getNormalisableRange().start, treestate.getParameter("OSC_B_PAN")->getNormalisableRange().end);
    modDest[15] = &Pan_B;

    Pitch.setRange(0.0f, 127.0f);
    modDest[16] = &Pitch;

    Pitch_A.setRange(0.0f, 127.0f);
    modDest[17] = &Pitch_A;

    Pitch_B.setRange(0.0f, 127.0f);
    modDest[18] = &Pitch_B;

    Phase.setRange(treestate.getParameter("OSC_A_PHASE")->getNormalisableRange().start, treestate.getParameter("OSC_A_PHASE")->getNormalisableRange().end);
    modDest[19] = &Phase;

    Phase_A.parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("OSC_A_PHASE"));
    Phase_A.setRange(treestate.getParameter("OSC_A_PHASE")->getNormalisableRange().start, treestate.getParameter("OSC_A_PHASE")->getNormalisableRange().end);
    modDest[20] = &Phase_A;

    Phase_B.parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("OSC_B_PHASE"));
    Phase_B.setRange(treestate.getParameter("OSC_B_PHASE")->getNormalisableRange().start, treestate.getParameter("OSC_B_PHASE")->getNormalisableRange().end);
    modDest[21] = &Phase_B;

    OscMix.parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("OSCMIX"));
    OscMix.setRange(treestate.getParameter("OSCMIX")->getNormalisableRange().start, treestate.getParameter("OSCMIX")->getNormalisableRange().end);
    modDest[22] = &OscMix;

    M_Radius[0].parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M1_RADIUS"));
    M_Radius[0].setRange(treestate.getParameter("M1_RADIUS")->getNormalisableRange().start, treestate.getParameter("M1_RADIUS")->getNormalisableRange().end);
    modDest[23] = &M_Radius[0];

    M_Soft[0].parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M1_SOFT"));
    M_Soft[0].setRange(treestate.getParameter("M1_SOFT")->getNormalisableRange().start, treestate.getParameter("M1_SOFT")->getNormalisableRange().end);
    modDest[24] = &M_Soft[0];

    M_Speed[0].parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M1_SPEED"));
    M_Speed[0].setRange(treestate.getParameter("M1_SPEED")->getNormalisableRange().start, treestate.getParameter("M1_SPEED")->getNormalisableRange().end);
    modDest[25] = &M_Speed[0];

    M_Radius[1].parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M2_RADIUS"));
    M_Radius[1].setRange(treestate.getParameter("M2_RADIUS")->getNormalisableRange().start, treestate.getParameter("M2_RADIUS")->getNormalisableRange().end);
    modDest[26] = &M_Radius[1];

    M_Soft[1].parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M2_SOFT"));
    M_Soft[1].setRange(treestate.getParameter("M2_SOFT")->getNormalisableRange().start, treestate.getParameter("M2_SOFT")->getNormalisableRange().end);
    modDest[27] = &M_Soft[1];

    M_Speed[1].parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M2_SPEED"));
    M_Speed[1].setRange(treestate.getParameter("M2_SPEED")->getNormalisableRange().start, treestate.getParameter("M2_SPEED")->getNormalisableRange().end);
    modDest[28] = &M_Speed[1];

    M_Radius[2].parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M3_RADIUS"));
    M_Radius[2].setRange(treestate.getParameter("M3_RADIUS")->getNormalisableRange().start, treestate.getParameter("M3_RADIUS")->getNormalisableRange().end);
    modDest[29] = &M_Radius[2];

    M_Soft[2].parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M3_SOFT"));
    M_Soft[2].setRange(treestate.getParameter("M3_SOFT")->getNormalisableRange().start, treestate.getParameter("M3_SOFT")->getNormalisableRange().end);
    modDest[30] = &M_Soft[2];

    M_Speed[2].parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M3_SPEED"));
    M_Speed[2].setRange(treestate.getParameter("M3_SPEED")->getNormalisableRange().start, treestate.getParameter("M3_SPEED")->getNormalisableRange().end);
    modDest[31] = &M_Speed[2];

    M_Radius[3].parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M4_RADIUS"));
    M_Radius[3].setRange(treestate.getParameter("M4_RADIUS")->getNormalisableRange().start, treestate.getParameter("M4_RADIUS")->getNormalisableRange().end);
    modDest[32] = &M_Radius[3];

    M_Soft[3].parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M4_SOFT"));
    M_Soft[3].setRange(treestate.getParameter("M4_SOFT")->getNormalisableRange().start, treestate.getParameter("M4_SOFT")->getNormalisableRange().end);
    modDest[33] = &M_Soft[3];

    M_Speed[3].parameter = dynamic_cast<juce::AudioParameterFloat*>(treestate.getParameter("M4_SPEED"));
    M_Speed[3].setRange(treestate.getParameter("M4_SPEED")->getNormalisableRange().start, treestate.getParameter("M4_SPEED")->getNormalisableRange().end);
    modDest[34] = &M_Speed[3];
};