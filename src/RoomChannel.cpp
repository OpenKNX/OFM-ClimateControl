#include "ClimateControlModule.h"
#include "RoomChannel.h"

RoomChannel::RoomChannel(uint8_t channelIndex) : _channelIndex(channelIndex)
{
    _name = "RoomChannel" + std::to_string(channelIndex + 1);
}

const std::string RoomChannel::name()
{
    return _name;
}

bool RoomChannel::processCommand(const std::string cmd, bool diagnoseKo)
{
    return false; 
}

void RoomChannel::processInputKo(GroupObject &ko)
{
    OpenKNX::Channel::processInputKo(ko);
    switch (CLI_KoCalcIndex(ko.asap()))
    {
        case CLI_KoCModeSelection: {
            handleModeChange(ko.value(DPT_DecimalFactor));
            break;
        }
        case CLI_KoCPower: {
            handlePowerChange(ko.value(DPT_Switch));
            break;
        }
        case CLI_KoCTargetTempRelativ: {
            if (ko.value(DPT_Switch))
            {
               auto temp = (float) KoCLI_CTargetTemp.value(DPT_Value_Temp);
               temp += ((float) ParamCLI_CHRelativTempChangeValue) / 10.0f;
               KoCLI_CTargetTemp.value(temp, DPT_Value_Temp);
            }
            else            
            {
               auto temp = (float) KoCLI_CTargetTemp.value(DPT_Value_Temp);
               temp -= ((float) ParamCLI_CHRelativTempChangeValue) / 10.0f;
               KoCLI_CTargetTemp.value(temp, DPT_Value_Temp);
            }
            break;
        }
    }
}

void RoomChannel::setup()
{
    OpenKNX::Channel::setup();

    if (!KoCLI_CModeSelection.initialized())
    {
        KoCLI_CModeSelection.valueNoSend(DefaultMode, DPT_DecimalFactor);
        KoCLI_CModeSelection.requestObjectRead();
    }
    KoCLI_CPowerFb.value(KoCLI_CPower.value(DPT_Switch), DPT_Switch);
    handleModeChange(KoCLI_CModeSelection.value(DPT_DecimalFactor));
    if (KoCLI_CPower.initialized())
    {
        handlePowerChange(KoCLI_CPower.value(DPT_Switch));
    }
    else
    {
        KoCLI_CPower.valueNoSend(DefaultMode != ClimateModeSelection::Off, DPT_Switch);
        KoCLI_CPower.requestObjectRead();
    }
}

void RoomChannel::handlePowerChange(bool power)
{
    if (_currentMode != ClimateModeSelection::Off && !power)
    {
        logInfoP("Power turned off, changing mode to 'Off'");
        handleModeChange(ClimateModeSelection::Off);
    }
    else
    {
        logInfoP("Power turned on, changing mode to previous state");
        handleModeChange(KoCLI_CModeSelection.value(DPT_DecimalFactor));
    }
}

void RoomChannel::handleModeChange(uint8_t mode)
{
    if (_currentMode != mode)
    {
        switch (mode)
        {
        case ClimateModeSelection::Auto:
            logInfoP("Mode changed to 'Auto'");
            break;
        case ClimateModeSelection::Heating:
            logInfoP("Mode changed to 'Heating'");
            break;
        case ClimateModeSelection::Cooling:
            logInfoP("Mode changed to 'Cooling'");
            break;
        case ClimateModeSelection::Off: 
            logInfoP("Mode changed to 'Off'");
            break;
        case ClimateModeSelection::Fan:
            logInfoP("Mode changed to 'Fan'");
            break;
        case ClimateModeSelection::Dehumification:
            logInfoP("Mode changed to 'Dehumification'");
            break;
        default:
            logInfoP("Mode changed to unknown value %d", mode);
            if (_currentMode == 255)
                mode = DefaultMode;
            else
                mode = _currentMode;
            break;
        }
        KoCLI_CModeSelectionFb.value(mode, DPT_DecimalFactor);
        if (mode != ClimateModeSelection::Off && !ParamCLI_CHModeSelectionTurnOn && !KoCLI_CPower.value(DPT_Switch))
        {
            logInfoP("Power is off, not changing mode");
            return;       
        }
        KoCLI_CPower.valueCompare(mode != ClimateModeSelection::Off, DPT_Switch);
        _currentMode = mode;
    }
}