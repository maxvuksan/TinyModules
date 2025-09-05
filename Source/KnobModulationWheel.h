#pragma once
#include <JuceHeader.h>

class KnobModulationWheel : public juce::Component
{

    public:
        KnobModulationWheel();

        // value in [-1.0, 1.0]  (maps to angle in [-pi, +pi])
        void SetValue(float v, juce::NotificationType nt = juce::sendNotification);
        float GetValue() const noexcept { return value; }

        // callback to reach to value
        std::function<void(float)> onValueChanged;

        // painting
        void paint(juce::Graphics& g) override;

        // interaction (rotary vertical drag style)
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void mouseDoubleClick(const juce::MouseEvent& e) override;

        void SetWireColourIndex(int index);

    private:

        int wireColourIndex = 0;

        float value = 0.0f;                  // [-1..1]
        float startDragValue = 0.0f;

        juce::Point<int> dragStartPos{};

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KnobModulationWheel)
};
