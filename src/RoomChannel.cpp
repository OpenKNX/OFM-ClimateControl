#include "RoomChannel.h"
#include "ClimateControlModule.h"
#include <cmath>


RoomChannel::RoomChannel(int channelIndex) : _channelIndex(channelIndex),
                                             _name("RoomChannel"),
                                             _climateDevice1(channelIndex, 0, *this), _climateDevice2(channelIndex, 1, *this)
{
}

const std::string RoomChannel::name()
{
    return _name;
}

float RoomChannel::getTemperatureFromRawKnx(uint16_t rawKnx)
{
    return openknxClimateControlModule.getTemperatureFromRawKnx(rawKnx);
}

uint16_t RoomChannel::getRawKnxFromTemperature(float temperature)
{
    return openknxClimateControlModule.getRawKnxFromTemperature(temperature);
}

void RoomChannel::setup()
{
    _name = "RoomChannel" + std::to_string(_channelIndex + 1);
    _waitForPower = !ParamCLI_CHModeSelectionTurnOn;
}

void RoomChannel::writeFlash()
{
    openknx.flash.write((uint8_t *)&_targetTemperatureCoolingRawKnx, sizeof(uint16_t));
    openknx.flash.write((uint8_t *)&_targetTemperatureHeatingRawKnx, sizeof(uint16_t));
    openknx.flash.writeByte((uint8_t)_currentMode);
    openknx.flash.writeByte((uint8_t)_currentPower);
}

uint16_t RoomChannel::flashSize()
{
    return 2 /* _targetTemperatureCooling */ + 2 /* _targetTemperatureHeating */ + 1 /* _currentMode */ + 1 /* power */;
}

void RoomChannel::readFlash(const uint8_t *iBuffer, const uint16_t iSize, uint8_t version)
{
    _targetTemperatureCoolingRawKnx = openknx.flash.readWord();
    _targetTemperatureHeatingRawKnx = openknx.flash.readWord();
    _currentMode = (ClimateModeSelection)openknx.flash.readByte();
    _currentPower = (PowerState)openknx.flash.readByte();
}

