#include "RoomChannel.h"
#include "ClimateControlModule.h"

RoomChannel::RoomChannel(int channelIndex) : _channelIndex(channelIndex),
                                             _name("RoomChannel"),
                                             _climateDevice1(channelIndex, 0, *this), _climateDevice2(channelIndex, 1, *this)
{
}

const std::string RoomChannel::name()
{
    return _name;
}

bool RoomChannel::processCommand(const std::string cmd, bool diagnoseKo)
{
    if (cmd == "")
    {
        logStatus();
        return true;
    }
    return false;
}

void RoomChannel::processInputKo(GroupObject &ko)
{
    OpenKNX::Channel::processInputKo(ko);
    switch (CLI_KoCalcIndex(ko.asap()))
    {
        case CLI_KoCModeSelection:
        {
            handleModeChange((ClimateModeSelection)(uint8_t)ko.value(DPT_DecimalFactor));
            break;
        }
        case CLI_KoCPower:
        {
            KoCLI_CPowerFb.value(ko.value(DPT_Switch), DPT_Switch);
            handle();
            break;
        }
        case CLI_KoCTargetTempRelativ:
        {
            if (ko.value(DPT_Switch))
            {
                auto temp = (float)KoCLI_CTargetTemp.value(DPT_Value_Temp);
                temp += ((float)ParamCLI_CHRelativTempChangeValue) / 10.0f;
                KoCLI_CTargetTemp.value(temp, DPT_Value_Temp);
            }
            else
            {
                auto temp = (float)KoCLI_CTargetTemp.value(DPT_Value_Temp);
                temp -= ((float)ParamCLI_CHRelativTempChangeValue) / 10.0f;
                KoCLI_CTargetTemp.value(temp, DPT_Value_Temp);
            }
            break;
        }
    }
}

void RoomChannel::setup()
{
    OpenKNX::Channel::setup();

    if (KoCLI_CModeSelection.initialized())
    {
        KoCLI_CModeSelection.valueNoSend((uint8_t)DefaultMode, DPT_DecimalFactor);
        KoCLI_CModeSelection.requestObjectRead();
    }
    else
    {
        handleModeChange((ClimateModeSelection)(uint8_t)KoCLI_CModeSelection.value(DPT_DecimalFactor));
    }
    KoCLI_CPowerFb.value(KoCLI_CPower.value(DPT_Switch), DPT_Switch);
    if (KoCLI_CPower.initialized())
    {
        handle();
    }
    else
    {
        KoCLI_CPower.valueNoSend(DefaultMode != ClimateModeSelection::Off, DPT_Switch);
        KoCLI_CPower.requestObjectRead();
    }
}

void RoomChannel::handleModeChange(ClimateModeSelection mode)
{
    if (_currentMode != mode)
    {
        switch (mode)
        {
            case ClimateModeSelection::Auto:
                logInfoP("Mode changed to 'Auto'");
                break;
            case ClimateModeSelection::Heating:
                if (ParamCLI_CHHeatDeactiveInSummer && openknxClimateControlModule.isSummer())
                {
                    logInfoP("Heating deactivated in Summer");
                    mode = _currentMode;
                }
                else
                {
                    logInfoP("Mode changed to 'Heating'");
                }
                break;
            case ClimateModeSelection::Cooling:
                if (ParamCLI_CHCoolDeactiveInWinter && openknxClimateControlModule.isWinter())
                {
                    logInfoP("Cooling deactivated in Winter");
                    mode = _currentMode;
                }
                else
                {
                    logInfoP("Mode changed to 'Cooling'");
                }
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
                if (_currentMode == ClimateModeSelection::Undefined)
                    mode = DefaultMode;
                else
                    mode = _currentMode;
                break;
        }
        if (mode != _currentMode)
        {

            _currentMode = mode;
            if (mode == ClimateModeSelection::Off)
            {
                if (KoCLI_CPowerFb.valueCompare(false, DPT_Switch))
                    logInfoP("Power off");
            }
            else
            {
                if (ParamCLI_CHModeSelectionTurnOn)
                {
                    if (KoCLI_CPowerFb.valueCompare(true, DPT_Switch))
                        logInfoP("Power on");
                }
            }
            logInfoP("Active mode %s", ClimateModeSelectionHelper::toString(mode));
            if (mode == ClimateModeSelection::Auto)
            {
                _currentMode = mode;
                handleAuto();
            }
            else
            {
                if (handleMode(mode))
                {
                    _currentActiveMode = mode;
                }
            }
        }
        KoCLI_CModeSelectionFb.value((uint8_t)_currentMode, DPT_DecimalFactor);
    }
}

