#include "Obs_ZDFSVF.h"

SVF::SVF() {
	freq = 10000.0f;
	Q = 1.0f;
	this->setMode(0.0f);
	//this->setup(44100.0f);
	state1 = state2 = { 0.0f, 0.0f };
}

void SVF::setup(const float& samplingRate) {
	sampleRateI = 1.0f / samplingRate;
}
