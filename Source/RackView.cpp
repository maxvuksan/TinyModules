#include "RackView.h"
#include "Globals.h"
#include "ModuleIncludes.h"
#include "CustomLookAndFeel.h"
#include "ModuleCache.h"
#include "Popups.h"

RackView* RackView::instance = nullptr;
ProcessingManager RackView::processingManager;

RackView::RackView() //: tooltipWindow(this)
{   
    // there will only ever be a single Rack, so we can make this a singleton
    instance = this;
    setOpaque(true);
    
    lodFactor = LOD_CLOSE;

    Reset(); 

    setSize(
        GLOBAL_RACK_WIDTH * GLOBAL_BLOCK_WIDTH_UNIT,
        GLOBAL_RACK_HEGHT * (GLOBAL_BLOCK_HEIGHT + GLOBAL_BLOCK_HEIGHT_PADDING)
    );

    addAndMakeVisible(floatingBlock, 0);
    floatingBlock.setVisible(false);

    addAndMakeVisible(selection);

    addAndMakeVisible(wireManager);
    wireManager.setBounds(getLocalBounds());

    addAndMakeVisible(wireManager);
    wireManager.setBounds(getLocalBounds());
    

    ModuleCache::Init();

    CreateModule(0, 2, MOD_OUTPUT);
}

RackView::~RackView() {

}

RackLODFactor RackView::GetLODFactor() {
    return lodFactor;
}

void RackView::Reset() {

    wireManager.Reset();

    processingManager.SetProcessingEnabled(false);
    processingManager.Reset();


    grid.resize(GLOBAL_RACK_WIDTH * GLOBAL_RACK_HEGHT);

    for (auto& cell : grid) {

        if (cell.blockReference) {
            removeChildComponent(cell.blockReference.get());
        }
    }

    for (int x = 0; x < GLOBAL_RACK_WIDTH; x++) {
        for (int y = 0; y < GLOBAL_RACK_HEGHT; y++) {

            int i = GI(x, y);

            grid[i].width = 0;
            grid[i].blockReference = nullptr;
            grid[i].occupiedByBlock = false;
        }
    }

    zoom = 1.0f;


    repaint();

    processingManager.SetProcessingEnabled(true);

}


void RackView::SetFloatingBlockFootprintShow(bool shouldShow) {

    floatingBlock.setVisible(shouldShow);
}

void RackView::SetFloatingBlockFootprint(juce::Point<int> newRackPosition, int widthUnits) {

    floatingBlock.setBounds({
        newRackPosition.x * GLOBAL_BLOCK_WIDTH_UNIT,
        newRackPosition.y * (GLOBAL_BLOCK_HEIGHT + GLOBAL_BLOCK_HEIGHT_PADDING),
        widthUnits * GLOBAL_BLOCK_WIDTH_UNIT,
        GLOBAL_BLOCK_HEIGHT + GLOBAL_BLOCK_HEIGHT_PADDING
    });
}

Module* RackView::CreateModule(int idealX, int idealY, ModuleType type) {

    using ModuleFactory = std::function<std::unique_ptr<Module>()>;
    static const std::unordered_map<ModuleType, ModuleFactory> factoryMap = {

        { ModuleType::MOD_LFO,           [] { return std::make_unique<Module_LFO>();            } },
        { ModuleType::MOD_OUTPUT,        [] { return std::make_unique<Module_Output>();         } },
        { ModuleType::MOD_NOISE,         [] { return std::make_unique<Module_Noise>();          } },
        { ModuleType::MOD_OSCILLATOR,    [] { return std::make_unique<Module_Oscillator>();     } },
        { ModuleType::MOD_SCOPE,         [] { return std::make_unique<Module_Scope>();          } },
        { ModuleType::MOD_FILTER,        [] { return std::make_unique<Module_Filter>();         } },
        { ModuleType::MOD_ADSR,          [] { return std::make_unique<Module_ADSR>();           } },
        { ModuleType::MOD_SEQUENCER,     [] { return std::make_unique<Module_Sequencer>();      } },
        { ModuleType::MOD_CHORD_SEQUENCER,[] { return std::make_unique<Module_ChordSequencer>();} },
        { ModuleType::MOD_VCA,           [] { return std::make_unique<Module_VCA>();            } },
        { ModuleType::MOD_REVERB,        [] { return std::make_unique<Module_Reverb>();         } },
        { ModuleType::MOD_CLOCK_DIVIDE,  [] { return std::make_unique<Module_ClockDivide>();    } },
        { ModuleType::MOD_VOLT_QUANTIZER,[] { return std::make_unique<Module_VoltQuantizer>();  } },
        { ModuleType::MOD_VOLT_KNOBS,    [] { return std::make_unique<Module_VoltKnobs>();      } },
        { ModuleType::MOD_MIXER,         [] { return std::make_unique<Module_Mixer>();          } },
    };


    auto it = factoryMap.find(type);

    if (it == factoryMap.end()) {
        jassert(false);
    }

    std::unique_ptr<Module> newBlock = it->second();
    newBlock->SetModuleType(type);

    auto& cell = grid[GI(idealX, idealY)];
    cell.blockReference = std::move(newBlock); // Transfer ownership to the grid
    cell.width = cell.blockReference->GetWidthUnits();

    for (int x = 0; x < cell.width; ++x) {
        grid[GI(idealX + x, idealY)].occupiedByBlock = true;
    }

    addAndMakeVisible(cell.blockReference.get(), 1);

    cell.blockReference->SetRackPosition(idealX, idealY);
    cell.blockReference->resized();
    cell.blockReference->Reset();
    processingManager.AddModule(cell.blockReference.get());

    if (type == MOD_OUTPUT) {
        outputBlock = cell.blockReference.get();
    }

    return cell.blockReference.get();
}

