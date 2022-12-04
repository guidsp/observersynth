#pragma once
#include "Obs_Osc.h"
#include "Obs_ZDFSVF.h"
#include "SmoothParam.h"
#include "Obs_ModDestination.h"

//==============================================================================

class ObserverAudioProcessor  : public juce::AudioProcessor
{
public:
    ObserverAudioProcessor();
    ~ObserverAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    void setInitSettings();
    
    //==============================================================================
    /// Set polyphony
    void setVoices(const int& nvoices); 

    /// Interpret midi messages
    void processMidi(const juce::MidiMessage& message); 

    /// Arbitrary processes before rendering (calculate pitch, check for VCA envelopes freeing and update modulator data)
    void updateVoice(const int& v);

    /// Render designated audio block
    void render(juce::AudioBuffer<float>& audiobuffer, int start, int end);

    /// Write image brightness data to matrix
    void writeImgToArray(const juce::Image& imageToWrite);

    //===========================================

    //APVTS
    juce::AudioProcessorValueTreeState treestate;

    //Pointers to mod destinations
    ModulationDestinationBase* modDest[nOfModulationDestinations];
    //Holds the previous modulation destination of each modulator.
    int prevModDest[nOfModulators] = { -1, -1, -1, -1 }; 

    //VCA modulation destination for editor access.
    ModulationDestination<false, false> VCA;

    //Modulators
    Modulator modulator[nOfModulators];

    //Atomics for GUI communication.
    juce::Atomic<float> _polePosX_[nOfModulators], _polePosY_[nOfModulators];
    juce::Atomic<float> _voicePosX_[nOfModulators][maximumVoices], _voicePosY_[nOfModulators][maximumVoices];
    juce::Atomic<float> _voiceOpacity_[nOfModulators][maximumVoices];

    //Number of modulators assigned to VCA.
    int vcaModsSize = 0; 

    //Observer image.
    juce::Image observerImg;
    //Raw image data for saving and loading with plugin state.
    juce::MemoryBlock imageData;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout makeParams();
    void setParamPointers();
    void setModDestPointers();

    //Queue with voice indexes ordered by age.
    std::vector<int> voiceQueue; 

    /// Data type describing each voice.
    struct Voice {
        enum class VoiceState {
            free,
            busy,
            releasing
        };

        VoiceState state = VoiceState::free;
        int note = 69;
    };

    //Array with voices.
    Voice voice[maximumVoices]; 

    //=====
    //Modulation destinations except VCA.
    ModulationDestination<false, true> Pitch_A, Pitch_B, Pitch,
        Shape1,
        Shape2,
        Phase,
        Pan;

    ModulationDestination<false, false> VCA_A, VCA_B;
 
    ModulationDestination<true, true> Shape1_A, Shape1_B,
        Shape2_A, Shape2_B,
        Phase_A, Phase_B,
        OscMix, Pan_A, Pan_B,
        filterCutoff, filterQ, filterState,
        M_Radius[nOfModulators], M_Soft[nOfModulators], M_Speed[nOfModulators];
    //=====

    //=====
    //Pointers to the smoothed parameters (not modulated)
    SmoothParamBase* parameter[nOfSmoothedParams];

    //Osc A
    juce::AudioParameterInt* Octave_A;
    int Octave_A_;
    SmoothParam<juce::AudioParameterFloat> Tune_A;
    float Tune_A_;

    //Osc B
    juce::AudioParameterInt* Octave_B;
    int Octave_B_;
    SmoothParam<juce::AudioParameterFloat> Tune_B;
    float Tune_B_;

    //Settings
    juce::AudioParameterInt* voices;
    int voices_;

    juce::AudioParameterFloat* glide;

    juce::AudioParameterFloat* output;

    //Observers
    SmoothParam<juce::AudioParameterFloat> M_Depth[nOfModulators];

    juce::AudioParameterFloat* M_Velocity[nOfModulators], * M_Attack[nOfModulators], * M_Decay[nOfModulators],
        * M_Sustain[nOfModulators], * M_Release[nOfModulators];

    juce::AudioParameterBool* M_Retrigger[nOfModulators];

    juce::AudioParameterInt* M_Destination[nOfModulators];
    //=====

    //Oscillators
    WT_Osc oscA; 
    WT_Osc oscB;

    //Filter
    SVF filter[maximumVoices]; 

    //Observer poles
    ObserverPole pole[nOfModulators];

    //Gliding pitch value
    LineSlide notePitchSlide[maximumVoices]; 

    /*
    struct SampleData {
        float l = 0.0,
            r = 0.0;
    };
    */

    //Sample data
    SampleData audioSignal;

    float 
        //Glide speed in ms.
        glideSpeed, 
        octaveAoffset,
        octaveBoffset,
        //Pitchwheel offset in ST
        pitchWheel = 0.0f, 
        //Previous values for listening to changes
        prevGain = 0.0f,
        prevNoteA[maximumVoices] = { 0.0f },
        prevNoteB[maximumVoices] = { 0.0f }, 
        prevMspeed[nOfModulators][maximumVoices] = { 0.0f },
        prevMradius[nOfModulators][maximumVoices] = { 0.0f },
        prevMsoft[nOfModulators][maximumVoices] = { 0.0f },
        prevCutoff[maximumVoices] = { 0.0f },
        prevQ[maximumVoices] = { 0.0f },
        prevState[maximumVoices] = { 0.0f };

    
    int 
        //Number of active envelope voices.
        activeEnvelopes,
        //Number of active voices at a given time.
        activeVoices,
        //Previous number of voices for setVoices.
        pastnVoices = 0; 

    //Transit time. How long an envelope will take to fade out before starting when it's retriggered.
    static constexpr float envelopeTransitTimeInMs = 2.0f; 
    //Range in ST of pitchwheel bend.
    static constexpr float pitchWheelRange = 1.0f;
    //Update rate in hz of pole physics.
    static constexpr int poleUpdateFreq = 75; 

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ObserverAudioProcessor)
};
