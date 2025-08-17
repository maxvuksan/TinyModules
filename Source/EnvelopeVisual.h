#pragma once
#include <JuceHeader.h>
#include "DSPUtility.h"

/*
	A visualation of ADSR envelopes
*/
class EnvelopeVisual : public juce::Component {

public:

	EnvelopeVisual();

	void resized();
	void paint(juce::Graphics& g);

	/*
		sets the controls for the envelope, the envelope path will be recalculated
	*/
	void SetControls(float attack, float sustain, float decay, float release);

private:
	float attackTime, decayTime, sustainLevel, releaseTime;

	juce::Path envelopePath;

	void rebuildPath();

};