void RackView::CreateModuleFromBrowser(const juce::String& moduleName) {

    const ModuleData& data =  ModuleCache::GetModuleData(moduleName.toStdString());
    CreateModule(1, 1, (ModuleType)data.moduleType);
    Popups::ClosePopup();
}

void RackView::MoveModule(int originalX, int originalY, int newX, int newY) {

    RackModule& _module = grid[GI(originalX, originalY)];

    if (_module.blockReference == nullptr) {
        return;
    }

    if (!CanMoveModule(newX, newY, _module)) {
        return;
    } 

    for (int x = 0; x < grid[GI(originalX, originalY)].width; x++) {
        grid[GI(originalX + x, originalY)].occupiedByBlock = false;
    }
    grid[GI(originalX, originalY)].width = 0;

    grid[GI(newX, newY)].blockReference = std::move(_module.blockReference);
    grid[GI(originalX, originalY)].blockReference = nullptr;

    grid[GI(newX, newY)].width = grid[GI(newX, newY)].blockReference->GetWidthUnits();
    grid[GI(newX, newY)].blockReference->SetRackPosition(newX, newY);
    grid[GI(newX, newY)].blockReference->resized();

    for (int x = 0; x < grid[GI(newX, newY)].width; x++) {
        grid[GI(newX + x, newY)].occupiedByBlock = true;
    }

}

bool RackView::CanMoveModule(int newX, int newY, const RackModule& module) {
    if (newX >= 0 && newX + module.width <= GLOBAL_RACK_WIDTH &&
        newY >= 0 && newY + 1 <= GLOBAL_RACK_HEGHT)
    {
        for (int x = 0; x < module.width; x++) {
            
            const auto& cell = grid[GI(newX + x, newY)];

            // a block exists at this location
            if (cell.blockReference != nullptr || cell.occupiedByBlock) {
                return false;
            }
        }

        return true;
    }

    return false;
}

void RackView::DeleteModule(int x, int y) {

    // can only delete non null
    jassert(grid[GI(x, y)].blockReference != nullptr);

    removeChildComponent(grid[GI(x, y)].blockReference.get());

    grid[GI(x, y)].blockReference = nullptr;
    grid[GI(x, y)].occupiedByBlock = false;
    grid[GI(x, y)].width = 0;

    repaint();
}

void RackView::SetAudioConfig(const AudioConfig& config) {
    this->audioConfig = config;
}

const AudioConfig& RackView::GetAudioConfig() {
    return this->audioConfig;
}




void RackView::paint(juce::Graphics& g)
{
    g.setColour(CustomLookAndFeel::GetTheme()->colour_backgroundRackOdd);
    g.fillRect(rackBounds.toFloat());

    // Draw horizontal grid lines
    for (int y = 0; y < rackBounds.getHeight(); y += (GLOBAL_BLOCK_HEIGHT + GLOBAL_BLOCK_HEIGHT_PADDING)) {

        if ((y / (GLOBAL_BLOCK_HEIGHT + GLOBAL_BLOCK_HEIGHT_PADDING)) % 2 == 0)
        {
            g.setColour(CustomLookAndFeel::GetTheme()->colour_backgroundRackEven);
            g.fillRect(0, y, rackBounds.getWidth(), (GLOBAL_BLOCK_HEIGHT + GLOBAL_BLOCK_HEIGHT_PADDING));
        }

        g.setColour(CustomLookAndFeel::GetTheme()->colour_backgroundRackLines);
        g.drawLine(0.0f, (float)y, (float)rackBounds.getWidth(), (float)y);


    }
}

void RackView::resized()
{
    rackBounds = juce::Rectangle<int>(
        0, 0,
        GLOBAL_RACK_WIDTH * GLOBAL_BLOCK_WIDTH_UNIT,
        GLOBAL_RACK_HEGHT * GLOBAL_BLOCK_HEIGHT
    );
}

