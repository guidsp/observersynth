#pragma once
#include <JuceHeader.h>
#include "Obs_Envelope.h"
#include "Obs_Pole.h"

class Modulator {
public:
	Modulator();

	/// Envelope amplitude multiplier.
	float depth;

	/// Gets the next envelope value multiplied by the modulator's depth value and observer value.
	inline float value(const int& v) {
		value_[v] = envelope[v].getValue();
		return depth * value_[v] * pole->getValue(v);
	};

	/// Set attack time in seconds.
	inline void setAttack(const float& a) {
		for (int i = 0; i < maximumVoices_; ++i)
			envelope[i].setAttack(a);
	}

	/// Set decay time in seconds.
	inline void setDecay(const float& d) {
		for (int i = 0; i < maximumVoices_; ++i)
			envelope[i].setDecay(d);
	}

	/// Set sustain (0 - 1 non clipping).
	inline void setSustain(const float& s) {
		for (int i = 0; i < maximumVoices_; ++i)
			envelope[i].setSustain(s);
	}

	/// Set release time in seconds.
	inline void setRelease(const float& r) {
		for (int i = 0; i < maximumVoices_; ++i)
			envelope[i].setRelease(r);
	}

	/// Set all params in seconds (sustain amplitude 0 - 1 non clipping).
	void setParams(const float& attack, const float& decay, const float& sustain, const float& release);

	/// Mandatory before playback. Sets the transit time (quick envelope fade to 0 on note steal) in milliseconds.
	void setup(const float& samplerate, const float& transitTimeInMs);

	/// Amount of velocity effect (0: always maximum velocity; 1: velocity totally controls envelope amplitude).
	float veloAmt;

	/// The most recent envelope amplitude value; use this to look up the envelope value without advancing to the next one.
	float value_[maximumVoices] = { 0.0f };

	/// Envelope owned by this modulator.
	Obs_Envelope envelope[maximumVoices];

	/// Pointer to pole.
	ObserverPole* pole;

	/// Modulation destination sets this when assigned.
	//bool isPoly = true;

private:
	constexpr static int maximumVoices_ = maximumVoices;
};