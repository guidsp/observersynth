#pragma once
//Set this to reset pole positions to default.
constexpr bool resetPoles = false; 
//Maximum allowed voices.
constexpr auto maximumVoices = 16;
//Number of modulation sources.
constexpr auto nOfModulators = 4;
//Number of modulation destinations.
constexpr auto nOfModulationDestinations = 35;
//Number of smoothed parameters (not modulated).
constexpr auto nOfSmoothedParams = 6;

//Dimensions of observer image data matrix
constexpr auto observerWidth = 517;
constexpr auto observerHeight = 335;