#pragma once
#include <JuceHeader.h>
#include "Module.h"
#include "Globals.h"
#include "ModuleType.h"
#include "WireManager.h"
#include "CustomLookAndFeel.h"
#include "FloatingBlockComponent.h"
#include "ProcessingManager.h"

struct RackModule {

    int width;
    Module* blockReference;  // if blockReference == nullptr, this is a free space
    bool occupiedByBlock;   // if a rack module extends into this spot, we mark it as occupied (to prevent overlap)
};

struct RackGridCell {
    RackModule* module;
};

class RackView : public juce::Component
{

    public:

        RackView();
        ~RackView();
        void paint(juce::Graphics&) override;
        void resized() override;

        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

        // enables/disables the floating block footprint, floating block refers to the ui shown when a block is moving positions
        void setFloatingBlockFootprintShow(bool shouldShow);
        void setFloatingBlockFootprint(juce::Point<int> newRackPosition, int widthUnits);

        /*
            creates a new module at a desired position, if the position is full, attempts to find nearby valid positions,
            memory for the block is first searched for in blockInactivePool, otherwise new memory is allocated.
        */
        void createModule(int idealX, int idealY, ModuleType type);
        void moveModule(int originalX, int originalY, int newX, int newY);
        // @returns true if a space is free for a block to be moved into
        bool canMoveModule(int newX, int newY, const RackModule& module);
        // removes a block from the rack, stores the memory in blockInactivePool vector for potential future reuse.
        void deleteModule(int x, int y);


        // grid index (for 1D layout)
        int GI(int x, int y) { return x + y * GLOBAL_RACK_WIDTH; }

        static RackView* instance;
        static ProcessingManager processingManager;

        const std::vector<RackModule>& getGrid() { return grid; }

        Module* outputBlock = nullptr;

    private:
        
        CustomLookAndFeel customLookAndFeel;

        std::vector<RackModule> grid;

        //juce::TooltipWindow tooltipWindow;
        WireManager wireManager;

        FloatingBlockComponent floatingBlock;
        juce::Point<int> floatingBlockCoordinate;
        int floatingBlockWidthUnits;

        juce::Rectangle<int> rackBounds;
        juce::Point<float> lastMousePos;
        juce::Point<float> panOffset{ 0, 0 };

        float zoom = 1.0f;
};
