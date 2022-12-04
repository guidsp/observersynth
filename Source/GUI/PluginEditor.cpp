#include "../Audio/PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ObserverAudioProcessorEditor::ObserverAudioProcessorEditor (ObserverAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    observerComponent(p, this),
    voicesKnob(ImageCache::getFromMemory(BinaryData::knob2Strip_png, BinaryData::knob2Strip_pngSize), 127, true, "", 7, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),
    glideKnob(ImageCache::getFromMemory(BinaryData::knob2Strip_png, BinaryData::knob2Strip_pngSize), 127, true, " ms", 1, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),
    outputKnob(ImageCache::getFromMemory(BinaryData::knob2Strip_png, BinaryData::knob2Strip_pngSize), 127, true, "", 3, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),
    oscMixKnob(ImageCache::getFromMemory(BinaryData::knob2Strip_png, BinaryData::knob2Strip_pngSize), 127, true, "", 3, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),

    qKnob(ImageCache::getFromMemory(BinaryData::knob2Strip_png, BinaryData::knob2Strip_pngSize), 127, true, "", 4, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag), 
    cutoffKnob(ImageCache::getFromMemory(BinaryData::knob1Strip_png, BinaryData::knob1Strip_pngSize), 127, true, "", 3, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag), 
    filterStateKnob(ImageCache::getFromMemory(BinaryData::knob2Strip_png, BinaryData::knob2Strip_pngSize), 127, true, "", 7, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),

    voicesKnobAttachment((*audioProcessor.treestate.getParameter("VOICES")), voicesKnob, nullptr),
    glideKnobAttachment((*audioProcessor.treestate.getParameter("GLIDE")), glideKnob, nullptr),
    outputKnobAttachment((*audioProcessor.treestate.getParameter("OUTPUT")), outputKnob, nullptr),
    oscMixKnobAttachment((*audioProcessor.treestate.getParameter("OSCMIX")), oscMixKnob, nullptr),

    qKnobAttachment((*audioProcessor.treestate.getParameter("FILTER_Q")), qKnob, nullptr),
    cutoffKnobAttachment((*audioProcessor.treestate.getParameter("FILTER_CUTOFF")), cutoffKnob, nullptr),
    filterStateKnobAttachment((*audioProcessor.treestate.getParameter("FILTER_STATE")), filterStateKnob, nullptr),

    oscAcomponent(audioProcessor, this),
    oscBcomponent(audioProcessor, this),
    mod1component(audioProcessor, this, this),
    mod2component(audioProcessor, this, this),
    mod3component(audioProcessor, this, this),
    mod4component(audioProcessor, this, this)
{
    //Plugin window size
    this->setSize(pluginWidth, pluginHeight);

    //Default font
    LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypefaceName("Avenir Next");

     //Set slider attachments
    oscAcomponent.connectToTreestate("OSC_A");
    oscBcomponent.connectToTreestate("OSC_B");
    mod1component.connectToTreestate("M1");
    mod2component.connectToTreestate("M2");
    mod3component.connectToTreestate("M3");
    mod4component.connectToTreestate("M4");
    //=====
    
    //Resize limits
    this->setResizeLimits(getWidth() * 0.6f, getHeight() * 0.6f, getWidth() * 2.f, getHeight() * 2.f);

    //Set fixed aspect ratio
    double ratio = (double)getWidth() / (double)getHeight();
    this->getConstrainer()->setFixedAspectRatio(ratio);

    //Retrieve images from resources
    auto bgImageUnder_ = juce::ImageCache::getFromMemory(BinaryData::LayoutUnder_png, BinaryData::LayoutUnder_pngSize);
    auto bgImageOver_ = juce::ImageCache::getFromMemory(BinaryData::LayoutOver_png, BinaryData::LayoutOver_pngSize);
    auto bellImage_ = juce::ImageCache::getFromMemory(BinaryData::Bell_png, BinaryData::Bell_pngSize);
    
    //Stretch image to the window
    if (bgImageUnder_.isValid())
        bgImageUnder.setImage(bgImageUnder_, RectanglePlacement::stretchToFit);

    if (bgImageOver_.isValid())
        bgImageOver.setImage(bgImageOver_, RectanglePlacement::stretchToFit);

    if (bellImage_.isValid()) {
        auto col = Colour(0.0f, 0.0f, 0.0f, 0.0f);
        juce::Image nullImg = juce::ImageCache::getFromMemory(NULL, 0);
        bell.setImages(true, true, false, bellImage_, 1.0f, col, nullImg, 1.0f, col, bellImage_, 1.0f, col);
    }

    //Set image saved by the processor
    observerComponent.changeImage(audioProcessor.observerImg);

    //Mouse ignores the overlay component
    bgImageOver.setInterceptsMouseClicks(false, false);

    //=====
    //UI
    //BG image
    Component::addAndMakeVisible(bgImageUnder);
    bgImageUnder.setBoundsRelative(0.f, 0.f, 1.f, 1.f);

    //Oscillators
    Component::addAndMakeVisible(oscAcomponent);
    oscAcomponent.setBoundsRelative(0.055791388f, 0.083406496f, 0.258944815f, 0.23046532f);
    Component::addAndMakeVisible(oscBcomponent);
    oscBcomponent.setBoundsRelative(0.684657368f, 0.083406496f, 0.258944815f, 0.23046532f);

    //Settings
    voicesKnob.setRange(audioProcessor.treestate.getParameter("VOICES")->getNormalisableRange().start,
        audioProcessor.treestate.getParameter("VOICES")->getNormalisableRange().end, 1);
    voicesKnob.showValue(true, 0.3f);
    voicesKnob.addListener(this);
    Component::addAndMakeVisible(voicesKnob);
    voicesKnob.setBoundsRelative(0.478775015f, 0.0469710272f, 0.0412371134f, 0.0597014925f);

    glideKnob.setRange(audioProcessor.treestate.getParameter("GLIDE")->getNormalisableRange().start,
        audioProcessor.treestate.getParameter("GLIDE")->getNormalisableRange().end, 1.0f);
    glideKnob.addListener(this);
    Component::addAndMakeVisible(glideKnob);
    glideKnob.setBoundsRelative(0.394784718f, 0.1035996488f, 0.0412371134f, 0.0597014925f);

    outputKnob.setRange(audioProcessor.treestate.getParameter("OUTPUT")->getNormalisableRange().start,
        audioProcessor.treestate.getParameter("OUTPUT")->getNormalisableRange().end, 0.00001f);
    outputKnob.addListener(this);
    Component::addAndMakeVisible(outputKnob);
    outputKnob.setBoundsRelative(0.563674954f, 0.1035996488f, 0.0412371134f, 0.0597014925f);

    oscMixKnob.setRange(audioProcessor.treestate.getParameter("OSCMIX")->getNormalisableRange().start,
        audioProcessor.treestate.getParameter("OSCMIX")->getNormalisableRange().end, 0.00001f);
    oscMixKnob.addListener(this);
    Component::addAndMakeVisible(oscMixKnob);
    oscMixKnob.setBoundsRelative(0.478775015f, 0.1624231782f, 0.0412371134f, 0.0597014925f);

    //Filter
    qKnob.setRange(audioProcessor.treestate.getParameter("FILTER_Q")->getNormalisableRange().start,
        audioProcessor.treestate.getParameter("FILTER_Q")->getNormalisableRange().end, 0.00001f);
    qKnob.addListener(this);
    qKnob.showValue(true, 0.35f);
    Component::addAndMakeVisible(qKnob);
    qKnob.setBoundsRelative(0.37477258f, 0.35118525f, 0.0412371134f, 0.0597014925f);
    
    cutoffKnob.setRange(audioProcessor.treestate.getParameter("FILTER_CUTOFF")->getNormalisableRange().start,
        audioProcessor.treestate.getParameter("FILTER_CUTOFF")->getNormalisableRange().end, 0.00001f);
    cutoffKnob.addListener(this);
    Component::addAndMakeVisible(cutoffKnob);
    cutoffKnob.setBoundsRelative(0.467556094f, 0.33143107989f, 0.0636749545f, 0.092186128f);

    filterStateKnob.setRange(audioProcessor.treestate.getParameter("FILTER_STATE")->getNormalisableRange().start,
        audioProcessor.treestate.getParameter("FILTER_STATE")->getNormalisableRange().end, 0.00001f);
    filterStateKnob.addListener(this);
    Component::addAndMakeVisible(filterStateKnob);
    filterStateKnob.setBoundsRelative(0.58277744f, 0.35118525f, 0.0412371134f, 0.0597014925f);

    //Modulators and observer components
    Component::addAndMakeVisible(mod1component);
    mod1component.setBoundsRelative(0.006367495f, 0.391571554f, 0.223165555f, 0.236172081f);
    Component::addAndMakeVisible(mod2component);
    mod2component.setBoundsRelative(0.766525166f, 0.391571554f, 0.223165555f, 0.236172081f);
    Component::addAndMakeVisible(mod3component);
    mod3component.setBoundsRelative(0.006367495f, 0.707638272f, 0.223165555f, 0.236172081f);
    Component::addAndMakeVisible(mod4component);
    mod4component.setBoundsRelative(0.766525166f, 0.703248464f, 0.223165555f, 0.236172081f);

    Component::addAndMakeVisible(observerComponent);
    observerComponent.setBoundsRelative(0.26228f, 0.5079f, 0.47043f, 0.44137f);

    //Overlay
    Component::addAndMakeVisible(bgImageOver);
    bgImageOver.setBoundsRelative(0.f, 0.f, 1.f, 1.f);

    //Bell
    bell.addListener(this);
    Component::addAndMakeVisible(bell);
    bell.setBoundsRelative(0.716798f, 0.7212467f, 0.048514f, 0.1944688f);
    //=====
}

