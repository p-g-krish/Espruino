## C-step-counter

This port of the algorithm was developed within the thesis *Open-source algorithm for wearables in healthcare-applications* made by Anna Brondin & Marcus Nordstrom at Malmö University.

## Guide

You may need to optimise constants used in the algoritm which may depend on the actual accelerometer used.

 - The constant timeScalingFactor in preProcessingStage.c is used to scale the timestamps if they are not in ms.
 - You can activate interpolation in preProcessingStage.c if you think that sampling frequency has jitter.
 - The coefficients of filterStage.c (`int32_t coeffs`) depend on the sampling frequency (here supposed to be 100Hz). These coefficients are used in a FIR filter to remove unwanted frequencies (we used a low pass filter with a cut-off frequency of 3Hz). You can use [this online tool](http://t-filter.engineerjs.com/) to compute different coefficients.
 - There are 3 constants that can be optimised in the algorithm. This is the suggested procedure:
   1. Walk 150 steps (count them manually) while collecting raw accelerometry data into a CSV file formated as *time(ms), X, Y, Z*
   2. These raw data should be collected multiple times and in different conditions (e.g. different walking speeds, styles, different terrains etc.)
   3. Find the best constants with [C-optimize-variables](https://github.com/MarcusNordstrom/C-optimize-variables)
   4. Modify the constants in this algorithm, for that, use the functions: `changeWindowSize()`, `changeDetectionThreshold()` and `changeTimeThreshold()`.
 - Build the library with cmake or add the library into your project
