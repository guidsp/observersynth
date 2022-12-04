#pragma once
#include "JuceHeader.h"
#include "LineSlide.h"

class SmoothParamBase {
public:
    /// Linearly interpolating parameter value. Remember to set up interpolation time.
    LineSlide slidingParamVal;

protected:
    /// Previous value.
    float prevVal;
};

/// Class for smoothed parameter pointers
template <typename T>
class SmoothParam : public SmoothParamBase {
public:
    inline float getValue() {
        auto val = this->parameter->get();

        if (val != this->prevVal) {
            this->slidingParamVal.set(val);
            this->prevVal = val;
        }

        return this->slidingParamVal.getVal();
    }

    T* parameter;
};