#pragma once
#include <JuceHeader.h>

/// Class for a value that interpolates linearly over time.
class LineSlide {
public:
	LineSlide();

	/// Latest calculated value.
	float value;

	/// Calculate the next value. If you only need to check the latest value, use the value variable.
	inline float getVal() {
		if (step > 0) {
			value += increment;
			step--;
		}

		return value;
	}

	/// Set travel settings
	inline void set(const float& newtarget) {
		increment = (newtarget - value) / nSteps;
		step = static_cast<int>(nSteps);
	}

	/// Travel time in number of steps (setting this variable directly disregards and overrides sampling rate). 0 breaks this, set to 1 for no slide.
	float nSteps = 1.0f;

	/// Set travel time in ms. Remember to set the sampleRate before using this.
	inline void setTime(const float& timeInMs) {
		nSteps = timeInMs * sampleRateKhz;
	}

	/// Setting this is mandatory before using setTime.
	static void setSampleRate(const float& sampleRate) {
		sampleRateKhz = sampleRate * 0.001f;
	}

private:
	float increment;

	inline static float sampleRateKhz;
	int step = 0;
};