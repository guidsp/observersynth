#pragma once
#include <JuceHeader.h>
#include "FilmStripKnob.h"
#include "../Audio/PluginProcessor.h"

constexpr int oscWidth = 854;
constexpr int oscHeight = 525;

class Component_Oscillator : public juce::Component
{
public:
	Component_Oscillator(ObserverAudioProcessor&, juce::Slider::Listener* listener);
	~Component_Oscillator() override;

	void paint(juce::Graphics& g) override;
	void resized() override;
	void connectToTreestate(String append);

	FilmStripKnob octKnob, shape1Knob, phaseKnob, tuneKnob, shape2Knob, panKnob;
	std::unique_ptr<SliderParameterAttachment> octKnobAttachment, shape1KnobAttachment, phaseKnobAttachment, tuneKnobAttachment, shape2KnobAttachment, panKnobAttachment;

private:
	ObserverAudioProcessor& audioProcessor;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Component_Oscillator)
};