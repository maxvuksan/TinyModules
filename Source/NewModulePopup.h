
#pragma once
#include <JuceHeader.h>
#include "TextList.h"
#include "CustomLookAndFeel.h"
#include "SearchBar.h"

/*
	Acts as a module browser (select a new module to add to rack)
*/
class NewModulePopup : public juce::Component {

	public:

		NewModulePopup();

		void paint(juce::Graphics& g);
		void resized();

		void FilterModules(const juce::String& searchQuery);


		static void SetOpenState(bool state);
		static bool GetOpenState();
		static NewModulePopup* GetInstance();

	private:

		bool open;
		static NewModulePopup* instance;
		
		std::vector<juce::String> filteredModules;

		SearchBar searchBar;
		TextList textList;
		juce::ListBox listBox;


};