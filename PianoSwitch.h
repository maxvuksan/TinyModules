#pragma once
#include <JuceHeader.h>

class PianoSwitch : public juce::Component {

	public:

		PianoSwitch();

		void paint(juce::Graphics& g);
		void mouseDown(const juce::MouseEvent& e);

		bool GetKeyState(int index);
		void SetHighlightedKey(int index);

	private:

		void ComputeKeyStates();

		int highlightedKeyIndex = -1;		// indexed into either white or black keys
		bool highlightedIsWhiteKey = false;

		bool keyStates[12] = { false };

		bool whiteKeys[7] = { false }; // C D E F G A B
		bool blackKeys[5] = { false }; // C#, D#, F#, G#, A#

		juce::Rectangle<float> whiteKeyRects[7];
		juce::Rectangle<float> blackKeyRects[5];
};