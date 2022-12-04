# Observer Synth

Observer is a polyphonic plugin synth featuring four image based modulation sources. Each modulation source consists of a draggable "pole" over an image (chosen by the user and shared by all four poles) around which rotate several cursors - one for each synth voice. Each cursor reads the brightness of the pixel it's over and generates a signal that can be used to modulate a variety of modulation destinations.

This is the first synth I've coded, so bear with all the noob comments describing every other line of code.

# Build
The source can be compiled with JUCE: https://github.com/juce-framework/JUCE (latest version as of writing is 7.0.2). I recommend the Projucer 
