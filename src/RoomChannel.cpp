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

void RoomChannel::setup()
{
    _name = "RoomChannel" + std::to_string(_channelIndex + 1);
    _waitForPower = !ParamCLI_CHModeSelectionTurnOn;
}

void RoomChannel::writeFlash()
{
    openknx.flash.write((uint8_t*) &_targetTemperatureCoolingRawKnx, sizeof(uint16_t));
    openknx.flash.write((uint8_t*) &_targetTemperatureHeatingRawKnx, sizeof(uint16_t));
    openknx.flash.writeByte((uint8_t) _currentMode);
    openknx.flash.writeByte((uint8_t) _currentPower);
}

uint16_t RoomChannel::flashSize()
{
    return 2 /* _targetTemperatureCooling */ + 2 /* _targetTemperatureHeating */ + 1 /* _currentMode */ + 1 /* power */;
}

void RoomChannel::readFlash(const uint8_t *iBuffer, const uint16_t iSize, uint8_t version)
{
    _targetTemperatureCoolingRawKnx = openknx.flash.readWord();
    _targetTemperatureHeatingRawKnx = openknx.flash.readWord();
    _currentMode = (ClimateModeSelection) openknx.flash.readByte();
    _currentPower = (PowerState) openknx.flash.readByte();
}

bool RoomChannel::isWaiting()
{
    return _waitForTargetTemperature || _waitForMode || _waitForPower;
}

void RoomChannel::afterReadFlash(uint8_t version)
{
    switch (ParamCLI_CHInitMode)
    {
        case PT_CLIInit::Saved:
            if (version == 0 || _currentMode == ClimateModeSelection::Undefined)
            {
                _currentMode = (ClimateModeSelection) (uint8_t) ParamCLI_CHDefaultMode;
                logDebug("Use default mode %s", ClimateModeSelectionHelper::toString(_currentMode));
            }
            else
            {
                logDebug("Use mode from flash %s", ClimateModeSelectionHelper::toString(_currentMode));
            }
            _waitForMode = false;
            break;
        case PT_CLIInit::ReadFromBus:
            _currentMode = (ClimateModeSelection) (uint8_t) ParamCLI_CHDefaultMode;
            logDebug("Use default mode %s if read request does not response", ClimateModeSelectionHelper::toString(_currentMode));
            KoCLI_CModeSelection.requestObjectRead();
            break;
        case PT_CLIInit::ReadFromBusOrSaved:
            if (version == 0 || _currentMode == ClimateModeSelection::Undefined)
            {
                _currentMode = (ClimateModeSelection) (uint8_t) ParamCLI_CHDefaultMode;
                logDebug("Use default mode %s if read request does not response", ClimateModeSelectionHelper::toString(_currentMode));
            }
            else
            {
                logDebug("Use mode from flash %s if read request does not response", ClimateModeSelectionHelper::toString(_currentMode));
            }
            KoCLI_CModeSelection.requestObjectRead();
            break;
        default:
            logErrorP("Unknown init mode %d, using default value", (int)ParamCLI_CHInitMode);
            _currentMode = (ClimateModeSelection) ParamCLI_CHDefaultMode;
            break;
    }   
    if (ParamCLI_CHModeSelectionTurnOn)
    {
        _currentPower = _currentMode != ClimateModeSelection::Off ? PowerState::On : PowerState::Off;
    }
    else
    {
        switch (ParamCLI_CHInitPower)
        {
        case PT_CLIInit::Saved:
            if (version == 0 || _currentPower == PowerState::Undefined)
            {
                _currentPower = ParamCLI_CHDefaultPower ? PowerState::On : PowerState::Off;
                logDebug("Use default power %s", _currentPower == PowerState::On ? "On" : "Off");
            }
            else
            {
                logDebug("Use power from flash %s", _currentPower == PowerState::On ? "On" : "Off");
            }
            _waitForPower = false;
            break;
        case PT_CLIInit::ReadFromBus:
            _currentPower = (PowerState) ParamCLI_CHDefaultPower;
            logDebug("Use default power %s if read request does not response", _currentPower == PowerState::On ? "On" : "Off");
            KoCLI_CPower.requestObjectRead();
            break;
        case PT_CLIInit::ReadFromBusOrSaved:
            if (version == 0 || _currentPower == PowerState::Undefined)
            {
                _currentPower = ParamCLI_CHDefaultPower ? PowerState::On : PowerState::Off;
                logDebug("Use default power %s if read request does not response", _currentPower == PowerState::On ? "On" : "Off");
            }
            else
            {
                logDebug("Use power from flash %s if read request does not response", _currentPower == PowerState::On ? "On" : "Off");
            }
            KoCLI_CPower.requestObjectRead();
            break;
        default:
            logErrorP("Unknown init mode %d, using default value", (int)ParamCLI_CHInitMode);
            _currentPower = (PowerState) ParamCLI_CHDefaultPower;
            break;
        }  
    }
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
            _forceSendMode = true;
            auto mode = (ClimateModeSelection)(uint8_t)ko.value(DPT_DecimalFactor);
            if (_waitForMode)
            {
                logInfoP("Received initial mode %s from bus", ClimateModeSelectionHelper::toString(mode));
                _waitForMode = false;
            }
            setMode(mode);
            break;
        }
        case CLI_KoCPower:
        {
            _forceSendPower = true;
            auto power = (PowerState) (((bool)ko.value(DPT_Switch)) ? PowerState::On : PowerState::Off);
            if (_waitForPower)
            {
                logInfoP("Received initial power state %s from bus", _currentPower == PowerState::On ? "On" : "Off");
                _waitForPower = false;
            }
            setPower(power);
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



void RoomChannel::start()
{
    if (_currentMode == ClimateModeSelection::DefaultFromWinterOrSummer)
    {
        if (ParamCLI_SummerWinterDayTemp || ParamCLI_SummerWinterDate || ParamCLI_SummerWinterKo)
        {
            _currentMode = (ClimateModeSelection)(uint8_t)( openknxClimateControlModule.isWinter() ? ParamCLI_WinterModeChange : ParamCLI_SummerModeChange);
        }
        else
        {
            _currentMode = openknxClimateControlModule.isWinter() ? ClimateModeSelection::Heating : ClimateModeSelection::Cooling;
        }
    }
    setMode(_currentMode);
    setPower(_currentPower);
    _started = true;
    handle();
}

void RoomChannel::setPower(PowerState power)
{
    if (power != _currentPower && power != PowerState::Undefined)
    {
        _currentPower = power;
        if (power == PowerState::Off)
            setMode(ClimateModeSelection::Off);
        handle();
    }
}

void RoomChannel::setMode(ClimateModeSelection mode)
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
        if (mode != _currentMode && mode != ClimateModeSelection::Undefined)
        {
            _currentMode = mode;
            _waitForMode = false;
            if (mode == ClimateModeSelection::Off)
            {
                _currentPower = PowerState::Off;
                _waitForPower = false;
            }
            else
            {
                if (ParamCLI_CHModeSelectionTurnOn)
                {
                    _currentPower = PowerState::On;
                    _waitForPower = false;
                }
            }
        }
        handle();
    }
}

