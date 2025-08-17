#pragma once
#include <JuceHeader.h>

/*
    used to display a list of text entires in a scrolling rect
*/
class TextList : public juce::ListBoxModel
{
    public:

        void SetItems(const std::vector<juce::String>& items);

        
        // is called when we click on an item in the TextList
        std::function<void(const juce::String&)> onItemClicked;

        void listBoxItemClicked(int row, const juce::MouseEvent&) override;
        int getNumRows() override;
        void paintListBoxItem(int rowNumber, juce::Graphics& g,
            int width, int height, bool rowIsSelected) override;

    private:
        std::vector<juce::String> items;
};