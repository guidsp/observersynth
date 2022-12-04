#pragma once
#include "Com_Obser.h"

Component_Observer::Component_Observer(ObserverAudioProcessor& p, juce::Slider::Listener* listener) :
boundsConstrainer(new juce::ComponentBoundsConstrainer),
audioProcessor(p)
{
	this->setSize(observerWidth, observerHeight);

	observerPoleComponent[0].pole.setImage(juce::ImageCache::getFromMemory(BinaryData::pole1_png, BinaryData::pole1_pngSize), juce::RectanglePlacement::centred);
	observerPoleComponent[1].pole.setImage(juce::ImageCache::getFromMemory(BinaryData::pole2_png, BinaryData::pole2_pngSize), juce::RectanglePlacement::centred);
	observerPoleComponent[2].pole.setImage(juce::ImageCache::getFromMemory(BinaryData::pole3_png, BinaryData::pole3_pngSize), juce::RectanglePlacement::centred);
	observerPoleComponent[3].pole.setImage(juce::ImageCache::getFromMemory(BinaryData::pole4_png, BinaryData::pole4_pngSize), juce::RectanglePlacement::centred);

	this->addAndMakeVisible(img);
	img.setBoundsRelative(0.0f, 0.0f, 1.0f, 1.0f);

	boundsConstrainer->setMinimumOnscreenAmounts(static_cast<int>(berth * this->getHeight()), static_cast<int>(berth * this->getWidth()),
		static_cast<int>(berth * this->getHeight()), static_cast<int>(berth * this->getWidth()));

	for (int i = 0; i < nOfModulators_; i++) {
		observerPoleComponent[i].pole.addMouseListener(this, false);
		observerPoleComponent[i].pole.setComponentID(juce::String(i));
		this->addAndMakeVisible(observerPoleComponent[i].pole, -1);
		observerPoleComponent[i].pole.setAlpha(poleOpacity);

		observerPoleComponent[i].pole.addChildComponent(observerPoleComponent[i].radiusSlider, -1);
		observerPoleComponent[i].radiusSlider.setRange(p.treestate.getParameter("M1_RADIUS")->getNormalisableRange().start, p.treestate.getParameter("M1_RADIUS")->getNormalisableRange().end, 0.0f);
		observerPoleComponent[i].radiusSlider.addListener(listener);
		observerPoleComponent[i].radiusSlider.setAlpha(2.0);

		for (int j = 0; j < maximumVoices_; j++) {
			observerPoleComponent[i].voice[j].setImage(juce::ImageCache::getFromMemory(BinaryData::poleVoice_png, BinaryData::poleVoice_pngSize), juce::RectanglePlacement::centred);
			observerPoleComponent[i].voice[j].setInterceptsMouseClicks(false, false);
			this->addAndMakeVisible(observerPoleComponent[i].voice[j], -1);
		}

		observerPoleComponent[i].polePos.setXY(audioProcessor._polePosX_[i].get() - relPoleWidth * 0.5f, audioProcessor._polePosY_[i].get() - relPoleHeight * 0.5f);
	}

	this->connectToTreestate();

	Timer::startTimerHz(20);
}

Component_Observer::~Component_Observer() {
	Timer::stopTimer();
	delete boundsConstrainer;
}

void Component_Observer::paint(juce::Graphics& g) {
}

void Component_Observer::resized() {
	boundsConstrainer->setMinimumOnscreenAmounts(static_cast<int>(berth * this->getHeight()), static_cast<int>(berth * this->getWidth()),
		static_cast<int>(berth * this->getHeight()), static_cast<int>(berth * this->getWidth()));

	for (int i = 0; i < nOfModulators_; i++) {
		observerPoleComponent[i].pole.setBoundsRelative(observerPoleComponent[i].polePos.getX(), observerPoleComponent[i].polePos.getY(), relPoleWidth, relPoleHeight);
		observerPoleComponent[i].radiusSlider.setBoundsRelative(0.0f, 0.0f, 0.3f, 1.0f);

		for (int j = 0; j < maximumVoices_; j++) {
			
			observerPoleComponent[i].voice[j].setBoundsRelative(
				audioProcessor._voicePosX_[i][j].get() - (relVoiceWidth * 0.5f), 
				audioProcessor._voicePosY_[i][j].get() - (relVoiceHeight * 0.5f),
				relVoiceWidth, relVoiceHeight
			);
		}
	}

	img.setBoundsRelative(0.0f, 0.0f, 1.0f, 1.0f);
}

