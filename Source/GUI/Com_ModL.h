#pragma once
#include <JuceHeader.h>
#include "FilmStripKnob.h"
#include "../Audio/PluginProcessor.h"

class Component_ModL : public juce::Component
{
public:
	Component_ModL(ObserverAudioProcessor&, juce::Slider::Listener* listener, juce::Button::Listener* blistener);
	~Component_ModL() override;

	void paint(juce::Graphics& g) override;
	void resized() override;

	/// Connect the slider attachments to treestate.
	void connectToTreestate(String prepend);

	//Sliders
	FilmStripKnob amtKnob, veloKnob, softKnob, speedKnob, modDestKnob,
		aSlider, dSlider, sSlider, rSlider;

	ImageButton retrigger;

	//Attachment pointers
	std::unique_ptr<SliderParameterAttachment> amtKnobAttachment, veloKnobAttachment, softKnobAttachment, speedKnobAttachment, modDestKnobAttachment,
		aSliderAttachment, dSliderAttachment, sSliderAttachment, rSliderAttachment;
	std::unique_ptr<ButtonParameterAttachment> retrigAttachment;

private:
	//AP reference
	ObserverAudioProcessor& audioProcessor;

	static constexpr int modWidth = 736;
	static constexpr int modHeight = 538;
};