#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

class FloatingBlockComponent : public juce::Component
{

    public:

        void paint(juce::Graphics& g) override
        {
            g.setColour(CustomLookAndFeel::GetTheme()->colour_floatingBlock);
            g.fillRect(getLocalBounds());
        }

};
