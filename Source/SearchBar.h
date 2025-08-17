#pragma once
#include <JuceHeader.h>


class SearchBar : public juce::Component
{

    // this is silly... just use TextEditor directly

    public:
    
        SearchBar();

        void resized() override;

        bool keyPressed(const juce::KeyPress& key);

        /*
            is called whenever the text in the bar changes
        */
        std::function<void(const juce::String&)> onSearchChanged;

    private:
        juce::TextEditor searchBox;

};
