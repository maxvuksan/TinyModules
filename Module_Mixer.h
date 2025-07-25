#pragma once
#include "Module.h"

class Module_Mixer : public Module {

	public:
		Module_Mixer();

		void Prepare(double sampleRate, int blockSize) override;
		void Process() override;

		void UnSoloAll();

	private:

		int soloChannel; // which channel is solo'd
		float gainRamps[7] = { 0 }; // used to smoothly lerp to correct gain level

};