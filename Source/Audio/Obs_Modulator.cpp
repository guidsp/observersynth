#pragma once
#include "Obs_Modulator.h"

Modulator::Modulator() {
	veloAmt = 1.0;
	depth = 1.0;
}

void Modulator::setup(const float& samplingrate, const float& envelopeTransitTime) {
	for (int i = 0; i < maximumVoices_; i++)
		envelope[i].setup(samplingrate, envelopeTransitTime);
}

void Modulator::setParams(const float& attack, const float& decay, const float& sustain, const float& release) {
	this->setAttack(attack);
	this->setDecay(decay);
	this->setSustain(sustain);
	this->setRelease(release);
}