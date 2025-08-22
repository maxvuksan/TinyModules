
#pragma once
#include <JuceHeader.h>
#include "TextList.h"
#include "CustomLookAndFeel.h"
#include "SearchBar.h"

/*
	Acts as a module browser (select a new module to add to rack)
*/
class Popup_ModuleBrowser : public juce::Component {

	public:

		Popup_ModuleBrowser();

		void paint(juce::Graphics& g);
		void resized();

		void FilterModules(const juce::String& searchQuery);

	private:

		std::vector<juce::String> filteredModules;

		SearchBar searchBar;
		TextList textList;
		juce::ListBox listBox;


};