void RackView::mouseDown(const juce::MouseEvent& e)
{
    // dont continue if module browser is open...
    if (Popups::IsPopupOpen()) {
        return;
    }


    if (e.mods.isMiddleButtonDown()) {

        movingSelectedModules = false;

        int gridX = (int)(std::floor(e.position.x / (float)GLOBAL_BLOCK_WIDTH_UNIT));
        int gridY = (int)(std::floor(e.position.y / (float)(GLOBAL_BLOCK_HEIGHT + GLOBAL_BLOCK_HEIGHT_PADDING)));

        // Check if mouse is inside any selected module
        for (auto* module : selectedModules)
        {
            if (module != nullptr)
            {
                auto bounds = module->getBounds();
                if (bounds.contains(e.getPosition().toInt()))
                {
                    movingSelectedModules = true;
                    break;
                }
            }
        }

        // we did not hit one of our selected modules, 
        if (!movingSelectedModules) {
            // Clamp to grid bounds
            if (gridX >= 0 && gridX < GLOBAL_RACK_WIDTH &&
                gridY >= 0 && gridY < GLOBAL_RACK_HEGHT)
            {
                RackModule* cell = &grid[GI(gridX, gridY)];

                // step left until we find the first cell of the module
                while (cell->occupiedByBlock && cell->blockReference == nullptr && gridX > 0) {
                    gridX--;
                    cell = &grid[GI(gridX, gridY)];
                }

                if (cell->blockReference){
                
                    ClearSelections();
                    cell->blockReference->SetSelected(true);
                    cell->blockReference->SetGridOffsetOnSelection(0, 0);
                    selectedModules.push_back(cell->blockReference.get());

                    selectionCoordinateOrigin = { gridX, gridY };
                    movingSelectedModules = true;
                }
            }
        }

        initalSelectionMouseOffset =
            selectionCoordinateOrigin *
            juce::Point<int>(GLOBAL_BLOCK_WIDTH_UNIT, GLOBAL_BLOCK_HEIGHT + GLOBAL_BLOCK_HEIGHT_PADDING) -
            juce::Point<int>{ e.getPosition().x, e.getPosition().y };

        lastMousePos = e.getScreenPosition();
    }
    else if (e.mods.isLeftButtonDown())
    {
        float screenX = e.position.x;
        float screenY = e.position.y;

        initalSelectionPos = { screenX, screenY };

        ClearSelections();


    }

}

void RackView::mouseDrag(const juce::MouseEvent& e)
{
    if (Popups::IsPopupOpen()) {
        return;
    }

    if (e.mods.isLeftButtonDown())
    {
        float screenX = e.position.x;
        float screenY = e.position.y;

        float rx = std::min(initalSelectionPos.x, screenX);
        float ry = std::min(initalSelectionPos.y, screenY);
        float rw = std::abs(screenX - initalSelectionPos.x);
        float rh = std::abs(screenY - initalSelectionPos.y);

        selection.setBounds(juce::roundToInt(rx), juce::roundToInt(ry),
            juce::roundToInt(rw), juce::roundToInt(rh));
    }
    else if (e.mods.isMiddleButtonDown()) {

        auto currentPos = e.position.roundToInt();

        // dragging selected modules
        if (movingSelectedModules) {

            currentPos += initalSelectionMouseOffset;

            for (int i = 0; i < selectedModules.size(); i++) {

                juce::Point<int> pos =
                    currentPos.roundToInt() +
                    (selectedModules[i]->GetGridOffsetOnSelection()) *
                    juce::Point<int>(GLOBAL_BLOCK_WIDTH_UNIT, GLOBAL_BLOCK_HEIGHT + GLOBAL_BLOCK_HEIGHT_PADDING);

                selectedModules[i]->setTopLeftPosition(pos);
                
                // ensure wires are updated...
                for (auto& socket : selectedModules[i]->GetSockets()) {
                    socket.second.second->RecomputeWireGraphics();
                }
                for (auto& knob : selectedModules[i]->GetKnobs()) {
                    knob.second.second->RecomputeWireGraphics();
                }
            }
        }
        else {

            auto   current = e.getScreenPosition();
            auto   delta = current - lastMousePos;           // screen-px
            view = view.translated((float)delta.x,
                (float)delta.y);              // screen-px
            lastMousePos = current;

            UpdateTransform();
        }
    
    }

}

void RackView::mouseUp(const juce::MouseEvent& e) {

    if (Popups::IsPopupOpen()) {
        Popups::ClosePopup();
        return;
    }
    else {
        if (e.mods.isRightButtonDown()) {
            Popups::OpenPopup(POPUP_BROWSER);
        }
    }



    if (e.mods.isMiddleButtonDown()) {
        MoveSelectedModules();
    }
    else {
        CalculateSelectedModules();
    }


}

