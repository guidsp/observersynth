#pragma once
#include <JuceHeader.h>
#include "../Audio/PluginProcessor.h"
#include "Com_Osc.h"
#include "Com_ModL.h"
#include "Com_ModR.h"
#include "Com_Obser.h"

//constexpr auto pluginWidth = 3298
//constexpr auto pluginHeight = 2278

/// Plugin window size
constexpr auto pluginWidth = 935;
constexpr auto pluginHeight = 645;

//==============================================================================
/**
*/
class ObserverAudioProcessorEditor  : public juce::AudioProcessorEditor, public Slider::Listener, public Button::Listener
{
public:
    ObserverAudioProcessorEditor (ObserverAudioProcessor&);
    ~ObserverAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged(juce::Slider* slider) override;
    void buttonClicked(juce::Button* button) override;

    //Settings
    FilmStripKnob voicesKnob, glideKnob, outputKnob, oscMixKnob;
    //VCF
    FilmStripKnob qKnob, cutoffKnob, filterStateKnob;

private:
    //Reference to ap
    ObserverAudioProcessor& audioProcessor;

    //Layout images
    ImageComponent bgImageUnder,
        bgImageOver;

    //Oscillator components
    Component_Oscillator oscAcomponent,
        oscBcomponent;

    //Modulator components
    Component_ModL mod1component, mod3component;
    Component_ModR mod2component, mod4component;

    //Bell component
    ImageButton bell;

    //Observer component; includes all 4 poles
    Component_Observer observerComponent;

    //Slider attachments
    SliderParameterAttachment voicesKnobAttachment, glideKnobAttachment, outputKnobAttachment, oscMixKnobAttachment,
        qKnobAttachment, cutoffKnobAttachment, filterStateKnobAttachment;

    //Image picker
    std::unique_ptr<FileChooser> imgPicker;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ObserverAudioProcessorEditor)
};
