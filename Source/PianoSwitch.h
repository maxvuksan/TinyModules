#pragma once
#include <JuceHeader.h>



class PianoSwitch : public juce::Component {

	public:

		PianoSwitch();

		juce::var Serialize();
		void Deserialize(const juce::var&);

		void paint(juce::Graphics& g);
		void mouseDown(const juce::MouseEvent& e);

		bool GetKeyState(int index);
		void SetHighlightedKey(int index);
		
		/*
			@returns	the number of channels data was written to (e.g if 3 notes pressed, 3 channels down)
		
			@param		writeBuffer		the output buffer we are writing to, we can assume this is polyphonic (16 channels)
			@param		sampleRate		the number of samples to process
		*/
		int WriteVoltagesToPolyphonicBuffer(juce::AudioBuffer<float>& writeBuffer, int numberOfSamples, float vOctShift = 0);

	private:

		void ComputeKeyStates();

		std::vector<float> cachedVoltages;

		int highlightedKeyIndex = -1;		// indexed into either white or black keys
		bool highlightedIsWhiteKey = false;

		bool keyStates[12] = { false };

		bool whiteKeys[7] = { false }; // C D E F G A B
		bool blackKeys[5] = { false }; // C#, D#, F#, G#, A#

		juce::Rectangle<float> whiteKeyRects[7];
		juce::Rectangle<float> blackKeyRects[5];
};	