ObserverAudioProcessorEditor::~ObserverAudioProcessorEditor() {

};

//==============================================================================
void ObserverAudioProcessorEditor::paint(juce::Graphics& g) {
};

void ObserverAudioProcessorEditor::resized() {
    //BG
    bgImageUnder.setBoundsRelative(0.f, 0.f, 1.f, 1.f);

    //Oscillators
    oscAcomponent.setBoundsRelative(0.055791388f, 0.083406496f, 0.258944815f, 0.23046532f);
    oscBcomponent.setBoundsRelative(0.684657368f, 0.083406496f, 0.258944815f, 0.23046532f);

    //Settings
    voicesKnob.setBoundsRelative(0.478775015f, 0.0469710272f, 0.0412371134f, 0.0597014925f);
    glideKnob.setBoundsRelative(0.394784718f, 0.1035996488f, 0.0412371134f, 0.0597014925f);
    outputKnob.setBoundsRelative(0.563674954f, 0.1035996488f, 0.0412371134f, 0.0597014925f);
    oscMixKnob.setBoundsRelative(0.478775015f, 0.1624231782f, 0.0412371134f, 0.0597014925f);

    //Filter
    qKnob.setBoundsRelative(0.37477258f, 0.35118525f, 0.0412371134f, 0.0597014925f);
    cutoffKnob.setBoundsRelative(0.467556094f, 0.33143107989f, 0.0636749545f, 0.092186128f);
    filterStateKnob.setBoundsRelative(0.58277744f, 0.35118525f, 0.0412371134f, 0.0597014925f);

    //Modulation
    mod1component.setBoundsRelative(0.006367495f, 0.391571554f, 0.223165555f, 0.236172081f);
    mod2component.setBoundsRelative(0.766525166f, 0.391571554f, 0.223165555f, 0.236172081f);
    mod3component.setBoundsRelative(0.006367495f, 0.707638272f, 0.223165555f, 0.236172081f);
    mod4component.setBoundsRelative(0.766525166f, 0.703248464f, 0.223165555f, 0.236172081f);

    //Observer
    observerComponent.setBoundsRelative(0.26228f, 0.5079f, 0.47043f, 0.44137f);

    //Overlay
    bgImageOver.setBoundsRelative(0.f, 0.f, 1.f, 1.f);

    //Bell
    bell.setBoundsRelative(0.716798f, 0.7212467f, 0.048514f, 0.1944688f);
};