void RackView::mouseWheelMove(const juce::MouseEvent& e,
    const juce::MouseWheelDetails& wheel)
{
    if (!e.mods.isCtrlDown()) return;

    const float factor = wheel.deltaY > 0 ? 1.10f : 0.90f;
    zoom *= factor;

    DBG(view.getScaleFactor());

    if (view.getScaleFactor() > 0.66) {
        lodFactor = LOD_CLOSE;
    }
    else {
        lodFactor = LOD_MID;
    }





    view = juce::AffineTransform::scale(factor, factor,
        e.position.x, e.position.y)
        .followedBy(view);      

    UpdateTransform();
}

void RackView::UpdateTransform() {
    setTransform(view);
}

void RackView::ClearSelections() {
    
    for (int i = 0; i < selectedModules.size(); i++) {
        selectedModules[i]->SetSelected(false);
    }
    
    selectedModules.clear();

}

void RackView::MoveSelectedModules() {

    for (int i = 0; i < selectedModules.size(); i++) {

        selectedModules[i]->SetSelected(false);

        juce::Point<int> newRackPosition;

        newRackPosition.x = round(selectedModules[i]->getPosition().x / (float)GLOBAL_BLOCK_WIDTH_UNIT);
        newRackPosition.y = round(selectedModules[i]->getPosition().y / (float)(GLOBAL_BLOCK_HEIGHT + GLOBAL_BLOCK_HEIGHT_PADDING));

        selectedModules[i]->setTopLeftPosition({
            newRackPosition.x * GLOBAL_BLOCK_WIDTH_UNIT,
            newRackPosition.y * (GLOBAL_BLOCK_HEIGHT + GLOBAL_BLOCK_HEIGHT_PADDING)
        });

        for (auto& socket : selectedModules[i]->GetSockets()) {
            socket.second.second->RecomputeWireGraphics();
        }
        for (auto& knob : selectedModules[i]->GetKnobs()) {
            knob.second.second->RecomputeWireGraphics();
        }

        MoveModule( selectedModules[i]->GetRackPosition().x,
                    selectedModules[i]->GetRackPosition().y,
                    newRackPosition.x,
                    newRackPosition.y
        );

    }
    selectedModules.clear();
}

void RackView::ReactToCanConnectToKnobs(bool canConnectToKnobs) {

    repaint();
}


void RackView::CalculateSelectedModules() {

    selectedModules.clear();

    // Convert selection bounds to logical space
    auto screenSelection = selection.getBounds();

    float logicalX = screenSelection.getX();
    float logicalY = screenSelection.getY();
    float logicalW = screenSelection.getWidth();
    float logicalH = screenSelection.getHeight();

    juce::Rectangle<float> logicalSelection(logicalX, logicalY, logicalW, logicalH);

    constexpr float eps = 1e-4f;
    const float unitX = (float)GLOBAL_BLOCK_WIDTH_UNIT;
    const float unitY = (float)(GLOBAL_BLOCK_HEIGHT + GLOBAL_BLOCK_HEIGHT_PADDING);

    int gridX1 = std::clamp((int)std::floor(logicalSelection.getX() / unitX), 0, GLOBAL_RACK_WIDTH - 1);
    int gridY1 = std::clamp((int)std::floor(logicalSelection.getY() / unitY), 0, GLOBAL_RACK_HEGHT - 1);
    int gridX2 = std::clamp((int)std::floor((logicalSelection.getRight() - eps) / unitX), 0, GLOBAL_RACK_WIDTH - 1);
    int gridY2 = std::clamp((int)std::floor((logicalSelection.getBottom() - eps) / unitY), 0, GLOBAL_RACK_HEGHT - 1);

    selectionCoordinateOrigin.setX(gridX1);
    selectionCoordinateOrigin.setY(gridY1);


    for (int y = gridY1; y <= gridY2; ++y)
    {
        for (int x = gridX1; x <= gridX2; ++x)
        {   

            RackModule* cell = &grid[GI(x, y)];
            if (!cell->blockReference && !cell->occupiedByBlock) {
                continue;
            }
            int tempX = x;

            // shift left until we find origin of module
            while (cell->occupiedByBlock && cell->blockReference == nullptr && tempX > 0) {
                tempX--;
                cell = &grid[GI(tempX, y)];
            }

            if (cell->blockReference == nullptr) {
                continue;
            }

            cell->blockReference->SetGridOffsetOnSelection(tempX - gridX1, y - gridY1);
            cell->blockReference->SetSelected(true);

            // Add it
            selectedModules.push_back(cell->blockReference.get());
        }
    }

    selection.setSize(0, 0);
}

