#include "RackView.h"
#include "Globals.h"
#include "ModuleIncludes.h"
#include "CustomLookAndFeel.h"

RackView* RackView::instance = nullptr;
ProcessingManager RackView::processingManager;

RackView::RackView() //: tooltipWindow(this)
{   
    // there will only ever be a single Rack, so we can make this a singleton
    instance = this;


    setOpaque(true);

    setSize(
        GLOBAL_RACK_WIDTH * GLOBAL_BLOCK_WIDTH_UNIT,
        GLOBAL_RACK_HEGHT * (GLOBAL_BLOCK_HEIGHT + GLOBAL_BLOCK_HEIGHT_PADDING)
    );

    addAndMakeVisible(floatingBlock, 0);
    floatingBlock.setVisible(false);

    addAndMakeVisible(wireManager);
    wireManager.setBounds(getLocalBounds());


    grid.resize(GLOBAL_RACK_WIDTH * GLOBAL_RACK_HEGHT);

    for (int x = 0; x < GLOBAL_RACK_WIDTH; x++) {
        for (int y = 0; y < GLOBAL_RACK_HEGHT; y++) {

            grid[GI(x, y)].width = 0;
            grid[GI(x, y)].blockReference = nullptr;
        }
    }

    createModule(0, 2, MOD_OUTPUT);
    createModule(1, 1, MOD_NOISE);
    createModule(6, 0, MOD_SCOPE);
    createModule(3, 3, MOD_OSCILLATOR);
    createModule(3, 4, MOD_VOLT_KNOBS);
    createModule(3, 1, MOD_OSCILLATOR);
    createModule(7, 4, MOD_LFO);
    createModule(6, 5, MOD_FILTER);

    createModule(1, 3, MOD_VOLT_QUANTIZER);
    createModule(7, 3, MOD_VOLT_QUANTIZER);
    createModule(2, 5, MOD_CLOCK_DIVIDE);
    createModule(8, 2, MOD_CLOCK_DIVIDE);
    for (int i = 8; i < 15; i++) {
        createModule(i, 5, MOD_ADSR);
    }
    createModule(6, 4, MOD_SEQUENCER);
    createModule(5, 4, MOD_SEQUENCER);

    createModule(12, 4, MOD_MIXER);

    createModule(2, 3, MOD_VCA);
    createModule(8, 3, MOD_VCA);
    createModule(9, 3, MOD_VCA);
    createModule(6, 6, MOD_REVERB);
}

RackView::~RackView() {

    for (int x = 0; x < GLOBAL_RACK_WIDTH; x++) {
        for (int y = 0; y < GLOBAL_RACK_HEGHT; y++) {

            if (grid[GI(x, y)].blockReference != nullptr) {
                delete grid[GI(x, y)].blockReference;
            }
        }
    }
}


void RackView::setFloatingBlockFootprintShow(bool shouldShow) {
    floatingBlock.setVisible(true);
}

void RackView::setFloatingBlockFootprint(juce::Point<int> newRackPosition, int widthUnits) {

    floatingBlock.setBounds({
        newRackPosition.x * GLOBAL_BLOCK_WIDTH_UNIT,
        newRackPosition.y * (GLOBAL_BLOCK_HEIGHT + GLOBAL_BLOCK_HEIGHT_PADDING),
        widthUnits * GLOBAL_BLOCK_WIDTH_UNIT,
        GLOBAL_BLOCK_HEIGHT + GLOBAL_BLOCK_HEIGHT_PADDING
    });
}

