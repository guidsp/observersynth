#include "Com_Osc.h"

Component_Oscillator::Component_Oscillator(ObserverAudioProcessor& p, juce::Slider::Listener* listener) :
    audioProcessor(p),
    octKnob(ImageCache::getFromMemory(BinaryData::knob1Strip_png, BinaryData::knob1Strip_pngSize), 127, true, " octaves", 7, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),
shape1Knob(ImageCache::getFromMemory(BinaryData::knob1Strip_png, BinaryData::knob1Strip_pngSize), 127, true, "", 4, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),
phaseKnob(ImageCache::getFromMemory(BinaryData::knob1Strip_png, BinaryData::knob1Strip_pngSize), 127, true, "", 3, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),
tuneKnob(ImageCache::getFromMemory(BinaryData::knob1Strip_png, BinaryData::knob1Strip_pngSize), 127, true, " st", 4, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),
shape2Knob(ImageCache::getFromMemory(BinaryData::knob1Strip_png, BinaryData::knob1Strip_pngSize), 127, true, "", 4, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag),
panKnob(ImageCache::getFromMemory(BinaryData::knob1Strip_png, BinaryData::knob1Strip_pngSize), 127, true, "", 3, 1.0, juce::Slider::SliderStyle::RotaryVerticalDrag)
{    
    this->setSize(oscWidth, oscHeight);

    //UI
    octKnob.setRange(audioProcessor.treestate.getParameter("OSC_A_OCTAVE")->getNormalisableRange().start, audioProcessor.treestate.getParameter("OSC_A_OCTAVE")->getNormalisableRange().end, 1);
    octKnob.addListener(listener);
    Component::addAndMakeVisible(octKnob);
    octKnob.setBoundsRelative(0.0f, 0.0f, 0.2459f, 0.4f);

    shape1Knob.setRange(audioProcessor.treestate.getParameter("OSC_A_SHAPE1")->getNormalisableRange().start, audioProcessor.treestate.getParameter("OSC_A_SHAPE1")->getNormalisableRange().end, 0.0f);
    shape1Knob.showValue(true);
    shape1Knob.addListener(listener);
    Component::addAndMakeVisible(shape1Knob);
    shape1Knob.setBoundsRelative(0.377f, 0.0f, 0.2459f, 0.4f);

    phaseKnob.setRange(audioProcessor.treestate.getParameter("OSC_A_PHASE")->getNormalisableRange().start, audioProcessor.treestate.getParameter("OSC_A_PHASE")->getNormalisableRange().end, 0.0f);
    phaseKnob.addListener(listener);
    Component::addAndMakeVisible(phaseKnob);
    phaseKnob.setBoundsRelative(0.7541f, 0.0f, 0.2459f, 0.4f);

    tuneKnob.setRange(audioProcessor.treestate.getParameter("OSC_A_TUNE")->getNormalisableRange().start, audioProcessor.treestate.getParameter("OSC_A_TUNE")->getNormalisableRange().end, 0.0f);
    tuneKnob.showValue(true);
    tuneKnob.addListener(listener);
    Component::addAndMakeVisible(tuneKnob);
    tuneKnob.setBoundsRelative(0.0f, 0.6f, 0.2459f, 0.4f);

    shape2Knob.setRange(audioProcessor.treestate.getParameter("OSC_A_SHAPE2")->getNormalisableRange().start, audioProcessor.treestate.getParameter("OSC_A_SHAPE2")->getNormalisableRange().end, 0.0f);
    shape2Knob.showValue(true);
    shape2Knob.addListener(listener);
    Component::addAndMakeVisible(shape2Knob);
    shape2Knob.setBoundsRelative(0.377f, 0.6f, 0.2459f, 0.4f);

    panKnob.setRange(audioProcessor.treestate.getParameter("OSC_A_PAN")->getNormalisableRange().start, audioProcessor.treestate.getParameter("OSC_A_PAN")->getNormalisableRange().end, 0.0f);
    panKnob.addListener(listener);
    Component::addAndMakeVisible(panKnob);
    panKnob.setBoundsRelative(0.7541f, 0.6f, 0.2459f, 0.4f);
    //=====
};

Component_Oscillator::~Component_Oscillator() {
};

void Component_Oscillator::paint(juce::Graphics& g) {
};

void Component_Oscillator::resized() {
    octKnob.setBoundsRelative(0.0f, 0.0f, 0.2459f, 0.4f);
    shape1Knob.setBoundsRelative(0.377f, 0.0f, 0.2459f, 0.4f);
    phaseKnob.setBoundsRelative(0.7541f, 0.0f, 0.2459f, 0.4f);
    tuneKnob.setBoundsRelative(0.0f, 0.6f, 0.2459f, 0.4f);
    shape2Knob.setBoundsRelative(0.377f, 0.6f, 0.2459f, 0.4f);
    panKnob.setBoundsRelative(0.7541f, 0.6f, 0.2459f, 0.4f);
}

void Component_Oscillator::connectToTreestate(String prepend) {
    octKnobAttachment = std::unique_ptr<SliderParameterAttachment>(new SliderParameterAttachment((*audioProcessor.treestate.getParameter(prepend + "_OCTAVE")), octKnob, nullptr));
    shape1KnobAttachment = std::unique_ptr<SliderParameterAttachment>(new SliderParameterAttachment((*audioProcessor.treestate.getParameter(prepend + "_SHAPE1")), shape1Knob, nullptr));
    phaseKnobAttachment = std::unique_ptr<SliderParameterAttachment>(new SliderParameterAttachment((*audioProcessor.treestate.getParameter(prepend + "_PHASE")), phaseKnob, nullptr));
    tuneKnobAttachment = std::unique_ptr<SliderParameterAttachment>(new SliderParameterAttachment((*audioProcessor.treestate.getParameter(prepend + "_TUNE")), tuneKnob, nullptr));
    shape2KnobAttachment = std::unique_ptr<SliderParameterAttachment>(new SliderParameterAttachment((*audioProcessor.treestate.getParameter(prepend + "_SHAPE2")), shape2Knob, nullptr));
    panKnobAttachment = std::unique_ptr<SliderParameterAttachment>(new SliderParameterAttachment((*audioProcessor.treestate.getParameter(prepend + "_PAN")), panKnob, nullptr));
};