#pragma once
#include <JuceHeader.h>
#include "DSPUtility.h"

/*
	A visualation for both ADSR envelopes and filters
*/
class EnvelopeVisual : public juce::Component {

public:

	EnvelopeVisual();

	void resized();
	void paint(juce::Graphics& g);

	/*
		sets the controls for the envelope (recalutes path as a envelope), the envelope path will be recalculated
	*/
	void SetEnvelopeControls(float attack, float sustain, float decay, float release);

	/*
		sets the controls for a filter (recalautes the path as a filter)

		@param	cutoffFrequency		frequency we begin filter cut at
		@param	resonanceLevel			0-1	value of resonance notch
		@param	cutLow				are we cutting low (filter left of spectrum) otherwise we cut high
	*/
	void SetFilterControls(float cutoffFrequency, float resonanceLevel, bool cutLow);

private:

	float attackTime, sustainLevel, decayTime, releaseTime;

	float cutoffHz, resonance01;
	bool  cutLow = false;        // true = high-pass (cut low), false = low-pass (cut high)

	bool isEnvelope;

	juce::Path path;

	void RebuildEnvelopePath();
	void RebuildFilterPath();
};