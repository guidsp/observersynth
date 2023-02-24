#pragma once
#include <JuceHeader.h>

struct SampleData {
	float l = 0.0,
		r = 0.0;
};

/// State variable filter.
class SVF {
public:
	SVF();

	/// Filters input and writes to it.
	inline void process(SampleData &in) {
		SampleData hp = { (in.l - (freq + Q) * state1.l - state2.l) * g,
			(in.r - (freq + Q) * state1.r - state2.r) * g };

		SampleData bp = { state1.l + freq * hp.l, state1.r + freq * hp.r };

		SampleData lp = { state2.l + freq * bp.l, state2.r + freq * bp.r };

		state1 = { 2.0f * bp.l - state1.l, 2.0f * bp.r - state1.r };
		state2 = { 2.0f * lp.l - state2.l, 2.0f * lp.r - state2.r };

		in = { amt0 * lp.l + amt1 * bp.l + amt2 * hp.l,
		amt0 * lp.r + amt1 * bp.r + amt2 * hp.r };
	}

	/// Set cutoff frequency and resonance.
	inline void setParams(const float& f, const float& q) {
		freq = tanf(juce::MathConstants<float>::pi * f * sampleRateI);
		Q = 1.0f / q;
		g = 1.0f / (freq * (freq + Q) + 1.0f);
	}

	/// 0 = lowpass, 1 = bandpass, 2 = highpass.
	inline void setMode(const float& filterMode) {
		amt0 = juce::jmax(0.0f, 1.0f - filterMode);
		amt1 = juce::jmax(0.0f, 1.0f - fabs(filterMode - 1.0f));
		amt2 = juce::jmax(0.0f, 1.0f - fabs(filterMode - 2.0f));

	}

	/// Set sample rate before processing.
	static void setup(const float& samplingRate);

private:
	SampleData state1, state2;

	float freq, Q, g,
		amt0, amt1, amt2;

	inline static float sampleRateI;
};