void ObserverAudioProcessorEditor::sliderValueChanged(juce::Slider* slider) {
    if (slider->getParentComponent() == &oscAcomponent) {
        oscAcomponent.setBoundsRelative(0.055791388f, 0.083406496f, 0.258944815f, 0.23046532f);
    }
    else if (slider->getParentComponent() == &oscBcomponent) {
        oscBcomponent.setBoundsRelative(0.684657368f, 0.083406496f, 0.258944815f, 0.23046532f);
    }
    else if (slider->getParentComponent() == &mod1component) {
        if (slider == &mod1component.modDestKnob) {
            audioProcessor.suspendProcessing(true);

            //Remove mod source if the previous selection wasn't none
            if (audioProcessor.prevModDest[0] > 0)
                audioProcessor.modDest[audioProcessor.prevModDest[0]]->removeModSource(audioProcessor.modulator[0]);

            //Add mod source if the new selection isn't none
            if (static_cast<int>(mod1component.modDestKnob.getValue()) > 0)
                audioProcessor.modDest[static_cast<int>(mod1component.modDestKnob.getValue())]->addModSource(audioProcessor.modulator[0]);

            audioProcessor.prevModDest[0] = static_cast<int>(mod1component.modDestKnob.getValue());

            audioProcessor.vcaModsSize = audioProcessor.VCA.modSources.size();

            audioProcessor.suspendProcessing(false);
        }

        mod1component.setBoundsRelative(0.006367495f, 0.391571554f, 0.223165555f, 0.236172081f);
    }
    else if (slider->getParentComponent() == &mod2component) {
        if (slider == &mod2component.modDestKnob) {
            audioProcessor.suspendProcessing(true);

            if (audioProcessor.prevModDest[1] > 0)
                audioProcessor.modDest[audioProcessor.prevModDest[1]]->removeModSource(audioProcessor.modulator[1]);

            if (static_cast<int>(mod2component.modDestKnob.getValue()) > 0)
                audioProcessor.modDest[static_cast<int>(mod2component.modDestKnob.getValue())]->addModSource(audioProcessor.modulator[1]);

            audioProcessor.prevModDest[1] = static_cast<int>(mod2component.modDestKnob.getValue());

            audioProcessor.vcaModsSize = audioProcessor.VCA.modSources.size();

            audioProcessor.suspendProcessing(false);
        }
        
        mod2component.setBoundsRelative(0.766525166f, 0.391571554f, 0.223165555f, 0.236172081f);
    }
    else if (slider->getParentComponent() == &mod3component) {
        if (slider == &mod3component.modDestKnob) {
            audioProcessor.suspendProcessing(true);

            if (audioProcessor.prevModDest[2] > 0)
                audioProcessor.modDest[audioProcessor.prevModDest[2]]->removeModSource(audioProcessor.modulator[2]);

            if (static_cast<int>(mod3component.modDestKnob.getValue()) > 0)
                audioProcessor.modDest[static_cast<int>(mod3component.modDestKnob.getValue())]->addModSource(audioProcessor.modulator[2]);

            audioProcessor.prevModDest[2] = static_cast<int>(mod3component.modDestKnob.getValue());

            audioProcessor.vcaModsSize = audioProcessor.VCA.modSources.size();

            audioProcessor.suspendProcessing(false);
        }

        mod3component.setBoundsRelative(0.006367495f, 0.707638272f, 0.223165555f, 0.236172081f);
    }
    else if (slider->getParentComponent() == &mod4component) {
        if (slider == &mod4component.modDestKnob) {
            audioProcessor.suspendProcessing(true);

            if (audioProcessor.prevModDest[3] > 0)
                audioProcessor.modDest[audioProcessor.prevModDest[3]]->removeModSource(audioProcessor.modulator[3]);

            if (static_cast<int>(mod4component.modDestKnob.getValue()) > 0)
                audioProcessor.modDest[static_cast<int>(mod4component.modDestKnob.getValue())]->addModSource(audioProcessor.modulator[3]);

            audioProcessor.prevModDest[3] = static_cast<int>(mod4component.modDestKnob.getValue());

            audioProcessor.vcaModsSize = audioProcessor.VCA.modSources.size();

            audioProcessor.suspendProcessing(false);
        }
        
        mod4component.setBoundsRelative(0.766525166f, 0.703248464f, 0.223165555f, 0.236172081f);
    }
    else {
        if (slider == &oscMixKnob)
            oscMixKnob.setBoundsRelative(0.478775015f, 0.1624231782f, 0.0412371134f, 0.0597014925f);
        else if (slider == &outputKnob)
            outputKnob.setBoundsRelative(0.563674954f, 0.1035996488f, 0.0412371134f, 0.0597014925f);
        else if (slider == &glideKnob)
            glideKnob.setBoundsRelative(0.394784718f, 0.1035996488f, 0.0412371134f, 0.0597014925f);
        else if (slider == &voicesKnob)
            voicesKnob.setBoundsRelative(0.478775015f, 0.0469710272f, 0.0412371134f, 0.0597014925f);
        else if (slider == &qKnob)
            qKnob.setBoundsRelative(0.37477258f, 0.35118525f, 0.0412371134f, 0.0597014925f);
        else if (slider == &cutoffKnob)
            cutoffKnob.setBoundsRelative(0.467556094f, 0.33143107989f, 0.0636749545f, 0.092186128f);
        else if (slider == &filterStateKnob)
            filterStateKnob.setBoundsRelative(0.58277744f, 0.35118525f, 0.0412371134f, 0.0597014925f);
        else if (slider == &observerComponent.observerPoleComponent[0].radiusSlider)
            observerComponent.setBoundsRelative(0.26228f, 0.5079f, 0.47043f, 0.44137f);
        else if (slider == &observerComponent.observerPoleComponent[1].radiusSlider)
            observerComponent.setBoundsRelative(0.26228f, 0.5079f, 0.47043f, 0.44137f);
        else if (slider == &observerComponent.observerPoleComponent[2].radiusSlider)
            observerComponent.setBoundsRelative(0.26228f, 0.5079f, 0.47043f, 0.44137f);
        else if (slider == &observerComponent.observerPoleComponent[3].radiusSlider)
            observerComponent.setBoundsRelative(0.26228f, 0.5079f, 0.47043f, 0.44137f);
    }
}

