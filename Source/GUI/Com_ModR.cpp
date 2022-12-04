#pragma once
#include "Com_ModR.h"

Component_ModR::Component_ModR(ObserverAudioProcessor& p, juce::Slider::Listener* listener, juce::Button::Listener* blistener) :
	audioProcessor(p),
	amtKnob(ImageCache::getFromMemory(BinaryData::knob2Strip_png, BinaryData::knob2Strip_pngSize), 127, true, "", 3, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),
	veloKnob(ImageCache::getFromMemory(BinaryData::knob2Strip_png, BinaryData::knob2Strip_pngSize), 127, true, "", 3, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),
	softKnob(ImageCache::getFromMemory(BinaryData::knob2Strip_png, BinaryData::knob2Strip_pngSize), 127, true, " ms", 7, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),
	speedKnob(ImageCache::getFromMemory(BinaryData::knob2Strip_png, BinaryData::knob2Strip_pngSize), 127, true, " hz", 4, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),
	modDestKnob(ImageCache::getFromMemory(BinaryData::modDestRStrip_png, BinaryData::modDestRStrip_pngSize), nOfModulationDestinations, false, "", 3, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),
	aSlider(ImageCache::getFromMemory(BinaryData::sliderStrip_png, BinaryData::sliderStrip_pngSize), 63, true, "", 3, 1.0, juce::Slider::SliderStyle::LinearBarVertical),
	dSlider(ImageCache::getFromMemory(BinaryData::sliderStrip_png, BinaryData::sliderStrip_pngSize), 63, true, "", 3, 1.0, juce::Slider::SliderStyle::LinearBarVertical),
	sSlider(ImageCache::getFromMemory(BinaryData::sliderStrip_png, BinaryData::sliderStrip_pngSize), 63, true, "", 3, 1.0, juce::Slider::SliderStyle::LinearBarVertical),
	rSlider(ImageCache::getFromMemory(BinaryData::sliderStrip_png, BinaryData::sliderStrip_pngSize), 63, true, "", 3, 1.0, juce::Slider::SliderStyle::LinearBarVertical),
	retrigger()
{
	this->setSize(modWidth, modHeight);

	//Set up retrigger button
	auto buttonOff = juce::ImageCache::getFromMemory(BinaryData::button0_png, BinaryData::button0_pngSize);
	auto buttonOn = juce::ImageCache::getFromMemory(BinaryData::button1_png, BinaryData::button1_pngSize);
	juce::Image nullImg = juce::ImageCache::getFromMemory(NULL, 0);
	auto col = Colour(0.0f, 0.0f, 0.0f, 0.0f);
	retrigger.setImages(true, true, false, buttonOff, 1.0f, col, nullImg, 1.0f, col, buttonOn, 1.0f, col);
	retrigger.setClickingTogglesState(true);
	//=====

	//Set up sliders
	amtKnob.addListener(listener);
	Component::addAndMakeVisible(amtKnob);
	amtKnob.setBoundsRelative(0.8152174f, 0.38104089f, 0.18478261f, 0.252788104f);
	amtKnob.setRange(audioProcessor.treestate.getParameter("M1_DEPTH")->getNormalisableRange().start, audioProcessor.treestate.getParameter("M1_DEPTH")->getNormalisableRange().end, 0.0f);
	amtKnob.showValue(true, 0.35);

	veloKnob.addListener(listener);
	Component::addAndMakeVisible(veloKnob);
	veloKnob.setBoundsRelative(0.09103261f, 0.747211896f, 0.18478261f, 0.252788104f);
	veloKnob.setRange(audioProcessor.treestate.getParameter("M1_VELOCITY")->getNormalisableRange().start, audioProcessor.treestate.getParameter("M1_VELOCITY")->getNormalisableRange().end, 0.0f);

	softKnob.addListener(listener);
	Component::addAndMakeVisible(softKnob);
	softKnob.setBoundsRelative(0.430706522f, 0.747211896f, 0.18478261f, 0.252788104f);
	softKnob.setRange(audioProcessor.treestate.getParameter("M1_SOFT")->getNormalisableRange().start, audioProcessor.treestate.getParameter("M1_SOFT")->getNormalisableRange().end, 0.0f);
	softKnob.showValue(true, 0.35f);

	speedKnob.addListener(listener);
	Component::addAndMakeVisible(speedKnob);
	speedKnob.setBoundsRelative(0.77173913f, 0.747211896f, 0.18478261f, 0.252788104f);
	speedKnob.setRange(audioProcessor.treestate.getParameter("M1_SPEED")->getNormalisableRange().start, audioProcessor.treestate.getParameter("M1_SPEED")->getNormalisableRange().end, 0.0f);
	speedKnob.showValue(true, 0.35);

	modDestKnob.addListener(listener);
	Component::addAndMakeVisible(modDestKnob);
	modDestKnob.setBoundsRelative(0.0f, 0.0f, 0.7065217f, 0.2527881f);
	modDestKnob.setRange(0, nOfModulationDestinations - 1, 0);

	aSlider.addListener(listener);
	Component::addAndMakeVisible(aSlider);
	aSlider.setBoundsRelative(0.2201087f, 0.2695167f, 0.149456422f, 0.4163569f);
	aSlider.setRange(audioProcessor.treestate.getParameter("M1_ATTACK")->getNormalisableRange().start, audioProcessor.treestate.getParameter("M1_ATTACK")->getNormalisableRange().end, 0.0f);

	dSlider.addListener(listener);
	Component::addAndMakeVisible(dSlider);
	dSlider.setBoundsRelative(0.36956522f, 0.2695167f, 0.149456422f, 0.4163569f);
	dSlider.setRange(audioProcessor.treestate.getParameter("M1_DECAY")->getNormalisableRange().start, audioProcessor.treestate.getParameter("M1_DECAY")->getNormalisableRange().end, 0.0f);
	
	sSlider.addListener(listener);
	Component::addAndMakeVisible(sSlider);
	sSlider.setBoundsRelative(0.51902174f, 0.2695167f, 0.149456422f, 0.4163569f);
	sSlider.setRange(audioProcessor.treestate.getParameter("M1_SUSTAIN")->getNormalisableRange().start, audioProcessor.treestate.getParameter("M1_SUSTAIN")->getNormalisableRange().end, 0.0f);

	rSlider.addListener(listener);
	Component::addAndMakeVisible(rSlider);
	rSlider.setBoundsRelative(0.66847826f, 0.2695167f, 0.149456422f, 0.4163569f);
	rSlider.setRange(audioProcessor.treestate.getParameter("M1_RELEASE")->getNormalisableRange().start, audioProcessor.treestate.getParameter("M1_RELEASE")->getNormalisableRange().end, 0.0f);

	retrigger.addListener(blistener);
	Component::addAndMakeVisible(retrigger);
	retrigger.setBoundsRelative(0.039402174f, 0.35873606f, 0.149456522f, 0.20632f);
	//=====
}

