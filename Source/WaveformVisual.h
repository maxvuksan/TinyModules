#pragma once
#include <JuceHeader.h>
#include "DSPUtility.h"

/*
	A visualation of primitive waveforms
*/
class WaveformVisual : public juce::Component {

	public:

		WaveformVisual();

		void resized();
		void paint(juce::Graphics& g);

		void mouseDown(const juce::MouseEvent& e);

		void SetWaveType(DSP::WaveType waveType);
		DSP::WaveType GetWaveType();

	private:
		DSP::WaveType currentType{};
		juce::Path waveformPath;

		void rebuildPath();

};