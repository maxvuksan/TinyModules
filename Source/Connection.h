#pragma once
#include <JuceHeader.h>

class Module;

enum ConnectionType {
	CONNECT_MONO,   // 1    channel
	CONNECT_POLY,   // 16   channels
};

/*
	Connections are data representations of the patched wires between modules
*/
struct Connection {

	ConnectionType connectionType;

	Module* outModule;
	int outSocketIndex;

	Module* inModule;
	int inSocketIndex;


	// introduce 1 sample delay to allow module graph to contain cycles?,
	//juce::AudioBuffer<float> delayBuffer;


	/* Two connections are equal if they have the same parameters */
	bool operator==(const Connection& other) const {
		return outModule == other.outModule &&
			outSocketIndex == other.outSocketIndex &&
			inModule == other.inModule &&
			inSocketIndex == other.inSocketIndex;
	}
};