Component_ModR::~Component_ModR() {
}

void Component_ModR::paint(juce::Graphics& g) {
}

void Component_ModR::resized() {
	amtKnob.setBoundsRelative(0.8152174f, 0.38104089f, 0.18478261f, 0.252788104f);
	veloKnob.setBoundsRelative(0.09103261f, 0.747211896f, 0.18478261f, 0.252788104f);
	softKnob.setBoundsRelative(0.430706522f, 0.747211896f, 0.18478261f, 0.252788104f);
	speedKnob.setBoundsRelative(0.77173913f, 0.747211896f, 0.18478261f, 0.252788104f);

	modDestKnob.setBoundsRelative(0.0f, 0.0f, 0.7065217f, 0.2527881f);

	aSlider.setBoundsRelative(0.2201087f, 0.2695167f, 0.149456422f, 0.4163569f);
	dSlider.setBoundsRelative(0.36956522f, 0.2695167f, 0.149456422f, 0.4163569f);
	sSlider.setBoundsRelative(0.51902174f, 0.2695167f, 0.149456422f, 0.4163569f);
	rSlider.setBoundsRelative(0.66847826f, 0.2695167f, 0.149456422f, 0.4163569f);

	retrigger.setBoundsRelative(0.039402174f, 0.35873606f, 0.149456522f, 0.20632f);
};

void Component_ModR::connectToTreestate(String prepend) {
	amtKnobAttachment = std::unique_ptr<SliderParameterAttachment>(new SliderParameterAttachment((*audioProcessor.treestate.getParameter(prepend + "_DEPTH")), amtKnob, nullptr));
	veloKnobAttachment = std::unique_ptr<SliderParameterAttachment>(new SliderParameterAttachment((*audioProcessor.treestate.getParameter(prepend + "_VELOCITY")), veloKnob, nullptr));
	softKnobAttachment = std::unique_ptr<SliderParameterAttachment>(new SliderParameterAttachment((*audioProcessor.treestate.getParameter(prepend + "_SOFT")), softKnob, nullptr));
	speedKnobAttachment = std::unique_ptr<SliderParameterAttachment>(new SliderParameterAttachment((*audioProcessor.treestate.getParameter(prepend + "_SPEED")), speedKnob, nullptr));
	modDestKnobAttachment = std::unique_ptr<SliderParameterAttachment>(new SliderParameterAttachment((*audioProcessor.treestate.getParameter(prepend + "_DESTINATION")), modDestKnob, nullptr));
	aSliderAttachment = std::unique_ptr<SliderParameterAttachment>(new SliderParameterAttachment((*audioProcessor.treestate.getParameter(prepend + "_ATTACK")), aSlider, nullptr));
	dSliderAttachment = std::unique_ptr<SliderParameterAttachment>(new SliderParameterAttachment((*audioProcessor.treestate.getParameter(prepend + "_DECAY")), dSlider, nullptr));
	sSliderAttachment = std::unique_ptr<SliderParameterAttachment>(new SliderParameterAttachment((*audioProcessor.treestate.getParameter(prepend + "_SUSTAIN")), sSlider, nullptr));
	rSliderAttachment = std::unique_ptr<SliderParameterAttachment>(new SliderParameterAttachment((*audioProcessor.treestate.getParameter(prepend + "_RELEASE")), rSlider, nullptr));
	retrigAttachment = std::unique_ptr<ButtonParameterAttachment>(new ButtonParameterAttachment((*audioProcessor.treestate.getParameter(prepend + "_RETRIGGER")), retrigger, nullptr));
};