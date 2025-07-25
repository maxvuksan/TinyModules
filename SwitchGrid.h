
#pragma once
#include <JuceHeader.h>


class SwitchGrid : public juce::Component {

	public:

		SwitchGrid(int width, int height);

		/*
			draws a highlight on a specific switch,

			@param x	- x coordinate of switch
			@param y	- y coordinate of switch

			if the coordinate is out of bounds, no switch will be highlighted.
		*/
		void SetHighlighted(int x, int y);
		// draws a outline on a specific switch
		void SetOutlined(int x, int y);

		/*
			@returns the state of a switch (is it on or off),
			
			Does not perform a bounds check, ensure coordinates are in vector bounds
		*/
		bool GetSwitchState(int x, int y);
		bool GetSwitchRecentlyInteractedState(int x, int y);
		void SetSwitchRecentlyInteractedState(int x, int y, bool state);

		void paint(juce::Graphics& g);
		void mouseDown(const juce::MouseEvent& e);


		// on click callback
		std::function<void(int x, int y, SwitchGrid* grid)> onClick;

	private:
		std::vector<std::vector<bool>> switches;
		std::vector<std::vector<bool>> switchRecentlyInteracted;

		int highlightX = -1;
		int highlightY = -1;
		
		int outlineX = -1;
		int outlineY = -1;
};