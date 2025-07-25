
#pragma once
#include <JuceHeader.h>

class Switch : public juce::Component {

	public:

		Switch();

		void SetInnerLabel(const std::string& innerLabel);

		void paint(juce::Graphics& g);
		void mouseDown(const juce::MouseEvent& e);

		bool GetState() const;
		void SetState(bool newState);

		void SetStateRecentlyEstablished(bool state);
		bool GetStateRecentlyEstablished();

	private:

		std::string innerLabel;
		bool state;
		bool stateRecentlyEstablished; // used to determine if the state has been updated recently, the parent module will mark this as false 
};