#pragma once
#include <JuceHeader.h>
#include "../common.h"
#include "FilmStripKnob.h"
#include "../Audio/PluginProcessor.h"

struct Component_ObserverPole {
	juce::ImageComponent pole;
	juce::Point<float> polePos;
	juce::ImageComponent voice[maximumVoices];
	FilmStripKnob radiusSlider = FilmStripKnob(ImageCache::getFromMemory(BinaryData::sliderStripwBG_png, BinaryData::sliderStripwBG_pngSize), 63, true, "", 3, 1.0, juce::Slider::SliderStyle::LinearBarVertical);
}; 

class Component_Observer : public juce::Component, public juce::Timer {
public:
	Component_Observer(ObserverAudioProcessor&, juce::Slider::Listener* listener);
	~Component_Observer();
	void paint(juce::Graphics& g) override;
	void resized() override;
	void timerCallback() override;
	void changeImage(const juce::Image& image);
	void mouseDoubleClick(const juce::MouseEvent& event) override;
	void mouseDown(const juce::MouseEvent& event) override;
	void mouseDrag(const juce::MouseEvent& event) override;
	void connectToTreestate();

	Component_ObserverPole observerPoleComponent[nOfModulators];

private:
	/// Logarithmic distribution between 0 and 1.
	inline float simpleLog(const float& in) {
		return 2.0f * in - in * in;
	}

	/// Amount of the pole components to keep on screen when dragging them outside the boundaries. eg 0.1 will let the pole leave the component until 0.1 * the width of the image remains of it.
	static constexpr float berth = 0.1f;
	/// Opacity of pole image components.
	static constexpr float poleOpacity = 0.9f;
	/// Pole image component relative width; for centering the coordinates.
	static constexpr float relPoleWidth = 0.1495f;
	/// Pole image component relative height; for centering the coordinates.
	static constexpr float relPoleHeight = 0.192075f;
	/// Pole voice component relative width; for centering the coordinates.
	static constexpr float relVoiceWidth = 0.024f;
	/// Pole voice component relative height; for centering the coordinates.
	static constexpr float relVoiceHeight = 0.0370368f;

	ObserverAudioProcessor& audioProcessor;

	juce::ImageComponent img;
	juce::ComponentDragger dragger;
	juce::ComponentBoundsConstrainer* boundsConstrainer;

	std::unique_ptr<juce::SliderParameterAttachment> radiusSliderAttachment[nOfModulators];

	constexpr static int maximumVoices_ = maximumVoices;
	constexpr static int nOfModulators_ = nOfModulators;
};