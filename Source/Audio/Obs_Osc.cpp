#pragma once
#include "Obs_Osc.h"

WT_Osc::WT_Osc() {
	//Set default voices
	for (int v = 0; v < maximumVoices; ++v)
		setNote(v, 64.0);

	//instanceCount++;
}

WT_Osc::~WT_Osc() {
	for (int r = 0; r < nRegions; r++) {
		delete[] sine2WT[r];
		delete[] sawWT[r];
	}

	delete[] sine2WT;
	delete[] sawWT;
}

float WT_Osc::output(const int& voice) {
	phase[voice] += increment[voice];
	auto region_ = static_cast<int>(region[voice]);
	phase[voice] = phase[voice] < 1.0f ? phase[voice] : phase[voice] - 1.0f;
	float oscMix;
	auto truePhase = phase[voice] + phaseOffset[voice];
	truePhase = truePhase < 1.0f ? truePhase : truePhase - 1.0f;

	if (shape[voice] < 2.0f) {
		//Sine2
		auto sine2 = lookup(truePhase, sine2WT[region_]);
		auto waveamp = 1.0f - fabs(shape[voice] - 1.0f);
		oscMix = waveamp * ((1.0f - shape2[voice]) * sine2 + shape2[voice] * (sine2 * sine2 - 0.5f) * 2.0f);

		if (shape[voice] < 1.0f) {
			float sinePhase = 1.0f - 2.0f * fabs(0.5f - fabs(2.0f * truePhase - 1.0f));
			auto sine = lookup(sinePhase, sineWT);
			auto signal = (truePhase < 0.5f) ? 1 : -1;

			//Sine
			waveamp = 1.0f - waveamp;
			oscMix += waveamp * signal * ((1.0f - shape2[voice]) * sine + shape2[voice] * sine * sine);
		}
		else {
			auto saw_ = lookup(truePhase, sawWT[region_]);
			auto signal = (truePhase < 0.5f) ? 1 : -1;

			//Saw
			auto saw = saw_ - shape2[voice] * (signal - saw_);
			waveamp = 1.0f - waveamp;
			oscMix += waveamp * saw * 0.5;
		}
	}
	else {
		auto saw_ = lookup(truePhase, sawWT[region_]);
		//auto signal = 2 * ((2 * truePhase) < 1) - 1;
		auto signal = (truePhase < 0.5f) ? 1 : -1;

		//Saw
		auto waveamp = 1.0f - fabs(shape[voice] - 2.0f);
		auto saw = saw_ - shape2[voice] * (signal - saw_);
		oscMix = waveamp * saw * 0.5f;

		//Square
		auto i_ = truePhase + 0.5f + 0.45f * shape2[voice];
		waveamp = 1.0 - waveamp;
		i_ = i_ < 1.0f ? i_ : i_ - 1.0f;
		auto sq = saw_ - lookup(i_, sawWT[region_]) - shape2[voice];
		oscMix += waveamp * sq * 0.3f;
	}

	return oscMix;
}

void WT_Osc::setSamplerate(const float& sr) {
	sampleInSecs = 1.0 / sr;
}

float* WT_Osc::makeSine() {
	static float sineWT_[wavetableSize];

	//Fill with first quarter of the sine
	for (int sample = 0; sample < wavetableSize; sample++)
		sineWT_[sample] = sin((static_cast<double>(sample) * 2.0 * juce::MathConstants<double>::pi) / (static_cast<double>(wavetableSize) * 4.0));

	return sineWT_;
}

float** WT_Osc::makeSine2() {
	float** sine2WT_;
	sine2WT_ = new float* [nRegions];

	//Make wavetables for each frequency region
	for (int r = 0; r < nRegions; r++) {
		sine2WT_[r] = new float[wavetableSize];

		//Max frequency of the region
		float regionMax = 25.2f + 25.2f * std::pow(2.f, (notesPerRegion * static_cast<float>(r) / 12.f));

		//Calculate number of harmonics for region
		int nharmonics = floor(44100.0f / regionMax); //NOTE: ?? 44100 / regionMax what sense does this make?

		for (int sample = 0; sample < wavetableSize; sample++) {
			sine2WT_[r][sample] = 0.0f;

			//Stack harmonics
			for (int h = 1; h < (nharmonics + 1); h++) {
				double phase_ = fmod((static_cast<double>(sample) * static_cast<double>(h)) / static_cast<double>(wavetableSize - 1), 1.0);
				double phase__ = 1.0 - fabs(1.0 - fmod(4.0 * phase_, 2.0));

				int signal = 2 * (phase_ < 0.5f) - 1;

				float val = signal * lookup(phase__, sineWT);

				sine2WT_[r][sample] = sine2WT_[r][sample] + (val / (std::pow(static_cast<float>(h), 2.0)));
			}
		}
	}

	return sine2WT_;
}

float** WT_Osc::makeSaw() {
	float** sawWT_;
	sawWT_ = new float* [nRegions];

	for (int r = 0; r < nRegions; r++) {
		sawWT_[r] = new float[wavetableSize];

		float regionMax = 25.2f + 25.2f * std::pow(2.f, (notesPerRegion * static_cast<float>(r) / 12.f));
		int nharmonics = floor(44100.0f / regionMax);

		for (int sample = 0; sample < wavetableSize; sample++) {
			sawWT_[r][sample] = 0.0f;

			for (int h = 1; h < (nharmonics + 1); h++) {
				double phase_ = fmod((static_cast<double>(sample) * static_cast<double>(h)) / static_cast<double>(wavetableSize - 1), 1.0);
				double phase__ = 1.0 - fabs(1.0 - fmod(4.0 * phase_, 2.0));

				int signal = 2 * (phase_ < 0.5f) - 1;

				float val = signal * lookup(phase__, sineWT);

				sawWT_[r][sample] = sawWT_[r][sample] + (val / (static_cast<float>(h)));
			}
		}
	}

	return sawWT_;
}

float* WT_Osc::makeMTOFtable() {
	static float mtofWT_[wavetableSize];

	for (int sample = 0; sample < wavetableSize; sample++) {
		auto note = static_cast<double>(maxMidiNote) * (static_cast<double>(sample) / static_cast<double>(wavetableSize - 1));
		mtofWT_[sample] = 440.0 * std::pow(2.0, ((note - 69.0) * (1.0 / 12.0)));
	}

	return mtofWT_;
}
