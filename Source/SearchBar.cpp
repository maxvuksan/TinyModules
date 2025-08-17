
#include "SearchBar.h"


SearchBar::SearchBar()
{
    addAndMakeVisible(searchBox);
    searchBox.setTextToShowWhenEmpty("Search...", juce::Colours::grey);

    searchBox.onTextChange = [this]()
        {
            if (onSearchChanged){
                onSearchChanged(searchBox.getText());
            }
    };
}

bool SearchBar::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::escapeKey)
    {
        // Remove focus from searchBox
        searchBox.setFocusContainer(true); 
        searchBox.giveAwayKeyboardFocus();

        if (auto* parent = getParentComponent()) {
            parent->grabKeyboardFocus();
        }

        return true;
    }

    return false;
}


void SearchBar::resized()
{
    searchBox.setBounds(getLocalBounds());
}