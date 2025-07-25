#pragma once
#include "Module.h"

class Module_VCA : public Module {

	public:
		Module_VCA();

		void Prepare(double sampleRate, int blockSize);
		void Process();

	private:
};