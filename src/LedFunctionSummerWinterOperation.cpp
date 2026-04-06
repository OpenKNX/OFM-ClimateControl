#include "LedFunctionSummerWinterOperation.h"
#include "ClimateControlModule.h"


void LedFunctionSummerWinterOperation::loop()
{
    bool isSummer = openknxClimateControlModule.isSummer();
    bool isStarting = !openknxClimateControlModule.isStarted();
    if (_ledFunctionGroup == nullptr)
    {
        _ledFunctionGroup = openknx.ledFunctions.get(OPENKNX_LEDFUNC_CLI_SUMMER_WINTER_OPERATION);
    }
    LedState currentState = isStarting ? LedState::Starting : (isSummer ? LedState::Summer : LedState::Winter);
    if (currentState != _lastState)
    {
        _lastState = currentState;
        switch (currentState)
        {
            case LedState::Starting:
                // color
                _ledFunctionGroup->color(OpenKNX::Led::Color::Yellow);
                _ledFunctionGroup->blinking(500, OpenKNX::Led::Capability::COLOR);
                // monochrome
                _ledFunctionGroup->blinking(500, OpenKNX::Led::Capability::MONOCHROME);
                break;
            case LedState::Summer:
                // color
                _ledFunctionGroup->color(OpenKNX::Led::Color::Blue);
                _ledFunctionGroup->on(OpenKNX::Led::Capability::COLOR);
                // monochrome
                _ledFunctionGroup->off(OpenKNX::Led::Capability::MONOCHROME);
                break;
            case LedState::Winter:
                // color
                _ledFunctionGroup->color(OpenKNX::Led::Color::Orange);
                _ledFunctionGroup->on(OpenKNX::Led::Capability::COLOR);
                // monochrome
                _ledFunctionGroup->on(OpenKNX::Led::Capability::MONOCHROME);
                break;
        }
    }
}