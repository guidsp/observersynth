#include "Obs_Pole.h"

ObserverPole::ObserverPole() {
	this->setAspectRatio(matrixSize.getX(), matrixSize.getY());
}

void ObserverPole::setup(const float& sampleRate, const int& refreshRateHz) {
	sampleRateI = 1.0f / sampleRate;
	updateInterval = static_cast<int>(sampleRate / static_cast<float>(refreshRateHz));
	clockConst = sampleRateI * static_cast<float>(updateInterval) * (juce::MathConstants<float>::pi * 2.0f);
	LineSlide::setSampleRate(sampleRate);
}

void ObserverPole::setAspectRatio(const float& width, const float& height) {
	if (width > height) {
		xScale = 1.0f;
		yScale = width / height;
	}
	else if (height > width) {
		xScale = height / width;
		yScale = 1.0f;
	}
	else {
		xScale = 1.0f;
		yScale = 1.0f;
	}
}

void ObserverPole::setAspectRatio(const int& width, const int& height) {
	if (width > height) {
		xScale = 1.0f;
		yScale = static_cast<float>(width) / static_cast<float>(height);
	}
	else if (height > width) {
		xScale = static_cast<float>(height) / static_cast<float>(width);
		yScale = 1.0f;
	}
	else {
		xScale = 1.0f;
		yScale = 1.0f;
	}
}