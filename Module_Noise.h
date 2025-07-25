#pragma once
#include "Module.h"

class Module_Noise : public Module {

	public:
		Module_Noise();

        void Prepare(double sampleRate, int blockSize) override;
        void Process();

	private:

        float GeneratePinkNoise();
        float GenerateBrownNoise();
        float GenerateBlueNoise();

        // Internal state for pink noise
        float pink_b0 = 0.0f;
        float pink_b1 = 0.0f;
        float pink_b2 = 0.0f;
        float pink_b3 = 0.0f;
        float pink_b4 = 0.0f;
        float pink_b5 = 0.0f;
        float pink_b6 = 0.0f;

        // Internal state for brown noise
        float brown_lastOutput = 0.0f;

        // Internal state for blue noise
        float blue_lastWhite = 0.0f;
};