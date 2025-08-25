#include "Popup_ModuleBrowser.h"
#include "ModuleCache.h"
#include "RackView.h"


Popup_ModuleBrowser::Popup_ModuleBrowser() {

	textList.SetItems({ });

	listBox.setColour(juce::ListBox::backgroundColourId,
		CustomLookAndFeel::GetTheme()->colour_backgroundRackEven);

	listBox.setModel(&textList);
	addAndMakeVisible(listBox);

	addAndMakeVisible(searchBar);

	textList.onItemClicked = [this](const juce::String& moduleName) {
		RackView::instance->CreateModuleFromBrowser(moduleName);
	};

	searchBar.onSearchChanged = [this](const juce::String& text) {
		FilterModules(text);
	};





}


void Popup_ModuleBrowser::paint(juce::Graphics& g) {

	g.setColour(CustomLookAndFeel::GetTheme()->colour_headerBar);
	g.fillRect(getLocalBounds());

	g.setColour(CustomLookAndFeel::GetTheme()->colour_knobPointer);
	g.drawRect(getLocalBounds());
}

void Popup_ModuleBrowser::resized() {

	searchBar.setBounds(getLocalBounds().reduced(20).removeFromTop(25));
	listBox.setBounds(getLocalBounds().reduced(20).withTrimmedTop(50));

	FilterModules(searchBar.GetValue());
}

void Popup_ModuleBrowser::FilterModules(const juce::String& searchQuery) {

	filteredModules.clear();

	for (auto& modData : ModuleCache::GetAllModuleData()) {

		juce::String modName(modData.first);

		if (searchQuery == "" || searchQuery == " " || modName.containsIgnoreCase(searchQuery)) {
			// search matches this module...

			filteredModules.push_back(modName);
		}
	}

	textList.SetItems(filteredModules);
	listBox.updateContent();
	listBox.repaint();
}