void Component_Observer::timerCallback() {
	for (int i = 0; i < nOfModulators_; i++) {
		for (int j = 0; j < maximumVoices_; j++) {
			auto opacity = std::abs(audioProcessor._voiceOpacity_[i][j].get());
			observerPoleComponent[i].voice[j].setAlpha(3.0f * opacity);

			if (opacity > 0.0001f) {
				observerPoleComponent[i].voice[j].setCentreRelative(
					audioProcessor._voicePosX_[i][j].get(),
					audioProcessor._voicePosY_[i][j].get()
				);
			}
		}
	}
}

void Component_Observer::changeImage(const juce::Image& image) {
	img.setImage(image.rescaled((observerWidth), (observerHeight)), juce::RectanglePlacement::stretchToFit);
	img.setBoundsRelative(0.0f, 0.0f, 1.0f, 1.0f);
}

void Component_Observer::mouseDoubleClick(const juce::MouseEvent& event) {
	if (event.eventComponent == &observerPoleComponent[0].pole ||
		event.eventComponent == &observerPoleComponent[1].pole || 
		event.eventComponent == &observerPoleComponent[2].pole || 
		event.eventComponent == &observerPoleComponent[3].pole) 
	{
		event.eventComponent->getChildComponent(0)->setVisible(!(event.eventComponent->getChildComponent(0)->isVisible()));
	}

}

void Component_Observer::mouseDown(const juce::MouseEvent& event) {
	dragger.startDraggingComponent(event.eventComponent, event);
}

void Component_Observer::mouseDrag(const juce::MouseEvent& event) {
	//Send pole coordinates to audio processor
	dragger.dragComponent(event.eventComponent, event, boundsConstrainer);

	auto componentIndex = event.eventComponent->getComponentID().getIntValue();
	auto relativeX = static_cast<float>(event.eventComponent->getX()) / static_cast<float>(event.eventComponent->getParentWidth());
	auto relativeY = static_cast<float>(event.eventComponent->getY()) / static_cast<float>(event.eventComponent->getParentHeight());

	observerPoleComponent[componentIndex].polePos.setXY(relativeX, relativeY);

	audioProcessor._polePosX_[componentIndex].set(relativeX + relPoleWidth * 0.5f); audioProcessor._polePosY_[componentIndex].set(relativeY + relPoleHeight * 0.5f);
}

void Component_Observer::connectToTreestate() {
	radiusSliderAttachment[0] = std::unique_ptr<SliderParameterAttachment>(new SliderParameterAttachment((*audioProcessor.treestate.getParameter("M1_RADIUS")), observerPoleComponent[0].radiusSlider, nullptr));
	radiusSliderAttachment[1] = std::unique_ptr<SliderParameterAttachment>(new SliderParameterAttachment((*audioProcessor.treestate.getParameter("M2_RADIUS")), observerPoleComponent[1].radiusSlider, nullptr));
	radiusSliderAttachment[2] = std::unique_ptr<SliderParameterAttachment>(new SliderParameterAttachment((*audioProcessor.treestate.getParameter("M3_RADIUS")), observerPoleComponent[2].radiusSlider, nullptr));
	radiusSliderAttachment[3] = std::unique_ptr<SliderParameterAttachment>(new SliderParameterAttachment((*audioProcessor.treestate.getParameter("M4_RADIUS")), observerPoleComponent[3].radiusSlider, nullptr));
}