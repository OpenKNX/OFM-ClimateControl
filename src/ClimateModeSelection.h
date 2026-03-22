#pragma once
#include "OpenKNX.h"

enum class ClimateModeSelection : uint8_t
{
    Auto = 0,
    Heating = 1,
    Cooling = 3,
    Off = 6,
    Fan = 9,
    Dehumification = 14,
    DefaultFromWinterOrSummer = 254,
    Undefined = 255
};

class ClimateModeSelectionHelper
{
  public:
    
    static const char* toString(ClimateModeSelection mode)
    {
        switch (mode)
        {
            case ClimateModeSelection::Auto:
                return "auto";
            case ClimateModeSelection::Heating:
                return "heating";
            case ClimateModeSelection::Cooling:
                return "cooling";
            case ClimateModeSelection::Off:
                return "off";
            case ClimateModeSelection::Fan:
                return "fan";
            case ClimateModeSelection::Dehumification:
                return "dehumification";
            case ClimateModeSelection::DefaultFromWinterOrSummer:
                return "default from winter or summer";
            case ClimateModeSelection::Undefined:
                return "undefined";
            default:
                return "error";
        }
    }
};