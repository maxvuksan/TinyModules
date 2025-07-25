#include "Module_VoltKnobs.h"


Module_VoltKnobs::Module_VoltKnobs(): Module(2, "voltages") {

	KnobConfiguration config;
	config.min = -5.0f;         // For bipolar CV
	config.max = 5.0f;
	config.defaultValue = 0.0f;
	config.increment = 0.01f;

	Component_CreateKnob("1", 0, 0, &config);
	Component_CreateKnob("2", 0, 1, &config);
	Component_CreateKnob("3", 0, 2, &config);
	Component_CreateKnob("4", 0, 3, &config);
	Component_CreateKnob("5", 0, 4, &config);
	Component_CreateKnob("6", 0, 5, &config);

	Component_CreateOutputSocket("1", 1, 0);
	Component_CreateOutputSocket("2", 1, 1);
	Component_CreateOutputSocket("3", 1, 2);
	Component_CreateOutputSocket("4", 1, 3);
	Component_CreateOutputSocket("5", 1, 4);
	Component_CreateOutputSocket("6", 1, 5);
}

void Module_VoltKnobs::Prepare(double sampleRate, int blockSize) {
	Module::Prepare(sampleRate, blockSize, 0, 6);
}

void Module_VoltKnobs::Process() {

	// iterate over each input
	for (int i = 0; i < 6; i++) {

		float* writePtr = GetOutputWritePtr(i);
		float knobValue = Component_GetKnobValue(std::to_string(i + 1));

		for (int n = 0; n < GetOutputBuffer(0).getNumSamples(); n++) {

			writePtr[n] = knobValue;
		}

	}
}