#pragma once
#include "Module.h"

class Module_Scope : public Module {

	public:
		Module_Scope();

		void Prepare(double sampleRate, int blockSize);
		void Process();

	private:
};