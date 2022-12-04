#pragma once
#include <JuceHeader.h>
#include "../common.h"

//Notespan of each wavetable region.
static constexpr int notesPerRegion = 8; 
static const float regionsPerNote = 1.0f / static_cast<float>(notesPerRegion);
//Number of different wavetables for each waveshape.
static constexpr int nRegions = 128 / notesPerRegion; //Upper limit 128 (is past 10 khz and won't need harmonics)

//Length of wavetables in samples.
constexpr int wavetableSize = 2048; 

//Maximum acceptable midi note.
static constexpr float maxMidiNote = 135.0f; //135 is comfortably beyond 20 khz
//Inverse for clipping to 0 - 1 when using mtof lookup table
static constexpr float invMaxMidiNote = 1.0f / maxMidiNote;

/// Wavetable oscillator that blends between shapes.
class WT_Osc {
public:
	WT_Osc();

	~WT_Osc();

	//Output for one voice.
	float output(const int& voice);

	//Input in midi notes (0 - maxMidiNote).
	inline void setNote(const int& voice, const float& midinote) {
		//Calculate region. Last region spans 2 * regionsPerNote
		region[voice] = juce::jlimit(0.0f, nRegions - 1.0f, midinote * regionsPerNote);

		//Calculate phase increment
		//increment[voice] = lookup(juce::jlimit(0.0f, 135.0f, midinote) * invMaxMidiNote, mtofWT) * sampleInSecs;
		increment[voice] = lookup(juce::jlimit(0.0f, 1.0f, midinote * invMaxMidiNote), mtofWT) * sampleInSecs;
	}

	//Mandatory.
	static void setSamplerate(const float& sr);

	//(0 - 3). 0 = sine, 1 = low res saw, 2 = saw, 3 = square. Decimals will blend between them.
	float shape[maximumVoices] = { 0.0f };

	//For square, sets pulse width (phase displacement of inverted saw wave) (0 - 1). For sines and saw, morphs into something arbitrary.
	//The morphed waves have no anti aliasing so take it easy on the high notes.
	float shape2[maximumVoices] = { 0.0f };

	//(0 - 1).
	float phaseOffset[maximumVoices] = { 0.0f };

	//Set to 0 to reset phase.
	float phase[maximumVoices] = { 0.0f };

private:
	//Quick floor function
	inline static int cheapfloor(const float& x) {
		return (int)x - (x < (int)x);
	}

	//Look up value in wavetable, input index between 0 to 1.
	inline static float lookup(const float& index, float* table) {
		float i = index * (wavetableSize - 1);

		//Linear interpolation
		auto prevSamp = cheapfloor(i); //Index of closest lower sample
		auto upperWeight = i - prevSamp; //Proximity of index to index of lower sample
		auto lowerWeight = 1.0f - upperWeight; //Proximity of index to index of higher sample

		//output
		return lowerWeight * table[prevSamp] + upperWeight * table[prevSamp + 1];
	}

	float increment[maximumVoices] = { 0.0f },
		region[maximumVoices];

	inline static float sampleInSecs;

	//Creates sine wavetable
	float* makeSine();

	//Creates hard sine/soft saw wavetable
	float** makeSine2();

	//Creates saw wavetable
	float** makeSaw();

	//Creates MTOF lookup table
	float* makeMTOFtable();

	//TODO Speed up wavetable creation
	float* sineWT = makeSine();
	float** sine2WT = makeSine2();
	float** sawWT = makeSaw();
	float* mtofWT = makeMTOFtable();
	//=====
};