bool RoomChannel::isWaiting()
{
    return _waitForTargetTemperature || _waitForMode || _waitForPower;
}
void RoomChannel::setInitTargetTemperatur()
{
    if (ParamCLI_CH2TargetTemp)
    {
        _targetTemperatureCoolingRawKnx = openknxClimateControlModule.getRawKnxFromTemperature(ParamCLI_CHTargetDefaultCooling);
        _targetTemperatureHeatingRawKnx = openknxClimateControlModule.getRawKnxFromTemperature(ParamCLI_CHTargetDefaultHeating);
        logDebugP("Use default target temperatures %0.1f °C (cooling) / %0.1f °C (heating)", ParamCLI_CHTargetDefaultCooling, ParamCLI_CHTargetDefaultHeating);
    }
    else
    {
        _targetTemperatureHeatingRawKnx = openknxClimateControlModule.getRawKnxFromTemperature(ParamCLI_CHTargetDefaultHeating);
        logDebugP("Use default target temperature %0.1f °C", ParamCLI_CHTargetDefaultHeating);
    }
}
void RoomChannel::afterReadFlash(uint8_t version)
{
    switch (ParamCLI_CHInitTemp)
    {
        case PT_CLIInit::Saved:
        case PT_CLIInit::ReadFromBusOrSaved:
            if (version == 0 || _targetTemperatureCoolingRawKnx == std::numeric_limits<uint16_t>::max() || _targetTemperatureHeatingRawKnx == std::numeric_limits<uint16_t>::max())
            {
                setInitTargetTemperatur();
            }
            else
            {
                if (ParamCLI_CH2TargetTemp)
                {
                    logDebugP("Use target temperatures from flash %0.1f °C (cooling) / %0.1f °C (heating)", getTemperatureFromRawKnx(_targetTemperatureCoolingRawKnx), getTemperatureFromRawKnx(_targetTemperatureHeatingRawKnx));
                }
                else
                {
                    logDebugP("Use target temperature from flash %0.1f °C", getTemperatureFromRawKnx(_targetTemperatureHeatingRawKnx));
                }
            }
            if (ParamCLI_CHInitTemp == PT_CLIInit::ReadFromBusOrSaved)
                KoCLI_CTargetTemp.requestObjectRead();
            else
                _waitForTargetTemperature = false;
            break;
        case PT_CLIInit::ReadFromBus:
            setInitTargetTemperatur();
            KoCLI_CTargetTemp.requestObjectRead();
            break;
        default:
            logErrorP("Unknown init mode %d, using default value", (int)ParamCLI_CHInitTemp);
            setInitTargetTemperatur();
            break;
    }
    switch (ParamCLI_CHInitMode)
    {
        case PT_CLIInit::Saved:
        case PT_CLIInit::ReadFromBusOrSaved:
            if (version == 0 || _currentMode == ClimateModeSelection::Undefined)
            {
                _currentMode = (ClimateModeSelection)(uint8_t)ParamCLI_CHDefaultMode;
                logDebug("Use default mode %s", ClimateModeSelectionHelper::toString(_currentMode));
            }
            else
            {
                logDebug("Use mode from flash %s", ClimateModeSelectionHelper::toString(_currentMode));
            }
            if (ParamCLI_CHInitMode == PT_CLIInit::ReadFromBusOrSaved)
                KoCLI_CModeSelection.requestObjectRead();
            else
                _waitForMode = false;
            break;
        case PT_CLIInit::ReadFromBus:
            _currentMode = (ClimateModeSelection)(uint8_t)ParamCLI_CHDefaultMode;
            logDebug("Use default mode %s if read request does not response", ClimateModeSelectionHelper::toString(_currentMode));
            KoCLI_CModeSelection.requestObjectRead();
            break;
        default:
            logErrorP("Unknown init mode %d, using default value", (int)ParamCLI_CHInitMode);
            _currentMode = (ClimateModeSelection)ParamCLI_CHDefaultMode;
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
            case PT_CLIInit::ReadFromBusOrSaved:
                if (version == 0 || _currentPower == PowerState::Undefined)
                {
                    _currentPower = ParamCLI_CHDefaultPower ? PowerState::On : PowerState::Off;
                    logDebug("Use default power %s", _currentPower == PowerState::On ? "On" : "Off");
                }
                else
                {
                    logDebug("Use power from flash %s", _currentPower == PowerState::On ? "On" : "Off");
                }
                if (ParamCLI_CHInitPower == PT_CLIInit::ReadFromBusOrSaved)
                    KoCLI_CPower.requestObjectRead();
                else
                    _waitForPower = false;
                break;
            case PT_CLIInit::ReadFromBus:
                _currentPower = (PowerState)ParamCLI_CHDefaultPower;
                logDebug("Use default power %s if read request does not response", _currentPower == PowerState::On ? "On" : "Off");
                KoCLI_CPower.requestObjectRead();
                break;
            default:
                logErrorP("Unknown init mode %d, using default value", (int)ParamCLI_CHInitMode);
                _currentPower = (PowerState)ParamCLI_CHDefaultPower;
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
            auto power = (PowerState)(((bool)ko.value(DPT_Switch)) ? PowerState::On : PowerState::Off);
            if (_waitForPower)
            {
                logInfoP("Received initial power state %s from bus", _currentPower == PowerState::On ? "On" : "Off");
                _waitForPower = false;
            }
            setPower(power);
            break;
        }
        case CLI_KoCTargetTemp:
        {
            auto targetTemperatureRawKnx = (uint16_t) ko.value(DPT_Value_2_Ucount);
            if (_waitForTargetTemperature)
            {
                logInfoP("Received initial target temperature %0.1f °C from bus", getTemperatureFromRawKnx(targetTemperatureRawKnx));
                _waitForTargetTemperature = false;
            }
            else
            {
                logInfoP("Received target temperature %0.1f (%d) °C from bus", getTemperatureFromRawKnx(targetTemperatureRawKnx), (int) targetTemperatureRawKnx);
            }
            _forceSendTargetTemperature = true;
            setTargetTemperatureRawKnx(targetTemperatureRawKnx);
            break;
        }
        case CLI_KoCTargetTempRelativ:
        {
            if (_started)
            {
                _forceSendTargetTemperature = true;
                auto temp = getTemperatureFromRawKnx(getTargetTemperatureRawKnx());
                auto offset = ((float) (ParamCLI_CH2TargetTemp && _useCoolingTargetTemperature ? ParamCLI_CHRelativTempChangeCooling : ParamCLI_CHRelativTempChangeHeating)) / 10.0f;
                if (ko.value(DPT_Switch))
                {
                    temp += offset;
                }
                else
                {
                    temp -= offset;
                }
                setTargetTemperatureRawKnx(getRawKnxFromTemperature(temp));
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
            _currentMode = (ClimateModeSelection)(uint8_t)(openknxClimateControlModule.isWinter() ? ParamCLI_WinterModeChange : ParamCLI_SummerModeChange);
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
            _useCoolingTargetTemperature = ParamCLI_CH2TargetTemp;
            break;
        case ClimateModeSelection::Heating:
            deviceSelection = ParamCLI_CHHeatDeviceSelection;
            _useCoolingTargetTemperature = false;
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
    if (_targetTemperatureSetWhileStarting != std::numeric_limits<uint16_t>::max())
    {
        if (_useCoolingTargetTemperature)
            _targetTemperatureHeatingRawKnx = _targetTemperatureCoolingRawKnx;    
        setTargetTemperatureRawKnx(_targetTemperatureSetWhileStarting);
        _targetTemperatureSetWhileStarting = std::numeric_limits<uint16_t>::max();
    
    }
    else
    {
        // Set new target temperature if needed
        setTargetTemperatureRawKnx(getTargetTemperatureRawKnx());
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
    if (ParamCLI_CH2TargetTemp)
    {
        logInfoP("Target temperature for cooling: %0.1f °C", getTemperatureFromRawKnx(_targetTemperatureCoolingRawKnx));
        logInfoP("Target temperature for heating: %0.1f °C", getTemperatureFromRawKnx(_targetTemperatureHeatingRawKnx));
    }
    else
    {
        logInfoP("Target temperature: %0.1f °C", getTemperatureFromRawKnx(_targetTemperatureCoolingRawKnx));
    }
    _climateDevice1.logStatus();
    _climateDevice2.logStatus();
}

uint16_t RoomChannel::getTargetTemperatureRawKnx()
{
    if (_useCoolingTargetTemperature)
    {
        return _targetTemperatureCoolingRawKnx;
    }
    else
    {
        return _targetTemperatureHeatingRawKnx;
    }
}

void RoomChannel::setTargetTemperatureRawKnx(uint16_t targetTemperatureRawKnx)
{
    if (!_started)
    {
        _targetTemperatureSetWhileStarting = targetTemperatureRawKnx;
        logDebugP("Set target temperature to %0.1f °C while starting", getTemperatureFromRawKnx(targetTemperatureRawKnx));
        return;
    }
    if (_useCoolingTargetTemperature)
    {
        if (targetTemperatureRawKnx < getRawKnxFromTemperature(ParamCLI_CHTargetMinCooling))
        {
            targetTemperatureRawKnx = getRawKnxFromTemperature(ParamCLI_CHTargetMinCooling);
            logDebugP("Cooling target temperature too low, set to minimum %0.1f °C", ParamCLI_CHTargetMinCooling);
        }
        else if (targetTemperatureRawKnx > getRawKnxFromTemperature(ParamCLI_CHTargetMaxCooling))
        {
            targetTemperatureRawKnx = getRawKnxFromTemperature(ParamCLI_CHTargetMaxCooling);
            logDebugP("Cooling target temperature too high, set to maximum %0.1f °C", ParamCLI_CHTargetMaxCooling);
        }

        if (ParamCLI_CH2TargetTemp)
            logDebugP("Set cooling target temperature to %0.1f °C", getTemperatureFromRawKnx(targetTemperatureRawKnx));
        else
            logDebugP("Set target temperature to %0.1f °C", getTemperatureFromRawKnx(targetTemperatureRawKnx));
        _targetTemperatureCoolingRawKnx = targetTemperatureRawKnx;
    }
    else
    {
        if (targetTemperatureRawKnx < getRawKnxFromTemperature(ParamCLI_CHTargetMinHeating))
        {
            targetTemperatureRawKnx = getRawKnxFromTemperature(ParamCLI_CHTargetMinHeating);
            if (ParamCLI_CH2TargetTemp)
                logDebugP("Heating target temperature too low, set to minimum %0.1f °C", ParamCLI_CHTargetMinHeating);
            else
                logDebugP("Target temperature too low, set to minimum %0.1f °C", ParamCLI_CHTargetMinHeating);
        }
        else if (targetTemperatureRawKnx > getRawKnxFromTemperature(ParamCLI_CHTargetMaxHeating))
        {
            targetTemperatureRawKnx = getRawKnxFromTemperature(ParamCLI_CHTargetMaxHeating);
            if (ParamCLI_CH2TargetTemp)
                logDebugP("Heating target temperature too high, set to maximum %0.1f °C", ParamCLI_CHTargetMaxHeating);
            else
                logDebugP("Target temperature too high, set to maximum %0.1f °C", ParamCLI_CHTargetMaxHeating);
        }
        logDebugP("Set heating target temperature to %0.1f °C", getTemperatureFromRawKnx(targetTemperatureRawKnx));
        _targetTemperatureHeatingRawKnx = targetTemperatureRawKnx;
    }
    KoCLI_CTargetTempFb.valueCompare(targetTemperatureRawKnx, DPT_Value_2_Ucount);
    if (_forceSendTargetTemperature)
    {
        KoCLI_CTargetTempFb.objectWritten();
        _forceSendTargetTemperature = false;
    }
}