void RoomChannel::handle()
{
    if (!_started)
        return;
    if (_currentMode == ClimateModeSelection::Undefined)
        return;
    if (_currentPower == PowerState::Undefined)
        return;
    KoCLI_CPowerFb.valueCompare(_currentPower == PowerState::On, DPT_Switch);
    if (_forceSendPower)
    {
        KoCLI_CPowerFb.objectWritten();
        _forceSendPower = false;   
    }
    KoCLI_CModeSelectionFb.valueCompare((uint8_t)_currentMode, DPT_DecimalFactor);    
    if (_forceSendMode)
    {
        KoCLI_CModeSelectionFb.objectWritten();
        _forceSendMode = false;   
    }
    if (_currentMode == ClimateModeSelection::Auto)
    {
        handleAuto();
    }
    else
    {
        handleMode(_currentMode);
    }
}

void RoomChannel::handleMode(ClimateModeSelection mode)
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
    if (mode != ClimateModeSelection::Off && _currentPower == PowerState::Off)
    {
        // Power is off, turn of devices
        _climateDevice1.setMode(ClimateModeSelection::Off);
        _climateDevice2.setMode(ClimateModeSelection::Off);
        return;
    }
    switch (deviceSelection)
    {
        case PT_CLIDeviceSelection::CoolingHeatingSystem1:

            _climateDevice2.setMode(ClimateModeSelection::Off);
            _climateDevice1.setMode(mode);
            return;
        case PT_CLIDeviceSelection::CoolingHeatingSystem2:
            _climateDevice1.setMode(ClimateModeSelection::Off);
            _climateDevice2.setMode(mode);
            return;
        case PT_CLIDeviceSelection::CoolingHeatingSystem1And2:
            _climateDevice1.setMode(mode);
            _climateDevice2.setMode(mode);
            return;
        case PT_CLIDeviceSelection::Disabled:
            if (mode == ClimateModeSelection::Off)
            {
                _climateDevice1.setMode(ClimateModeSelection::Off);
                _climateDevice2.setMode(ClimateModeSelection::Off);
                return;
            }
            return;
        default:
            logInfoP("No device assigned for mode %s", ClimateModeSelectionHelper::toString(mode));
            return;
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