#pragma once
#include "Obs_Envelope.h"

Obs_Envelope::Obs_Envelope() {
    //sampleRate = 44100.0f;
};

void Obs_Envelope::setup(const float& samplerate, const float& transitTimeInMs) {
    sampleRate = samplerate;

    //transitRate can't be larger than 1, otherwise it overshoots
    transitRate_ = juce::jlimit(0.0, 1.0, 1.0 / (transitTimeInMs * sampleRate * 0.001));
};

void Obs_Envelope::noteOn(const float& vel) {
    velocity_ = vel;

    decayRate = (1.0f - sustain) * decayRate_;

    if (state != State::_idle) {
        transitRate = transitRate_ * value;
        state = State::_transit;
    }
    else if (attackRate > 0.0f) {
        velocity = vel;
        state = State::_attack;
    }
    else if (decayRate_ > 0.0f) {
        velocity = vel;
        value = 1.0f;
        state = State::_decay;
    }
    else {
        velocity = vel;
        state = State::_sustain;
    }
};

void Obs_Envelope::noteOff() {
    if (releaseRate_ > 0.0f) {
        releaseRate = releaseRate_ * value; //NOTE: Necessary?
        state = State::_release;
    }
    else {
        value = 0.0f;
        state = State::_idle;
    }
};

void Obs_Envelope::kill() {
    value = 0.0;
    state = State::_idle;
}