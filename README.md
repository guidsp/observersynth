# Observer Synth

Observer is a polyphonic plugin synth featuring four image based modulation sources. Each modulation source consists of a draggable "pole" over an image (chosen by the user and shared by all four poles) around which rotate several cursors - one for each synth voice. Each cursor reads the brightness of the pixel it's over and generates a signal that can be used to modulate a variety of modulation destinations.

This is my first time coding a synth so bear with the noob comments describing every line of code.

# Build
The source can be compiled with JUCE: https://github.com/juce-framework/JUCE (latest version as of writing is 7.0.2). The Projucer includes necessary modules.

NOTE: The juce_audio_utils and juce_audio_devices modules are only necessary for compiling AU and standalone builds.

# Video w/ sound
https://user-images.githubusercontent.com/84092763/206265045-de866944-669c-4e7c-9844-09def903030e.mp4
