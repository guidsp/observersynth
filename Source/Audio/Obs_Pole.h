#pragma once
#include <JuceHeader.h>
#include "../common.h"
#include "LineSlide.h"

///A single observer pole, consisting of a pole around which nOfVoices voices travel on a 2d plane.
class ObserverPole {
private:
	//Maximum number of supported voices.
	static constexpr int maximumVoices_ = maximumVoices;

	/// Resolution for matrix to pick values from. Constant for now..
	static constexpr juce::Point<int> matrixSize = juce::Point<int>(observerWidth, observerHeight);

public:
	ObserverPole();

	/// Update physics and return matrix value for voice v's coordinates. Update happens periodically according to counter and updateInterval.
	/*inline*/ float getValue(const int& v) {
		counter[v]--;

		if (counter[v] <= 0) {
			counter[v] = updateInterval;

			//Advance clock
			clock[v] += clockSpeed[v] * clockConst;

			//Warp to 0 - 2PI
			if (clock[v] > juce::MathConstants<float>::pi * 2.0f) 
				clock[v] -= juce::MathConstants<float>::pi * 2.0f;
			else if (clock[v] < 0.0f)
				clock[v] += juce::MathConstants<float>::pi * 2.0f;

			//Get target coordinates
			juce::Point<float> target_(polePos.getX() + radius[v] * xScale * cos(clock[v]), polePos.getY() + radius[v] * yScale * sin(clock[v]));

			//Get acceleration
			accelerationFactor = 0.1f + 0.025f * abs(clockSpeed[v]); //Maps accelerationFactor to the speed (arbitrarily)
			voiceA[v] = (target_ - voicePos[v]) * accelerationFactor;

			//Get velocity
			voiceV[v] += voiceA[v];

			//Bounce off walls
			if ((voicePos[v].getX() >= 1.0f && voiceV[v].getX() > 0.0f) || (voicePos[v].getX() <= 0.0f && voiceV[v].getX() < 0.0f))
				voiceV[v].setX(voiceV[v].getX() * -1.0f);
			if ((voicePos[v].getY() >= 1.0f && voiceV[v].getY() > 0.0f) || (voicePos[v].getY() <= 0.0f && voiceV[v].getY() < 0.0f))
				voiceV[v].setY(voiceV[v].getY() * -1.0f);

			voiceV[v] *= speedDampening;

			speedFactor = 0.05f + 0.045f * abs(clockSpeed[v]); //Maps velocityFactor to the speed (arbitrarily)
			voicePos[v] += voiceV[v] * speedFactor;

			//Get value from matrix
			
			int x = juce::jlimit(0, matrixSize.getX() - 1, cheapfloor(voicePos[v].getX() * static_cast<float>(matrixSize.getX() - 1)));
			int y = juce::jlimit(0, matrixSize.getY() - 1, cheapfloor(voicePos[v].getY() * static_cast<float>(matrixSize.getY() - 1)));

			output[v].set(imgVal[x][y]);
		}

		return output[v].getVal();
	}

	/// Set sampling rate and rate of the physics update. Mandatory before playback.
	static void setup(const float& sampleRate, const int& refreshRateHz);

	/// (0 - 1) Voice progress along its circular route. Set to 0 to restart it's position movement.
	float clock[maximumVoices_] = { 0.0f };

	/// Velocity.
	juce::Point<float> voiceV[maximumVoices_];

	/// Acceleration.
	juce::Point<float> voiceA[maximumVoices_];

	/// Draggable pole coordinates.
	juce::Point<float> polePos;

	/// Voice coordinates.
	juce::Point<float> voicePos[maximumVoices_];

	/// Set voice cycle speed in hz. Negative values reverse the movement. 0 hz by default if not set.
	float clockSpeed[maximumVoices_] = { 0.0f };

	/// (0 - 1) Movement radius.
	float radius[maximumVoices_] = { 0.0f };

	/// Holds value to output.
	LineSlide output[maximumVoices_];

	/// Data matrix to retrieve from.
	float imgVal[matrixSize.getX()][matrixSize.getY()];

private:
	//Quick floor function
	inline int cheapfloor(const float& x) {
		return (int)x - (x < (int)x);
	}

	/// Set aspect ratio of the rectangle. 1:1 by default (a square).
	void setAspectRatio(const float& width, const float& height);

	/// Set aspect ratio of the rectangle. 1:1 by default (a square).
	void setAspectRatio(const int& width, const int& height);

	/// For scaling the circular movement to the aspect ratio.
	float xScale = 1.0f,
		yScale = 1.0f,
	/// Physics constants
		accelerationFactor, //Recommended 0.1 - 0.3
		speedFactor; //Recommended 0.05 - 0.4

	/// Countdown to update.
	int counter[maximumVoices_] = { 0 };

	constexpr static float speedDampening = 0.95f;

	/// Inverse samplerate.
	inline static float sampleRateI = 1.0f / 44100.0f;

	/// Multiply by Hz values to get in samples
	inline static float	clockConst = 0.0f;

	/// Update rate in samples.
	inline static int updateInterval = 0;
};