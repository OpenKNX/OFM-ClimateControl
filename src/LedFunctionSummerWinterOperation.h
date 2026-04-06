#pragma once
#include "OpenKNX.h"

#define OPENKNX_LEDFUNC_CLI_SUMMER_WINTER_OPERATION 404

class LedFunctionSummerWinterOperation
{
    OpenKNX::Led::FunctionGroup* _ledFunctionGroup = nullptr;
    enum class LedState
    {
        Undefined,
        Starting,
        Summer,
        Winter
    };
    LedState _lastState = LedState::Undefined;
public:
    void loop();
};

