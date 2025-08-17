#include "NewModulePopup.h"
#include "ModuleCache.h"
#include "RackView.h"

NewModulePopup* NewModulePopup::instance;

NewModulePopup::NewModulePopup() {

	instance = this;

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


void NewModulePopup::paint(juce::Graphics& g) {

	g.setColour(CustomLookAndFeel::GetTheme()->colour_headerBar);
	g.fillRect(getLocalBounds());
}

void NewModulePopup::resized() {

	searchBar.setBounds(getLocalBounds().reduced(20).removeFromTop(25));
	listBox.setBounds(getLocalBounds().reduced(20).withTrimmedTop(60).removeFromLeft(300));
}

void NewModulePopup::FilterModules(const juce::String& searchQuery) {

	filteredModules.clear();

	for (auto& modData : ModuleCache::GetAllModuleData()) {

		juce::String modName(modData.first);

		if (modName.containsIgnoreCase(searchQuery)) {
			// search matches this module...

			filteredModules.push_back(modName);
		}
	}

	textList.SetItems(filteredModules);
	listBox.updateContent();
	listBox.repaint();
}


void NewModulePopup::SetOpenState(bool state) {
	instance->open = state;
	instance->setVisible(state);
	instance->setWantsKeyboardFocus(state);
}

NewModulePopup* NewModulePopup::GetInstance() {
	return instance;
}

bool NewModulePopup::GetOpenState() {
	return instance->open;
}