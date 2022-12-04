#pragma once
#include <JuceHeader.h>
#include "Obs_Modulator.h"

/// Base class for allowing generic functionality between different types of modulation destinations.
class ModulationDestinationBase {
private:
	constexpr static auto maximumVoices_ = maximumVoices;

public:
	/// Set to polyphonic or monophonic mode. Is true if not specified.
	// bool isPoly = true;

	/// Linearly interpolating parameter value. Remember to set up interpolation time.
	LineSlide slidingParamVal;

	/// Vector of this ModulationDestination's Modulators.
	std::vector<Modulator*> modSources;

	/// Set parameter range. 0.0 to 1.0 if not specified.
	virtual void setRange(const float& outMin, const float& outMax) = 0;

	/// Add a Modulator as a modulation source.
	void addModSource(Modulator& modsource) {
		modSources.push_back(&modsource);
		modSourcesSize = modSources.size();
		//modsource.isPoly = this->isPoly;
	}

	void removeModSource(Modulator& modulatorToRemove) {
		for (int i = 0; i < modSources.size(); i++) {
			if (modSources[i] == &modulatorToRemove) {
				modSources.erase(modSources.begin() + i);
				modSourcesSize = modSources.size();
				break;
			}
		}

		//Kill the value
		for (int v = 0; v < maximumVoices_; v++)
			modulatorToRemove.value_[v] = 0.0f;
	}

protected:
	ModulationDestinationBase() {
	}

	int modSourcesSize = 0;
};

template <bool hasParam, bool isBipolar>
class ModulationDestination;

template <>
class ModulationDestination<true, true> : public ModulationDestinationBase {
public:
	ModulationDestination() {
	}

	/*inline*/ float value(const int& v) {
		auto val = parameter->get();

		if (modSourcesSize <= 0) {
			if (val != prevVal) {
				slidingParamVal.set(val);
				prevVal = val;
			}

			return slidingParamVal.getVal();
		}
		else {
			if (val != prevVal) {
				slidingParamVal.set(val);
				prevVal = val;
			}

			auto paramVal = slidingParamVal.getVal();

			//int voice = v * this->isPoly;
			auto modulationVal = 1.0f;

			for (int i = 0; i < modSourcesSize; i++)
				modulationVal *= modSources[i]->value(v/* * this->isPoly*/);

			modulationVal *= range;

			return juce::jlimit(min, max, paramVal + modulationVal);
		}
	}

	/// Set parameter range. 0.0 to 1.0 if not specified. The value range should not exceed the parameter's, 
	/// but a parameter-less modulation destination should be able to go into the negatives.
	void setRange(const float& outMin, const float& outMax) {
		min = outMin;
		max = outMax;
		range = outMax - outMin;
	}

	/// Pointer to the AudioParameterFloat. Set this before any operations.
	juce::AudioParameterFloat* parameter;

	/// Set default value for when no modulator is connected and parameter is null. It's 0.0 if not specified.
	float defaultValue = 0.0f,
		min = 0.0f,
		max = 1.0f;
private:
	float range = 0.0f,
		prevVal = 0.0f;
};

template <>
class ModulationDestination<true, false> : public ModulationDestinationBase {
public:
	ModulationDestination() {
	}

	/// Get value for voice v
	/*inline*/ float value(const int& v) {
		auto val = parameter->get();

		if (modSourcesSize <= 0) {
			if (val != prevVal) {
				slidingParamVal.set(val);
				prevVal = val;
			}

			return slidingParamVal.getVal();

		}
		else {
			if (val != prevVal) {
				slidingParamVal.set(val);
				prevVal = val;
			}

			auto paramVal = slidingParamVal.getVal();

			//int voice = v * isPoly;
			auto modulationVal = 1.0f;


			for (int i = 0; i < modSourcesSize; i++) {
				modulationVal *= (modSources[i]->depth >= 0.0f) ?
					modSources[i]->value(v/* * this->isPoly*/) : modSources[i]->value(v/* * this->isPoly*/) - modSources[i]->depth;
			}

			modulationVal *= range;

			return juce::jlimit(min, max, paramVal + modulationVal);
		}
	}

	/// Set parameter range. 0.0 to 1.0 if not specified.
	void setRange(const float& outMin, const float& outMax) {
		//The output value range should not exceed the parameter's, but a parameter-less modulation destination should be able to go into the negatives.
		min = outMin;
		max = outMax;
		range = outMax - outMin;
	}

	/// Pointer to the AudioParameterFloat. Set this before any operations.
	juce::AudioParameterFloat* parameter;

	/// Set default value for when no modulator is connected and parameter is null. It's 0.0 if not specified.
	float defaultValue = 0.0f,
		min = 0.0f,
		max = 1.0f;

private:
	float range = 0.0f,
		prevVal = 0.0f;
};

template <>
class ModulationDestination<false, true> : public ModulationDestinationBase {
public:
	ModulationDestination() {
	}

	/*inline*/ float value(const int& v) {
		if (modSourcesSize == 0) {
			return defaultValue;
		}
		else {
			//int voice = v * this->isPoly;
			auto modulationVal = 1.0f;

			for (int i = 0; i < modSourcesSize; i++)
				modulationVal *= modSources[i]->value(v/* * this->isPoly*/);

			modulationVal *= range;

			return juce::jlimit(min, max, modulationVal);
		}
	}

	/// Set parameter range. 0.0 to 1.0 if not specified.
	void setRange(const float& outMin, const float& outMax) {
		//The output value range should not exceed the parameter's, but a parameter-less modulation destination should be able to go into the negatives.
		min = -1.0f * outMax;
		max = outMax;
		range = outMax - outMin;
	}

	/// Pointer to the AudioParameterFloat. Set this before any operations.
	juce::AudioParameterFloat* parameter;

	/// Set default value for when no modulator is connected and parameter is null. It's 0.0 if not specified.
	float defaultValue = 0.0f,
		min = 0.0f,
		max = 1.0f;

private:
	float range = 0.0f,
		prevVal = 0.0f;
};

template <>
class ModulationDestination<false, false> : public ModulationDestinationBase {
public:
	ModulationDestination() {
	}

	/*inline*/ float value(const int& v) {
		if (modSourcesSize <= 0) {
			return defaultValue;
		}
		else {
			//int voice = v * this->isPoly;
			auto modulationVal = 1.0f;

			for (int i = 0; i < modSourcesSize; i++) {
				modulationVal *= (modSources[i]->depth >= 0.0f) ?
					modSources[i]->value(v/* * this->isPoly*/) : modSources[i]->value(v/* * this->isPoly*/) - modSources[i]->depth;
			}

			modulationVal *= range;

			return juce::jlimit(min, max, modulationVal);
		}
	}

	/// Set parameter range. 0.0 to 1.0 if not specified.
	void setRange(const float& outMin, const float& outMax) {
		//The output value range should not exceed the parameter's, but a parameter-less modulation destination should be able to go into the negatives.
		min = -1.0f * outMax;
		max = outMax;
		range = outMax - outMin;
	}

	/// Pointer to the AudioParameterFloat. Set this before any operations.
	juce::AudioParameterFloat* parameter;

	/// Set default value for when no modulator is connected and parameter is null. It's 0.0 if not specified.
	float defaultValue = 0.0f,
		min = 0.0f,
		max = 1.0f;

private:
	float range = 0.0f,
		prevVal = 0.0f;
};