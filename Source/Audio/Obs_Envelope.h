#pragma once
#include <JuceHeader.h>
#include "../common.h"

/// Envelope generator class similar to JUCE's with a few extra things.
class Obs_Envelope {
public:
	Obs_Envelope();

	enum State {
		_idle,
		_attack,
		_decay,
		_sustain,
		_release,
		_transit, //Linear interpolation of the envelope value to 0 when the envelope is triggered if it's not idle.
		_start
	};

	/// Current state of the envelope.
	State state = State::_idle;

	/// Most recent envelope amplitude value.
	float value = 0.0f;

	/// Calculate next amplitude value. Use the value member variable to consult the latest value without advancing the envelope.
	inline float getValue() {
		if (state <= 2) {
			if (state == State::_idle) {
				return 0.0f;
			}
			else if (state == State::_attack) {
				value += attackRate;

				if (value >= 1.0f) {
					value = 1.0f;
					state = decayRate_ > 0.0f ? State::_decay : State::_sustain;
					return velocity;
				}

				return value * value * velocity;
			}
			else /* if (state == State::_decay)*/ {
				value -= decayRate;

				if (value <= sustain) {
					value = sustain;
					state = State::_sustain;
					return value * value * velocity;
				}

				return value * value * velocity;
			}
		}
		else {
			if (state == State::_sustain) {
				return value * value * velocity;
			}
			else if (state == State::_release) {
				value -= releaseRate;

				if (value <= releaseThreshold) {
					value = 0.0f;
					state = State::_idle;
					return 0.0f;
				}

				return value * value * velocity;
			}
			else if (state == State::_transit) {
				value -= transitRate;

				if (value <= 0.0f) {
					value = 0.0f;
					velocity = velocity_;
					state = State::_start;
					return 0.0f;
				}

				return value * value * velocity;
			}
			else /*if (state == State::start)*/ {
				state = State::_attack;
				return 0.0f;
			}
		}
	}

	inline void setAttack(const float& attackInSeconds) {
		attackRate = 1.0f / ((attackInSeconds + 0.0002f) * sampleRate);
	}

	inline void setDecay(const float& decayInSeconds) {
		decayRate_ = 1.0f / ((decayInSeconds + 0.0002f) * sampleRate);
		decayRate = (1.0f - sustain) * decayRate_;
	}

	inline void setSustain(const float& sustainAmp) {
		sustain = sustainAmp;
	}

	inline void setRelease(const float& releaseInSeconds) {
		releaseRate_ = 1.0f / ((releaseInSeconds + 0.0002f) * sampleRate);

		//NOTE: Problematic?
		releaseRate = releaseRate_ * value;
	}

	/// Start envelope
    void noteOn(const float& vel);

	/// Start release phase
    void noteOff();

	/// Sets envelope to 0 and makes it idle
	void kill();

	/// Mandatory before any playback. Transit time is how long the envelope fades out for if it's retriggered before finishing.
	void setup(const float& samplerate, const float& transitTimeInMs);


private:
	inline static float sampleRate;

	float velocity = 0.0f,
		velocity_ = 0.0f,
		attackRate = 0.0f,
		decayRate = 0.0f,
		decayRate_ = 0.0f,
		sustain = 0.0f,
		releaseRate = 0.0f,
		releaseRate_ = 0.0f,
		transitRate = 0.0f,
		transitRate_ = 0.0f;

	//Release instantly snaps to 0 when it goes below this threshold
	const float releaseThreshold = 0.001f;
};