void RoomChannel::handle()
{
    if (_currentMode == ClimateModeSelection::Auto)
    {
        handleAuto();
    }
    else
    {
        handleMode(_currentMode);
    }
}

bool RoomChannel::handleMode(ClimateModeSelection mode)
{
    PT_CLIDeviceSelection deviceSelection = PT_CLIDeviceSelection::Disabled;
    switch (mode)
    {
        case ClimateModeSelection::Cooling:
            deviceSelection = ParamCLI_CHCoolDeviceSelection;
            break;
        case ClimateModeSelection::Heating:
            deviceSelection = ParamCLI_CHHeatDeviceSelection;
            break;
        case ClimateModeSelection::Fan:
            deviceSelection = ParamCLI_CHFanDeviceSelection;
            break;
        case ClimateModeSelection::Dehumification:
            deviceSelection = ParamCLI_CHDehumDeviceSelection;
            break;
        case ClimateModeSelection::Off:
            deviceSelection = PT_CLIDeviceSelection::Disabled;
            break;
        default:
            logInfoP("Invalid mode %s", ClimateModeSelectionHelper::toString(mode));
            break;
    }
    if (mode != ClimateModeSelection::Off && !KoCLI_CPowerFb.value(DPT_Switch))
    {
        // Power is of, do not forward mode to devices
        _climateDevice1.setMode(ClimateModeSelection::Off);
        _climateDevice2.setMode(ClimateModeSelection::Off);
        return deviceSelection != PT_CLIDeviceSelection::Disabled;
    }
    switch (deviceSelection)
    {
        case PT_CLIDeviceSelection::CoolingHeatingSystem1:

            _climateDevice2.setMode(ClimateModeSelection::Off);
            _climateDevice1.setMode(mode);
            return true;
        case PT_CLIDeviceSelection::CoolingHeatingSystem2:
            _climateDevice1.setMode(ClimateModeSelection::Off);
            _climateDevice2.setMode(mode);
            return true;
        case PT_CLIDeviceSelection::CoolingHeatingSystem1And2:
            _climateDevice1.setMode(mode);
            _climateDevice2.setMode(mode);
            return true;
        case PT_CLIDeviceSelection::Disabled:
            if (mode == ClimateModeSelection::Off)
            {
                _climateDevice1.setMode(ClimateModeSelection::Off);
                _climateDevice2.setMode(ClimateModeSelection::Off);
                return true;
            }
            return false;
        default:
            logInfoP("No device assigned for mode %s", ClimateModeSelectionHelper::toString(mode));
            return false;
    }
}

void RoomChannel::handleAuto()
{
}

void RoomChannel::logStatus()
{
    logInfoP("Power: %s", KoCLI_CPowerFb.valueCompare(true, DPT_Switch) ? "On" : "Off");
    logInfoP("Current mode: %s", ClimateModeSelectionHelper::toString(_currentMode));
    logInfoP("Last mode: %s", ClimateModeSelectionHelper::toString((ClimateModeSelection)(uint8_t)KoCLI_CModeSelection.value(DPT_DecimalFactor)));
    logInfoP("Active mode: %s", ClimateModeSelectionHelper::toString(_currentActiveMode));
    _climateDevice1.logStatus();
    _climateDevice2.logStatus();
}