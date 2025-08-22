#pragma once
#include <JuceHeader.h>
#include "Module.h"
#include "Globals.h"
#include "ModuleType.h"
#include "WireManager.h"
#include "CustomLookAndFeel.h"
#include "ProcessingManager.h"

#include "FloatingBlockComponent.h"
#include "SelectionComponent.h"


enum RackLODFactor {

    LOD_CLOSE,
    LOD_MID,
    LOD_FAR,
};

struct RackModule {

    int width;
    std::unique_ptr<Module> blockReference;  // if blockReference == nullptr, this is a free space
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

        /*
            @returns the current LOD factor of the rack, determines the quality module should be rendered at
        */
        RackLODFactor GetLODFactor();

        /*
            Reset the state of the rack, called when a new patch is loaded
        */
        void Reset();

        /*
            JUCE overrides...
        */
        void paint(juce::Graphics&) override;
        void resized() override;

        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;

        void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
        void UpdateTransform();

        void ClearSelections();
        void CalculateSelectedModules();
        void MoveSelectedModules();

        /* 
            enables / disables the floating block footprint, floating block refers to the ui shown when a block is moving positions
        */
        void SetFloatingBlockFootprintShow(bool shouldShow);
        void SetFloatingBlockFootprint(juce::Point<int> newRackPosition, int widthUnits);

        /*
            creates a new module at a desired position, if the position is full, attempts to find nearby valid positions,
            memory for the block is first searched for in blockInactivePool, otherwise new memory is allocated.
        */
        Module* CreateModule(int idealX, int idealY, ModuleType type);
        /*
            @param moduleCacheName  the name of the module in the module Cache
        */
        void CreateModuleFromBrowser(const juce::String& moduleCacheName);
        void MoveModule(int originalX, int originalY, int newX, int newY);
        
        /*
            @returns true if a space is free for a block to be moved into
        */
        bool CanMoveModule(int newX, int newY, const RackModule& module);
        
        /* 
            removes a block from the rack.TODO: store the memory in blockInactivePool vector for potential future reuse.
        */
        void DeleteModule(int x, int y);

        /* 
            grid index(for 1D layout)
        */
        int GI(int x, int y) { return x + y * GLOBAL_RACK_WIDTH; }

        static RackView* instance;
        static ProcessingManager processingManager;

        const std::vector<RackModule>& getGrid() { return grid; }

        Module* outputBlock = nullptr;

    private:

        RackLODFactor lodFactor;
        CustomLookAndFeel customLookAndFeel;

        std::vector<RackModule> grid;

        WireManager wireManager;

        SelectionComponent          selection;
        juce::Point<float>          initalSelectionPos;
        juce::Point<int>            initalSelectionMouseOffset;
        std::vector<Module*>        selectedModules;
        juce::Point<int>            selectionCoordinateOrigin;  // the grid coordinates of the selections local 0,0
        
        bool                        movingSelectedModules; // used to determine if we are panning or moving modules


        FloatingBlockComponent      floatingBlock;
        juce::Point<int>            floatingBlockCoordinate;
        int                         floatingBlockWidthUnits;

        juce::Rectangle<int>        rackBounds;
        juce::Point<int>            lastMousePos;

        juce::AffineTransform       view; // store transformations
        float zoom;
};