void RackView::createModule(int idealX, int idealY, ModuleType type) {

    Module* newBlock;

    switch (type) {

        case ModuleType::MOD_LFO:
            newBlock = new Module_LFO();
            break;

        case ModuleType::MOD_OUTPUT:
            newBlock = new Module_Output();
            outputBlock = newBlock;
            break;

        case ModuleType::MOD_NOISE:
            newBlock = new Module_Noise();
            break;

        case ModuleType::MOD_OSCILLATOR:
            newBlock = new Module_Oscillator();
            break;

        case ModuleType::MOD_SCOPE:
            newBlock = new Module_Scope();
            break;

        case ModuleType::MOD_FILTER:
            newBlock = new Module_Filter();
            break;

        case ModuleType::MOD_ADSR:
            newBlock = new Module_ADSR();
            break;

        case ModuleType::MOD_SEQUENCER:
            newBlock = new Module_Sequencer();
            break;

        case ModuleType::MOD_VCA:
            newBlock = new Module_VCA();
            break;

        case ModuleType::MOD_REVERB:
            newBlock = new Module_Reverb();
            break;

        case ModuleType::MOD_CLOCK_DIVIDE:
            newBlock = new Module_ClockDivide();
            break;

        case ModuleType::MOD_VOLT_QUANTIZER:
            newBlock = new Module_VoltQuantizer();
            break;

        case ModuleType::MOD_VOLT_KNOBS:
            newBlock = new Module_VoltKnobs();
            break;

        case ModuleType::MOD_MIXER:
            newBlock = new Module_Mixer();
            break;
    }


    newBlock->SetModuleType(type);
    grid[GI(idealX, idealY)].blockReference = newBlock;
    grid[GI(idealX, idealY)].width = newBlock->GetWidthUnits();

    for (int x = 0; x < grid[GI(idealX, idealY)].width; x++) {
        grid[GI(idealX + x, idealY)].occupiedByBlock = true;
    }

    addAndMakeVisible(newBlock, 1);

    newBlock->SetRackPosition(idealX, idealY);
    newBlock->resized();
    newBlock->Reset();

    processingManager.AddModule(newBlock);
}

void RackView::moveModule(int originalX, int originalY, int newX, int newY) {

    RackModule& _module = grid[GI(originalX, originalY)];

    if (_module.blockReference == nullptr) {
        return;
    }

    if (!canMoveModule(newX, newY, _module)) {
        return;
    } 

    for (int x = 0; x < grid[GI(originalX, originalY)].width; x++) {
        grid[GI(originalX + x, originalY)].occupiedByBlock = false;
    }
    grid[GI(originalX, originalY)].width = 0;

    grid[GI(newX, newY)].blockReference = _module.blockReference;
    grid[GI(originalX, originalY)].blockReference = nullptr;

    grid[GI(newX, newY)].width = grid[GI(newX, newY)].blockReference->GetWidthUnits();
    grid[GI(newX, newY)].blockReference->SetRackPosition(newX, newY);
    grid[GI(newX, newY)].blockReference->resized();

    for (int x = 0; x < grid[GI(newX, newY)].width; x++) {
        grid[GI(newX + x, newY)].occupiedByBlock = true;
    }

}

bool RackView::canMoveModule(int newX, int newY, const RackModule& module) {
    if (newX >= 0 && newX + module.width <= GLOBAL_RACK_WIDTH &&
        newY >= 0 && newY + 1 <= GLOBAL_RACK_HEGHT)
    {
        for (int x = 0; x < module.width; x++) {
            
            // a block exists at this location
            if (grid[GI(newX + x, newY)].blockReference != nullptr) {
                return false;
            }
        }

        return true;
    }

    return false;
}
void RackView::deleteModule(int x, int y) {

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
    lastMousePos = juce::Point<float>{ (float)e.getScreenPosition().x, (float)e.getScreenPosition().y };
}

void RackView::mouseDrag(const juce::MouseEvent& e)
{
    auto currentPos = juce::Point<float>{ (float)e.getScreenPosition().x, (float)e.getScreenPosition().y };
    
    auto screenDelta = currentPos - lastMousePos;
    auto worldDelta = screenDelta / zoom;
    
    panOffset += worldDelta;
    lastMousePos = currentPos;

    setTransform(juce::AffineTransform::scale(zoom).translated((float)panOffset.x, (float)panOffset.y));
}

void RackView::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    if (e.mods.isCtrlDown())
    {
        float scaleDelta = wheel.deltaY > 0 ? 1.1f : 0.9f;
        zoom *= scaleDelta;
        setTransform(juce::AffineTransform::scale(zoom).translated((float)panOffset.x, (float)panOffset.y));
    }
}