void ObserverAudioProcessorEditor::buttonClicked(juce::Button* button) {
    if (button == &bell) {
        imgPicker = std::make_unique<FileChooser>("Load image", File::getSpecialLocation(File::userDesktopDirectory), "*");
        
        imgPicker->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles, [this](const FileChooser& chooser) {

            File output (chooser.getResult());

            if (output.getFileExtension() == ".png") {
                MemoryOutputStream out;
                PNGImageFormat format;
                Image image = ImageCache::getFromFile(output);
                image = image.rescaled(observerWidth, observerHeight);
                
                format.writeImageToStream(image, out);

                audioProcessor.suspendProcessing(true);
                audioProcessor.imageData.insert(out.getData(), out.getDataSize(), 0);
                audioProcessor.observerImg = image;
                audioProcessor.writeImgToArray(image);
                audioProcessor.suspendProcessing(false);

                observerComponent.changeImage(image);
                ImageCache::releaseUnusedImages();
            }
            else if (output.getFileExtension() == ".jpg" || output.getFileExtension() == ".jpeg") {
                MemoryOutputStream out;
                JPEGImageFormat format;
                Image image = ImageCache::getFromFile(output);
                image = image.rescaled(observerWidth, observerHeight);

                format.writeImageToStream(image, out);

                audioProcessor.suspendProcessing(true);
                audioProcessor.imageData.insert(out.getData(), out.getDataSize(), 0);
                audioProcessor.observerImg = image;
                audioProcessor.writeImgToArray(image);
                audioProcessor.suspendProcessing(false);

                observerComponent.changeImage(ImageCache::getFromFile(output));
                ImageCache::releaseUnusedImages();
            }
            else if (output.getFileExtension() == ".gif") {
                MemoryOutputStream out;
                GIFImageFormat format;
                Image image = ImageCache::getFromFile(output);
                image = image.rescaled(observerWidth, observerHeight);

                format.writeImageToStream(image, out);

                audioProcessor.suspendProcessing(true);
                audioProcessor.imageData.insert(out.getData(), out.getDataSize(), 0);
                audioProcessor.observerImg = image;
                audioProcessor.writeImgToArray(image);
                audioProcessor.suspendProcessing(false);

                observerComponent.changeImage(ImageCache::getFromFile(output));
                ImageCache::releaseUnusedImages();
            }
        });
    }
}
