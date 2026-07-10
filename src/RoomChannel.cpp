#include "RoomChannel.h"
#include "ClimateControlModule.h"
#include "ClimateDeviceKnxActor.h"
#ifdef CREATE_CLIMATE_DEVICE
#include "ClimageDeviceCustom.h"
#endif 
#include <cmath>


RoomChannel::RoomChannel(int channelIndex) : _channelIndex(channelIndex),
                                             _name("RoomChannel"),
                                             _lastActiveMode(DefaultMode)
{
    isActiveChangedFromDevice();
        ClimateDevice* device1;
#ifdef CREATE_CLIMATE_DEVICE
    device1 = CREATE_CLIMATE_DEVICE(channelIndex, 0, *this);
#else
    device1 = new ClimateDeviceKnxActor(channelIndex, 0, *this);
#endif
    device1->init(
        ParamCLI_CHHeatDeviceSelection == PT_CLIDeviceSelection::CoolingHeatingSystem1 || ParamCLI_CHHeatDeviceSelection == PT_CLIDeviceSelection::CoolingHeatingSystem1And2,
        ParamCLI_CHCoolDeviceSelection == PT_CLIDeviceSelection::CoolingHeatingSystem1 || ParamCLI_CHCoolDeviceSelection == PT_CLIDeviceSelection::CoolingHeatingSystem1And2,
        ParamCLI_CHDehumDeviceSelection == PT_CLIDeviceSelection::CoolingHeatingSystem1 || ParamCLI_CHDehumDeviceSelection == PT_CLIDeviceSelection::CoolingHeatingSystem1And2,
        ParamCLI_CHFanDeviceSelection == PT_CLIDeviceSelection::CoolingHeatingSystem1 || ParamCLI_CHFanDeviceSelection == PT_CLIDeviceSelection::CoolingHeatingSystem1And2,
        ParamCLI_CHAutoDeviceSelection == PT_CLIDeviceSelection::CoolingHeatingSystem1 || ParamCLI_CHAutoDeviceSelection == PT_CLIDeviceSelection::CoolingHeatingSystem1And2);    
    _climateDevices.push_back(device1);
    auto device2 = new ClimateDeviceKnxActor(channelIndex, 1, *this);
    device2->init(
        ParamCLI_CHHeatDeviceSelection == PT_CLIDeviceSelection::CoolingHeatingSystem2 || ParamCLI_CHHeatDeviceSelection == PT_CLIDeviceSelection::CoolingHeatingSystem1And2,
        ParamCLI_CHCoolDeviceSelection == PT_CLIDeviceSelection::CoolingHeatingSystem2 || ParamCLI_CHCoolDeviceSelection == PT_CLIDeviceSelection::CoolingHeatingSystem1And2,
        ParamCLI_CHDehumDeviceSelection == PT_CLIDeviceSelection::CoolingHeatingSystem2 || ParamCLI_CHDehumDeviceSelection == PT_CLIDeviceSelection::CoolingHeatingSystem1And2,
        ParamCLI_CHFanDeviceSelection == PT_CLIDeviceSelection::CoolingHeatingSystem2 || ParamCLI_CHFanDeviceSelection == PT_CLIDeviceSelection::CoolingHeatingSystem1And2,
        ParamCLI_CHAutoDeviceSelection == PT_CLIDeviceSelection::CoolingHeatingSystem2 || ParamCLI_CHAutoDeviceSelection == PT_CLIDeviceSelection::CoolingHeatingSystem1And2);
    _climateDevices.push_back(device2);
    for (ClimateDevice* device : _climateDevices)
    {
        if (device->supportMode(ClimateModeSelection::Cooling))
        {
            _coolingSupported = true;
        }
        if (device->supportMode(ClimateModeSelection::Heating))
        {
            _heatingSupported = true;
        }
    }

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
    if (KoCLI_CRoomTemp.initialized())
    {
        _roomTemperature = KoCLI_CRoomTemp.value(DPT_Value_2_Ucount);
    }
    else
    {
        KoCLI_CRoomTemp.requestObjectRead();
    }   
    if (ParamCLI_CHWindowOpenEnabled)
    {
        KoCLI_CWindowOpenAlarm.value(false, DPT_Switch);
    }
    if (ParamCLI_CHAutoHeatDeviceSelection == PT_CLIAutomaticDevice::RequestByGroupObject && !KoCLI_CAutoReqHeat.initialized())
    {
        KoCLI_CAutoReqHeat.requestObjectRead();
    }
    if (ParamCLI_CHAutoCoolDeviceSelection == PT_CLIAutomaticDevice::RequestByGroupObject && !KoCLI_CAutoReqCool.initialized())
    {
        KoCLI_CAutoReqCool.requestObjectRead();  
    }   
    if (ParamCLI_CHAutoDehumDeviceSelection == PT_CLIAutomaticDevice::RequestByGroupObject && !KoCLI_CAutoReqDehum.initialized())
    {
        KoCLI_CAutoReqDehum.requestObjectRead();
    }
    if (ParamCLI_CHAutoFanDeviceSelection == PT_CLIAutomaticDevice::RequestByGroupObject && !KoCLI_CAutoReqFan.initialized())
    {
        KoCLI_CAutoReqFan.requestObjectRead();  
    }
   
}

void RoomChannel::writeFlash()
{
    openknx.flash.write((uint8_t *)&_roomTemperature, sizeof(uint16_t));
    if (_targetTemperatureBeforeWindowOpen != std::numeric_limits<uint16_t>::max())
    {
        if (_useCoolingTargetTemperature)
        {
            openknx.flash.write((uint8_t *)&_targetTemperatureBeforeWindowOpen, sizeof(uint16_t));
            openknx.flash.write((uint8_t *)&_targetTemperatureHeatingRawKnx, sizeof(uint16_t));
        }
        else
        {
            openknx.flash.write((uint8_t *)&_targetTemperatureCoolingRawKnx, sizeof(uint16_t));
            openknx.flash.write((uint8_t *)&_targetTemperatureBeforeWindowOpen, sizeof(uint16_t));
        }
    }
    else
    {
        openknx.flash.write((uint8_t *)&_targetTemperatureCoolingRawKnx, sizeof(uint16_t));
        openknx.flash.write((uint8_t *)&_targetTemperatureHeatingRawKnx, sizeof(uint16_t));
    }
    if (_modeLockedWhileOpenWindow != ClimateModeSelection::Undefined)
    {
        openknx.flash.writeByte((uint8_t)_modeLockedWhileOpenWindow);
    }
    else
    {
        openknx.flash.writeByte((uint8_t)_currentMode);
    }
    openknx.flash.writeByte((uint8_t)_lastActiveMode);
    openknx.flash.writeByte((uint8_t)_currentPower);
}

uint16_t RoomChannel::flashSize()
{
    return 2 /* _roomTemperature */ + 2 /* _targetTemperatureCooling */ + 2 /* _targetTemperatureHeating */ + 1 /* _currentMode */ + 1 /* power */ + 1 /* _lastActiveMode */;
}

void RoomChannel::readFlash(const uint8_t *iBuffer, const uint16_t iSize, uint8_t version)
{
    if (version >= 3)
    {
        _roomTemperature = openknx.flash.readWord();
    }
    _targetTemperatureCoolingRawKnx = openknx.flash.readWord();
    _targetTemperatureHeatingRawKnx = openknx.flash.readWord();

    _currentMode = (ClimateModeSelection)openknx.flash.readByte();
    if (version >= 2)
        _lastActiveMode = (ClimateModeSelection)openknx.flash.readByte();
    else if (_currentMode != ClimateModeSelection::Off)
        _lastActiveMode = _currentMode;
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
        logDebugP("Use default target temperatures %0.1f°C (cooling) / %0.1f°C (heating)", ParamCLI_CHTargetDefaultCooling, ParamCLI_CHTargetDefaultHeating);
    }
    else
    {
        _targetTemperatureHeatingRawKnx = openknxClimateControlModule.getRawKnxFromTemperature(ParamCLI_CHTargetDefaultHeating);
        logDebugP("Use default target temperature %0.1f°C", ParamCLI_CHTargetDefaultHeating);
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
                    logDebugP("Use target temperatures from flash %0.1f°C (cooling) / %0.1f°C (heating)", getTemperatureFromRawKnx(_targetTemperatureCoolingRawKnx), getTemperatureFromRawKnx(_targetTemperatureHeatingRawKnx));
                }
                else
                {
                    logDebugP("Use target temperature from flash %0.1f°C", getTemperatureFromRawKnx(_targetTemperatureHeatingRawKnx));
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
#ifdef OPENKNX_DEBUG
    if (cmd == "test")
    {
        logInfoP("Test command received, sending test values");
        KoCLI_CTargetTemp.value((float) 24.3, DPT_Value_Temp);
        KoCLI_CDev1SetFb.value((float) 24, DPT_Value_Temp);
        return true;
    }
#endif
    return false;
}

void RoomChannel::processInputKo(GroupObject &ko)
{
    OpenKNX::Channel::processInputKo(ko);
    switch (CLI_KoCalcIndex(ko.asap()))
    {
        case CLI_KoCRoomTemp:
        {
            auto roomTemperatureRawKnx = (uint16_t)ko.value(DPT_Value_2_Ucount);
            _roomTemperature = roomTemperatureRawKnx;
            if (_roomTemperatureBeforeWindowOpen != std::numeric_limits<uint16_t>::max())
            {
                logInfoP("Received room temperature %0.1f°C from bus, but forward stored %0.1f", getTemperatureFromRawKnx(roomTemperatureRawKnx), getTemperatureFromRawKnx(_roomTemperatureBeforeWindowOpen));
                for (auto &device : _climateDevices)
                {
                    device->setRoomTemperature(_roomTemperatureBeforeWindowOpen);
                }
            }
            else
            {
                logInfoP("Received room temperature %0.1f°C from bus", getTemperatureFromRawKnx(roomTemperatureRawKnx));
                for (auto &device : _climateDevices)
                {
                    device->setRoomTemperature(roomTemperatureRawKnx);
                }
            }
            handleAuto();
            break;
        }
        case CLI_KoCModeSelection:
        {
            _forceSendMode = true;
            auto mode = (ClimateModeSelection)(uint8_t)ko.value(DPT_DecimalFactor);
            if (_waitForMode)
            {
                logInfoP("Received initial mode %s from bus", ClimateModeSelectionHelper::toString(mode));
                _waitForMode = false;
            }
            _autoModeFallbackTimer = 0;
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
            auto targetTemperatureRawKnx = (uint16_t)ko.value(DPT_Value_2_Ucount);
            if (_waitForTargetTemperature)
            {
                logInfoP("Received initial target temperature %0.1f°C from bus", getTemperatureFromRawKnx(targetTemperatureRawKnx));
                _waitForTargetTemperature = false;
            }
            else
            {
                logInfoP("Received target temperature %0.1f (%d)°C from bus", getTemperatureFromRawKnx(targetTemperatureRawKnx), (int)targetTemperatureRawKnx);
            }
            _forceSendTargetTemperature = true;
            setTargetTemperatureRawKnx(targetTemperatureRawKnx, ChangeSource::User);
            break;
        }
        case CLI_KoCTargetTempRelativ:
        {
            if (_started)
            {
                _forceSendTargetTemperature = true;
                auto temp = getTemperatureFromRawKnx(getTargetTemperatureRawKnx());
                auto offset = ((float)(ParamCLI_CH2TargetTemp && _useCoolingTargetTemperature ? ParamCLI_CHRelativTempChangeCooling : ParamCLI_CHRelativTempChangeHeating)) / 10.0f;
                if (ko.value(DPT_Switch))
                {
                    temp += offset;
                }
                else
                {
                    temp -= offset;
                }
                setTargetTemperatureRawKnx(getRawKnxFromTemperature(temp), ChangeSource::User);
            }
            break;
        }
        case CLI_KoCWindowOpen:
        {
            if (_started)
            {
                setWindowOpen(ko.value(DPT_Switch));
            }
        }
        default:
        {
            for (auto &device : _climateDevices)
            {
                device->processInputKo(ko);
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
    if (_roomTemperature != std::numeric_limits<uint16_t>::max() && _roomTemperatureBeforeWindowOpen == std::numeric_limits<uint16_t>::max())
    {
        for (auto &device : _climateDevices)
        {
            device->setRoomTemperature(_roomTemperature);
        }
    }
    setMode(_currentMode);
    setPower(_currentPower);
    _started = true;
logErrorP("Start");
logStatus();
    handle();
    if (CLI_CHWindowOpenEnabled)
    {
        if (KoCLI_CWindowOpen.initialized())
            setWindowOpen(KoCLI_CWindowOpen.value(DPT_Switch));
        else
            KoCLI_CWindowOpen.requestObjectRead();
    }
}

void RoomChannel::setPower(PowerState power)
{
    if (power != _currentPower && power != PowerState::Undefined)
    {
        _currentPower = power;
        _autoModeFallbackTimer = 0;
        if (power == PowerState::Off)
            setMode(ClimateModeSelection::Off);
        else 
            setMode(_lastActiveMode);
        handle();
    }
}

void RoomChannel::setWindowOpen(bool open)
{
    bool windowOpen = _windowOpenState == WindowOpenState::Open;
    if (windowOpen != open)
    {
        logDebugP("Set window open to %s", open ? "open" : "closed");
        if (open)
        {
            setWindowOpenState(WindowOpenState::Open);
        }
        else
        {
            resetWindowOpenAlarm(ParamCLI_CHWindowOpenAction1, _windowOpenAction1Handled);
            resetWindowOpenAlarm(ParamCLI_CHWindowOpenAction2, _windowOpenAction2Handled);
            resetWindowOpenAlarm(ParamCLI_CHWindowOpenAction3, _windowOpenAction3Handled);
            resetWindowOpenAlarm(ParamCLI_CHWindowOpenAction4, _windowOpenAction4Handled);
            resetWindowOpenAlarm(ParamCLI_CHWindowOpenAction5, _windowOpenAction5Handled);   
            if (_windowOpenAction1Handled || _windowOpenAction2Handled || _windowOpenAction3Handled || _windowOpenAction4Handled || _windowOpenAction5Handled)
            {
                if (ParamCLI_CHWindowClose == PT_CLIWindowClose::WaitTime)
                {
                    setWindowOpenState(WindowOpenState::WaitDelayAfterClose);
                }
                else
                {
                    setWindowOpenState(WindowOpenState::InitialWaitAfterClose);
                }
            }
            else
            {
                resetWindowOpenActions("Window closed, but no action was handled");
            }
        }
    }
}

void RoomChannel::resetWindowOpenAlarm(PT_CLIWindowOpenAction action, bool &handledFlag)
{
    if (!handledFlag)
        return;
    if (action == PT_CLIWindowOpenAction::WindowOpenAlarm || action == PT_CLIWindowOpenAction::WindowOpenAlarmActiveHeatingCooling)
    {
        if (KoCLI_CWindowOpenAlarm.valueCompare(false, DPT_Switch))
        {
            logDebugP("Reset window open alarm");
            KoCLI_CWindowOpenAlarm.value(false, DPT_Switch);
        }
        handledFlag = false;
    }
}


void RoomChannel::setWindowOpenState(WindowOpenState state)
{
    logDebugP("Set window open state to %s", state == WindowOpenState::Closed ? "Closed" : state == WindowOpenState::Open ? "Open" : state == WindowOpenState::WaitDelayAfterClose ? "WaitDelayAfterClose" : state == WindowOpenState::InitialWaitAfterClose ? "InitialWaitAfterClose" : "WaitForStableRoomTemperature");
    _windowOpenState = state;
    if (_windowOpenState != WindowOpenState::Closed)
    {
        _windowOpenTimer = max(1UL, millis());
    }
    else
    {
        _windowOpenTimer = 0;
        _lastCurrentRoomTemperatureRawKnx = std::numeric_limits<uint16_t>::max();

    }
}

void RoomChannel::handleWindowOpen()
{
    switch (_windowOpenState)
    {
        case WindowOpenState::Closed:
            break;
        case WindowOpenState::Open:
            if (_windowOpenTimer != 0)
            {    
                // window open
                unsigned long timeSinceWindowOpen = millis() - _windowOpenTimer;
                handleWindowOpenAction(1, ParamCLI_CHWindowOpenDelayTime1MS, ParamCLI_CHWindowOpenCondition1, ParamCLI_CHWindowOpenAction1, timeSinceWindowOpen, ParamCLI_CHWindowOpenTempCorrection1, _windowOpenAction1Handled);
                handleWindowOpenAction(2, ParamCLI_CHWindowOpenDelayTime2MS, ParamCLI_CHWindowOpenCondition2, ParamCLI_CHWindowOpenAction2, timeSinceWindowOpen, ParamCLI_CHWindowOpenTempCorrection2, _windowOpenAction2Handled);
                handleWindowOpenAction(3, ParamCLI_CHWindowOpenDelayTime3MS, ParamCLI_CHWindowOpenCondition3, ParamCLI_CHWindowOpenAction3, timeSinceWindowOpen, ParamCLI_CHWindowOpenTempCorrection3, _windowOpenAction3Handled);
                handleWindowOpenAction(4, ParamCLI_CHWindowOpenDelayTime4MS, ParamCLI_CHWindowOpenCondition4, ParamCLI_CHWindowOpenAction4, timeSinceWindowOpen, ParamCLI_CHWindowOpenTempCorrection4, _windowOpenAction4Handled);
                handleWindowOpenAction(5, ParamCLI_CHWindowOpenDelayTime5MS, ParamCLI_CHWindowOpenCondition5, ParamCLI_CHWindowOpenAction5, timeSinceWindowOpen, ParamCLI_CHWindowOpenTempCorrection5, _windowOpenAction5Handled);
                if (_windowOpenAction1Handled && _windowOpenAction2Handled && _windowOpenAction3Handled && _windowOpenAction4Handled && _windowOpenAction5Handled)
                {
                    _windowOpenTimer = 0;
                    logDebugP("All window open actions handled");
                }
            }
            break;
        case WindowOpenState::WaitDelayAfterClose:
            if (millis() - _windowOpenTimer >= ParamCLI_CHWindowCloseWaitTimeDelayTimeMS)
            {
                resetWindowOpenActions("Wait delay after window close");
            }
            break;
        case WindowOpenState::InitialWaitAfterClose:
            {
                const unsigned long intialWaitTime = 5 * 60 * 1000UL; // 5 minutes      
                if (millis() - _windowOpenTimer >= intialWaitTime)
                {
                    // Initial wait time after close
                    setWindowOpenState(WindowOpenState::WaitForStableRoomTemperature);
                    _lastCurrentRoomTemperatureRawKnx = KoCLI_CRoomTemp.value(DPT_Value_2_Ucount);
                    logInfoP("Start to detect room temperature change, current %0.1f°C", getTemperatureFromRawKnx(_lastCurrentRoomTemperatureRawKnx));
                
                }
            }
            break;
        case WindowOpenState::WaitForStableRoomTemperature:
            {
                const unsigned long gradientWindowWaittime = 8 * 60 * 1000UL; // 8 minutes
                uint16_t currentRoomTemperatureRawKnx = KoCLI_CRoomTemp.value(DPT_Value_2_Ucount);       
                float currentTemp = getTemperatureFromRawKnx(currentRoomTemperatureRawKnx);
                float originalTargetTemperature = getTemperatureFromRawKnx(_targetTemperatureBeforeWindowOpen == std::numeric_limits<uint16_t>::max() ? getTargetTemperatureRawKnx() :  _targetTemperatureBeforeWindowOpen);
                auto activeMode = _modeLockedWhileOpenWindow == ClimateModeSelection::Undefined ?  _currentActiveMode : _modeLockedWhileOpenWindow;
                if (activeMode == ClimateModeSelection::Heating && currentTemp >= originalTargetTemperature)
                {
                    logInfoP("Current room temperature %0.1f°C already reach target %0.1f°C, reset window open actions", currentTemp, originalTargetTemperature);
                    resetWindowOpenActions("Room temperature already reach heating target");
                }
                else if (activeMode == ClimateModeSelection::Cooling && currentTemp <= originalTargetTemperature)
                {
                    logInfoP("Current room temperature %0.1f°C already reach target %0.1f°C, reset window open actions", currentTemp, originalTargetTemperature);
                    resetWindowOpenActions("Room temperature already reach cooling target");
                }
                else if (activeMode != ClimateModeSelection::Cooling && activeMode != ClimateModeSelection::Heating)
                {
                    logInfoP("Current active mode is %s, no need to detect room temperature change, reset window open actions", ClimateModeSelectionHelper::toString(activeMode));
                    resetWindowOpenActions("Current mode ist not heating or cooling");
                }
                else if (millis() - _windowOpenTimer >= gradientWindowWaittime)
                {
                    // Time window for room temperature changed over
                    logInfoP("Stable room temperature since %dmin, reset actions", gradientWindowWaittime / 60000);
                    resetWindowOpenActions("Stable room temperature");
                }
                else
                {
                    if (_lastCurrentRoomTemperatureRawKnx != currentRoomTemperatureRawKnx)
                    {
                        float lastTemp = getTemperatureFromRawKnx(_lastCurrentRoomTemperatureRawKnx);
                        float offset = currentTemp - lastTemp;
                        if (activeMode == ClimateModeSelection::Cooling)
                            offset = -offset;
                        if (offset > 0.1f)
                        {
                            logInfoP("Detected room temperature %s from %0.1f°C to %0.1f°C, start new wait window", activeMode == ClimateModeSelection::Cooling ? "decrease" : "increase", lastTemp, currentTemp);
                            // start new time window with current temperature
                            _lastCurrentRoomTemperatureRawKnx = currentRoomTemperatureRawKnx;
                            _windowOpenTimer = max(1UL, millis());
                        }
                        else if (offset < 0)
                        {
                            logInfoP("Detected room temperature %s from %0.1f°C to %0.1f°C, reset window open actiond", activeMode == ClimateModeSelection::Cooling ? "increase" : "decrease", lastTemp, currentTemp);
                            resetWindowOpenActions("Detect negative room temperature change");
                        }
                    }
                }
            }
            break;
        default:
            logErrorP("Invalid window open state %d", _windowOpenState);
            resetWindowOpenActions("Invalid window open state");
            break;
    }
}


void RoomChannel::resetWindowOpenActions(const char* reason)
{
    if (_windowOpenAction1Handled || _windowOpenAction2Handled || _windowOpenAction3Handled || _windowOpenAction4Handled || _windowOpenAction5Handled)
        logInfoP("%s: Reset window open actions", reason);
    else
        logDebugP("%s: Reset window open actions", reason);
    setWindowOpenState(WindowOpenState::Closed);
    _windowOpenAction1Handled = false;
    _windowOpenAction2Handled = false;
    _windowOpenAction3Handled = false;
    _windowOpenAction4Handled = false;
    _windowOpenAction5Handled = false;
    if (_modeLockedWhileOpenWindow != ClimateModeSelection::Undefined)
    {
        logDebugP("Restore mode %s after window closed", ClimateModeSelectionHelper::toString(_modeLockedWhileOpenWindow));
        setMode(_modeLockedWhileOpenWindow);
        _modeLockedWhileOpenWindow = ClimateModeSelection::Undefined;
    }
    if (_targetTemperatureBeforeWindowOpen != std::numeric_limits<uint16_t>::max())
    {
        logDebugP("Restore target temperature %0.1f°C after window closed", getTemperatureFromRawKnx(_targetTemperatureBeforeWindowOpen));
        setTargetTemperatureRawKnx(_targetTemperatureBeforeWindowOpen, ChangeSource::Internal);
        _targetTemperatureBeforeWindowOpen = std::numeric_limits<uint16_t>::max();
    }
    if (_roomTemperatureBeforeWindowOpen != std::numeric_limits<uint16_t>::max())
    {
         _roomTemperatureBeforeWindowOpen = std::numeric_limits<uint16_t>::max();
        logDebugP("Restore room temperature %0.1f°C after window closed", getTemperatureFromRawKnx(_roomTemperature));
        for (auto &device : _climateDevices)
        {
            device->setRoomTemperature(_roomTemperature);
        }
        handleAuto();
    }
}

void RoomChannel::setMode(ClimateModeSelection mode)
{
    _modeLockedWhileOpenWindow = ClimateModeSelection::Undefined;
    if (_currentMode != mode)
    {
        switch (mode)
        {
            case ClimateModeSelection::Auto:
                if (ParamCLI_CHAutoDeviceSelection == PT_CLIDeviceSelection::Disabled)
                {
                    logInfoP("Auto mode not supported, keep current mode %s", ClimateModeSelectionHelper::toString(_currentMode));
                    mode = _currentMode;
                }
                else
                {
                    logInfoP("Mode changed to 'Auto'");
                }
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
            resetWindowOpenActions("Mode changed");
        
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
            if (_currentPower == PowerState::Off)
            {
                _currentMode = ClimateModeSelection::Off;
            }
            else
            {
                _currentMode = mode;
                _lastActiveMode = mode;
            }
            setTargetTemperatureRawKnx(getTargetTemperatureRawKnx(), ChangeSource::User); // Update target temperature if needed for new mode
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
    if (ParamCLI_CHModeFeedbackReturnOff == PT_CLIModeFeedback::ReturnOff)
         KoCLI_CModeSelectionFb.valueCompare((uint8_t)_currentMode, DPT_DecimalFactor);
    else
        KoCLI_CModeSelectionFb.valueCompare((uint8_t)_lastActiveMode, DPT_DecimalFactor);
    if (_forceSendMode)
    {
        KoCLI_CModeSelectionFb.objectWritten();
        _forceSendMode = false;
    }
    if (!handleAuto())
    {
        handleMode(_currentMode);
    }
}

void RoomChannel::handleMode(ClimateModeSelection mode)
{
    _currentActiveMode = mode;
    bool needTargetTemperature = false;
    switch (mode)
    {
        case ClimateModeSelection::Cooling:
            _useCoolingTargetTemperature = ParamCLI_CH2TargetTemp;
            needTargetTemperature = true;
            break;
        case ClimateModeSelection::Heating:
            _useCoolingTargetTemperature = false;
            needTargetTemperature = true;
            break;
        case ClimateModeSelection::Fan:
        case ClimateModeSelection::Dehumification:
        case ClimateModeSelection::Off:
            break;
        default:
            logInfoP("Invalid mode %s", ClimateModeSelectionHelper::toString(mode));
            break;
    }
    if (_targetTemperatureSetWhileStartingRawKnx != std::numeric_limits<uint16_t>::max())
    {
        if (_useCoolingTargetTemperature)
            _targetTemperatureHeatingRawKnx = _targetTemperatureCoolingRawKnx;
        setTargetTemperatureRawKnx(_targetTemperatureSetWhileStartingRawKnx, ChangeSource::Internal);
        _targetTemperatureSetWhileStartingRawKnx = std::numeric_limits<uint16_t>::max();
    }
    else
    {
        // Force to update target temperature for new mode
        setTargetTemperatureRawKnx(getTargetTemperatureRawKnx(), ChangeSource::Internal);
    }

    if (mode != ClimateModeSelection::Off && _currentPower == PowerState::Off)
    {
        // Power is off, turn of devices
        for (auto &device : _climateDevices)
        {
            device->setMode(ClimateModeSelection::Off);
        }
        isActiveChangedFromDevice();
        return;
    }
    bool supported = false;
    for (auto &device : _climateDevices)
    {
        if (device->supportMode(mode))
        {
            supported = true;
            device->setMode(mode);
            if (needTargetTemperature)
                device->setTargetTemperature(getTargetTemperatureRawKnx());     
        }
        else
        {
            device->setMode(ClimateModeSelection::Off);
        }
    }
    isActiveChangedFromDevice();
    if (!supported)
    {
        logWarningP("No device supports mode %s", ClimateModeSelectionHelper::toString(mode));
    }
}

bool RoomChannel::handleAuto()
{   
    if (_currentMode != ClimateModeSelection::Auto || ParamCLI_CHAutoDeviceSelection != RoomChannel::OpenKNX)
        return false;
    if (_inHandleAuto)
    {
        logWarningP("Already in handleAuto, skip to avoid loop");
        return false;
    }
    _inHandleAuto = true;
    logDebugP("Handle auto mode");
    uint16_t roomTemperatureRawKnx = _roomTemperatureBeforeWindowOpen != std::numeric_limits<uint16_t>::max() ? _roomTemperatureBeforeWindowOpen : _roomTemperature;
    if (roomTemperatureRawKnx == std::numeric_limits<uint16_t>::max())
    {
        logDebugP("Room temperature not yet received, cannot handle auto mode, use target temperature as fallback");
        roomTemperatureRawKnx = _useCoolingTargetTemperature ? _targetTemperatureCoolingRawKnx : _targetTemperatureHeatingRawKnx; // Use target temperature as fallback to avoid not working if room temperature is not yet received
    }
    float roomTemperature = getTemperatureFromRawKnx(roomTemperatureRawKnx);
    float targetTemperature = getTemperatureFromRawKnx(getTargetTemperatureRawKnx());
    float hysteresis = 0.0f;
    if (ParamCLI_CHHeatingSummerAllowed || openknxClimateControlModule.isWinter())
    {
        if (ParamCLI_CHAutoHeatDeviceSelection == PT_CLIAutomaticDevice::RequestByGroupObject)
        {
            if (KoCLI_CAutoReqHeat.value(DPT_Switch))
            {
                logDebugP("Auto mode: Request heating by group object");
                handleMode(ClimateModeSelection::Heating);
                _inHandleAuto = false;
                return true;
            }
        }  
        else  if (_currentActiveMode == ClimateModeSelection::Heating)
        {  
            logDebugP("Current active mode is heating, check if we should keep heating with hysteresis");
            // Hystersis Heating
            switch (ParamCLI_CHHysteresisHeating)
            {
                case PT_CLIHysteresis::Hysteresis0_5:
                    hysteresis = -0.5f;
                    break;
                case PT_CLIHysteresis::Hysteresis1_0:
                    hysteresis = -1.0f;
                    break;
                case PT_CLIHysteresis::Hysteresis2_0:
                    hysteresis = -2.0f;
                    break;
            }
            if (roomTemperature + hysteresis < targetTemperature)
            {
                logDebugP("Room %0.1f°C is below %0.1f°C with hysteresis %0.1f°C, keep heating", roomTemperature, targetTemperature, hysteresis);
                handleMode(ClimateModeSelection::Heating);
                _inHandleAuto = false;
                return true;
            }
            logDebugP("Room %0.1f°C is above %0.1f°C with hysteresis %0.1f°C", roomTemperature, targetTemperature, hysteresis);
        }
    }
    if (ParamCLI_CHCoolingWinterAllowed || openknxClimateControlModule.isSummer())
    {
        if (ParamCLI_CHAutoCoolDeviceSelection == PT_CLIAutomaticDevice::RequestByGroupObject)
        {
            if (KoCLI_CAutoReqCool.value(DPT_Switch))
            {
                logDebugP("Auto mode: Request cooling by group object");
                handleMode(ClimateModeSelection::Cooling);
                _inHandleAuto = false;
                return true;
            }
        }   
        else if (_currentActiveMode == ClimateModeSelection::Cooling)
        {
            logDebugP("Current active mode is cooling, check if we should keep cooling with hysteresis");
            // Hystersis Cooling
            switch (ParamCLI_CHHysteresisCooling)
            {
                case PT_CLIHysteresis::Hysteresis0_5:
                    hysteresis = 0.5f;
                    break;
                case PT_CLIHysteresis::Hysteresis1_0:
                    hysteresis = 1.0f;
                    break;
                case PT_CLIHysteresis::Hysteresis2_0:
                    hysteresis = 2.0f;
                    break;
            }
            if (roomTemperature + hysteresis > targetTemperature)
            {
                logDebugP("Room %0.1f°C is above %0.1f°C with hysteresis %0.1f°C, keep cooling", roomTemperature, targetTemperature, hysteresis);
                handleMode(ClimateModeSelection::Cooling);
                _inHandleAuto = false;
                return true;
            }
            logDebugP("Room %0.1f°C is below %0.1f°C with hysteresis %0.1f°C", roomTemperature, targetTemperature, hysteresis);
        }
    }
    if (_heatingSupported && 
        ParamCLI_CHAutoHeatDeviceSelection == PT_CLIAutomaticDevice::OpenKNXAutomatic && 
        (ParamCLI_CHHeatingSummerAllowed || openknxClimateControlModule.isWinter()))
    {
        float heatingTemperature = getTemperatureFromRawKnx(_targetTemperatureHeatingRawKnx);
        logDebugP("Check heating with room %0.1f°C and target %0.1f°C", roomTemperature, heatingTemperature);
        if (roomTemperature <= heatingTemperature)
        {
            logDebugP("Room %0.1f°C is below %0.1f°C, switch to heating", roomTemperature, heatingTemperature);
            handleMode(ClimateModeSelection::Heating);
            _inHandleAuto = false;
            return true;
        }
        logDebugP("Room %0.1f°C is above %0.1f°C, do not switch to heating", roomTemperature, heatingTemperature);
    }
    if (_coolingSupported && 
        ParamCLI_CHAutoCoolDeviceSelection == PT_CLIAutomaticDevice::OpenKNXAutomatic && 
        (ParamCLI_CHCoolingWinterAllowed || openknxClimateControlModule.isSummer()))
    {
        float coolingTemperature = getTemperatureFromRawKnx(ParamCLI_CH2TargetTemp ? _targetTemperatureCoolingRawKnx : _targetTemperatureHeatingRawKnx);
        logDebugP("Check cooling with room %0.1f°C and target %0.1f°C", roomTemperature, coolingTemperature);
        if (roomTemperature >= coolingTemperature)
        {
            logDebugP("Room %0.1f°C is above %0.1f°C, switch to cooling", roomTemperature, coolingTemperature);
            handleMode(ClimateModeSelection::Cooling);
            _inHandleAuto = false;
            return true;
        }
        logDebugP("Room %0.1f°C is below %0.1f°C, do not switch to cooling", roomTemperature, coolingTemperature);
    }
    if (ParamCLI_CHAutoDehumDeviceSelection == PT_CLIAutomaticDevice::RequestByGroupObject)
    {
        if (KoCLI_CAutoReqDehum.value(DPT_Switch))
        {
            logDebugP("Auto mode: Request dehumification by group object");
            handleMode(ClimateModeSelection::Dehumification);
            _inHandleAuto = false;
            return true;
        }
    }
    if (ParamCLI_CHAutoFanDeviceSelection == PT_CLIAutomaticDevice::RequestByGroupObject)
    {
        if (KoCLI_CAutoReqFan.value(DPT_Switch))
        {
            logDebugP("Auto mode: Request fan by group object");
            handleMode(ClimateModeSelection::Fan);
            _inHandleAuto = false;
            return true;
        }
    }
    logDebugP("Fall back to auto mode 'Off'");
    handleMode(ClimateModeSelection::Off);  
    _inHandleAuto = false;
    return true;
}
void RoomChannel::logStatus()
{
    logInfoP("Power: %s", ((bool) KoCLI_CPowerFb.value(DPT_Switch)) ? "On" : "Off");
    logInfoP("Current mode: %s", ClimateModeSelectionHelper::toString(_currentMode));
    logInfoP("Last activemode: %s", ClimateModeSelectionHelper::toString(_lastActiveMode));
    logInfoP("Active mode: %s", ClimateModeSelectionHelper::toString(_currentActiveMode));
    logInfoP("Room temperature: %0.1f°C", getTemperatureFromRawKnx(_roomTemperature));
    if (ParamCLI_CH2TargetTemp)
    {
        logInfoP("Target temperature for heating: %0.1f°C", getTemperatureFromRawKnx(_targetTemperatureHeatingRawKnx));
        logInfoP("Target temperature for cooling: %0.1f°C", getTemperatureFromRawKnx(_targetTemperatureCoolingRawKnx));
    }
    else
    {
        logInfoP("Target temperature: %0.1f°C", getTemperatureFromRawKnx(_targetTemperatureCoolingRawKnx));
    }
    for (auto &device : _climateDevices)
    {
        device->logStatus();
    }
    if (_windowOpenState != WindowOpenState::Closed)
    {
        logInfoP("Window open action 1 %s", _windowOpenAction1Handled ? "done" : "not yet done");
        logInfoP("Window open action 2 %s", _windowOpenAction2Handled ? "done" : "not yet done");
        logInfoP("Window open action 3 %s", _windowOpenAction3Handled ? "done" : "not yet done");
        logInfoP("Window open action 4 %s", _windowOpenAction4Handled ? "done" : "not yet done");
        logInfoP("Window open action 5 %s", _windowOpenAction5Handled ? "done" : "not yet done");
    }
    switch (_windowOpenState)
    {
        case WindowOpenState::Closed:
            logInfoP("Window is closed");
            break;
        case WindowOpenState::Open:
            if (_windowOpenTimer != 0)
                logInfoP("Window is open since %lu seconds", (millis() - _windowOpenTimer) / 1000);
            else
                logInfoP("Window is open");
            break;
        case WindowOpenState::WaitDelayAfterClose:
            logInfoP("Window is closed, wait delay after close since %lu seconds", (millis() - _windowOpenTimer) / 1000);
            break;
        case WindowOpenState::InitialWaitAfterClose:
            logInfoP("Window is closed, initial wait after close since %lu seconds", (millis() - _windowOpenTimer) / 1000);
            break;
        case WindowOpenState::WaitForStableRoomTemperature:
            logInfoP("Window is closed, wait for stable room temperature since %lu seconds", (millis() - _windowOpenTimer) / 1000);
            break;
        default:
            logInfoP("Invalid window open state %d", _windowOpenState);
            break;
    }
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

void RoomChannel::setModeFromDevice(ClimateModeSelection mode, ClimateDevice &device)
{
    if (!_started)
    {
        logInfoP("Received mode %s from device %d while starting, ignoring", ClimateModeSelectionHelper::toString(mode), device.deviceNumber());
        return;
    }
    

    if (mode == ClimateModeSelection::Off && device.supportMode(_currentActiveMode))
    {
        // handle override
        if (_currentMode == ClimateModeSelection::Auto && ParamCLI_CHAutoDeviceSelection == OpenKNX)
        {
            switch (ParamCLI_CHBehaviorOnDeviceChange)
            {
                case PT_CLIBehaviorDeviceChange::OpenKNXOverridesSelection:
                    logInfoP("Auto mode overides device mode %s from device %d", ClimateModeSelectionHelper::toString(mode), device.deviceNumber());
                    handle();
                    return;
                case PT_CLIBehaviorDeviceChange::LeavesAutomaticModeTemp:
                    _autoModeFallbackTimer = max(1UL, millis());
                    logInfoP("Device %d changed mode to %s, will fallback to auto mode after %lu seconds", device.deviceNumber(), ClimateModeSelectionHelper::toString(mode), ParamCLI_CHFallbackAutoWaitTimeDelayTimeMS / 1000);
                    break;
                default:
                    break;
            }
        }

        logInfoP("Set mode 'Off' through device %d", device.deviceNumber());
        setMode(mode);
        return;
    }
    if (device.supportMode(mode))
    {
        // handle override
        if (_currentMode == ClimateModeSelection::Auto && ParamCLI_CHAutoDeviceSelection == OpenKNX)
        {
            switch (ParamCLI_CHBehaviorOnDeviceChange)
            {
                case PT_CLIBehaviorDeviceChange::OpenKNXOverridesSelection:
                    logInfoP("Auto mode overides device mode %s from device %d", ClimateModeSelectionHelper::toString(mode), device.deviceNumber());
                    handle();
                    return;
                case PT_CLIBehaviorDeviceChange::LeavesAutomaticModeTemp:
                    _autoModeFallbackTimer = max(1UL, millis());
                    logInfoP("Device %d changed mode to %s, will fallback to auto mode after %lu seconds", device.deviceNumber(), ClimateModeSelectionHelper::toString(mode), ParamCLI_CHFallbackAutoWaitTimeDelayTimeMS / 1000);
                    break;
                default:
                    break;
            }
        }
        logInfoP("Set mode %s through device %d", ClimateModeSelectionHelper::toString(mode), device.deviceNumber());
        setMode(mode);
        return;
    }
    else
    {
        logInfoP("Received not allowed mode %s from device %d, ignored", ClimateModeSelectionHelper::toString(mode), device.deviceNumber());
    }
}

void RoomChannel::setTargetTemperatureFromDevice(uint16_t targetTemperatureRawKnx, ClimateDevice &device)
{
    if (!_started)
    {
        logInfoP("Received temperature feedback %0.1f°C from device %d while starting, ignoring", getTemperatureFromRawKnx(targetTemperatureRawKnx), device.deviceNumber());
        return;
    }
    if (_targetTemperatureBeforeWindowOpen != std::numeric_limits<uint16_t>::max())
    {
        logInfoP("Received temperature feedback %0.1f°C from device %d while window open, ignoring", getTemperatureFromRawKnx(targetTemperatureRawKnx), device.deviceNumber());
        return;
    }
    auto deviceNumber = device.deviceNumber();
    if (_currentActiveMode == ClimateModeSelection::Cooling && device.supportMode(ClimateModeSelection::Cooling))
    {
        logInfoP("Received cooling temperature feedback %0.1f°C from active device %d", getTemperatureFromRawKnx(targetTemperatureRawKnx), deviceNumber);
        setTargetTemperatureRawKnx(targetTemperatureRawKnx, ChangeSource::Device);
        return;
    }
    if (_currentActiveMode == ClimateModeSelection::Heating && device.supportMode(ClimateModeSelection::Heating))
    {
        logInfoP("Received heating temperature feedback %0.1f°C from active device %d", getTemperatureFromRawKnx(targetTemperatureRawKnx), deviceNumber);
        setTargetTemperatureRawKnx(targetTemperatureRawKnx, ChangeSource::Device);
        return;
    }
    if (ParamCLI_CH2TargetTemp)
    {
        if (device.supportMode(ClimateModeSelection::Cooling))
        {
            logInfoP("Received cooling temperature feedback %0.1f°C from device %d which is not active", getTemperatureFromRawKnx(targetTemperatureRawKnx), deviceNumber);
            _targetTemperatureCoolingRawKnx = targetTemperatureRawKnx;
            return;
        }
        else if (device.supportMode(ClimateModeSelection::Heating))
        {
            logInfoP("Received heating temperature feedback %0.1f°C from device %d which is not active", getTemperatureFromRawKnx(targetTemperatureRawKnx), deviceNumber);
            _targetTemperatureHeatingRawKnx = targetTemperatureRawKnx;
            return;
        }
    }
    logWarningP("Received temperature feedback %0.1f°C from device %d which will be dropped", getTemperatureFromRawKnx(targetTemperatureRawKnx), deviceNumber);
}

void RoomChannel::setTargetTemperatureRawKnx(uint16_t targetTemperatureRawKnx, ChangeSource changeSource)
{
    if (!_started)
    {
        _targetTemperatureSetWhileStartingRawKnx = targetTemperatureRawKnx;
        logDebugP("Set target temperature to %0.1f°C while starting", getTemperatureFromRawKnx(targetTemperatureRawKnx));
        return;
    }
    uint16_t currentTargetTemperatureRawKnx = getTargetTemperatureRawKnx();
  
    if (changeSource == ChangeSource::User)
    {
        resetWindowOpenActions("Target temperature changed");
    }
    uint8_t roundingParam = 0;
    if (changeSource != ChangeSource::Device)
    {
        // Rounding must not be used, if temperarture is cooming from device feedback
        ClimateModeSelection searchForMode = _currentActiveMode;
        if (searchForMode != ClimateModeSelection::Cooling && searchForMode != ClimateModeSelection::Heating)
        {
            searchForMode = _useCoolingTargetTemperature ? ClimateModeSelection::Cooling : ClimateModeSelection::Heating;
        }
        for (auto &device : _climateDevices)
        {
            if (device->supportMode(searchForMode))
            {
                auto roundingParamDevice = device->getRoundingParameter();
                if (roundingParamDevice > roundingParam)
                {
                    roundingParam = roundingParamDevice;
                }
            }
        }
    }
    if (_useCoolingTargetTemperature)
    {
        targetTemperatureRawKnx = limitSetTemperature(_forceSendTargetTemperature, ParamCLI_CH2TargetTemp ? "Cooling" : "", _targetTemperatureCoolingRawKnx, targetTemperatureRawKnx, ParamCLI_CHTargetMinCooling, ParamCLI_CHTargetMaxCooling, roundingParam) ;
        _targetTemperatureCoolingRawKnx = targetTemperatureRawKnx;
    }
    else
    {
        targetTemperatureRawKnx = limitSetTemperature(_forceSendTargetTemperature, ParamCLI_CH2TargetTemp ? "Heating" : "", _targetTemperatureHeatingRawKnx, targetTemperatureRawKnx, ParamCLI_CHTargetMinHeating, ParamCLI_CHTargetMaxHeating, roundingParam);
        _targetTemperatureHeatingRawKnx = targetTemperatureRawKnx;
    }
    KoCLI_CTargetTempFb.valueCompare(targetTemperatureRawKnx, DPT_Value_2_Ucount);
    if (_forceSendTargetTemperature)
    {
        KoCLI_CTargetTempFb.objectWritten();
        _forceSendTargetTemperature = false;
    }
    if (_currentActiveMode == ClimateModeSelection::Cooling || _currentActiveMode == ClimateModeSelection::Heating)
    {
        for (auto &device : _climateDevices)
        {
            if (device->supportMode(_currentActiveMode))
            {
                device->setTargetTemperature(targetTemperatureRawKnx);
            }
        }
    }
    if (currentTargetTemperatureRawKnx != getTargetTemperatureRawKnx())
        handleAuto();
}

void RoomChannel::isActiveChangedFromDevice()
{
    bool isActive = false;
    if (_currentActiveMode != ClimateModeSelection::Off)
    {
        for (auto &device : _climateDevices)
        {
            if (device->isActive() && device->supportMode(_currentActiveMode))
            {
                isActive = true;
                break;
            }
        }
    }
    if (isActive)
        KoCLI_CActiveMode.valueCompare((uint8_t)_currentActiveMode, DPT_DecimalFactor);
    else
        KoCLI_CActiveMode.valueCompare((uint8_t) ClimateModeSelection::Off, DPT_DecimalFactor);   
}

uint16_t RoomChannel::limitSetTemperature(bool& forceSend,  const char* tempType, uint16_t currentTargetTemperatureRawKnx, uint16_t targetTemperatureRawKnx, float minTemperature, float maxTemperature, uint8_t roundingParam)
{
    float targetTemperature = getTemperatureFromRawKnx(targetTemperatureRawKnx);
    float currentTargetTemperature = getTemperatureFromRawKnx(currentTargetTemperatureRawKnx);
    logDebugP("Set %s target temperature to %0.2f°C", tempType, getTemperatureFromRawKnx(targetTemperatureRawKnx));
    if (roundingParam != 0)
    {
        auto roundedTargetTemperature = roundTemperature(targetTemperature, roundingParam);
        auto rawRoundedTemp = getRawKnxFromTemperature(roundedTargetTemperature);
        if (rawRoundedTemp != currentTargetTemperatureRawKnx)
        {
            logDebugP("%s temperature rounded to %0.2f°C", tempType, roundedTargetTemperature);
            targetTemperature = roundedTargetTemperature;
            targetTemperatureRawKnx = rawRoundedTemp;
            forceSend = true;
        }
        else
        {
            logDebugP("check for round up or down");
            // Check for round up or down
            auto diff = targetTemperature - currentTargetTemperature;
            if (abs(diff) > 0.05f)
            {
                auto step = static_cast<float>(roundingParam) / 10.0f;
                if (diff > 0)
                {
                    targetTemperature = currentTargetTemperature + step;
                    targetTemperatureRawKnx = getRawKnxFromTemperature(targetTemperature);
                    logDebugP("%s temperature rounded up to %0.2f°C (diff: %0.2f°C)", tempType, targetTemperature, diff);
                }
                else
                {
                    targetTemperature = currentTargetTemperature - step;
                    targetTemperatureRawKnx = getRawKnxFromTemperature(targetTemperature);
                    logDebugP("%s temperature rounded down to %0.2f°C (diff: %0.2f°C)", tempType, targetTemperature, diff);
                }
                forceSend = true;
            }
            else
            {
                if (rawRoundedTemp != targetTemperatureRawKnx)
                {
                    targetTemperature = currentTargetTemperature;
                    targetTemperatureRawKnx = currentTargetTemperatureRawKnx;
                    logDebugP("%s temperature  %0.2f°C unchanged because minimal offset %0.2f", tempType, getTemperatureFromRawKnx(targetTemperatureRawKnx), diff);
                    forceSend = true;
                }
                else
                {
                    logDebugP("%s temperature unchanged", tempType);
                }
            }
        }
    }
    if (targetTemperature < minTemperature)
    {
        if (ParamCLI_CHTargetLimitHandling == PT_CLITargetLimitHandling::MinMax)
           targetTemperatureRawKnx = getRawKnxFromTemperature(minTemperature);
        else
            targetTemperatureRawKnx = currentTargetTemperatureRawKnx;
        forceSend = true;
        logDebugP("%s target temperature too low, set to minimum %0.2f°C", tempType, minTemperature);
    }
    else if (targetTemperature > maxTemperature)
    {
        if (ParamCLI_CHTargetLimitHandling == PT_CLITargetLimitHandling::MinMax)
            targetTemperatureRawKnx = getRawKnxFromTemperature(maxTemperature);
        else
            targetTemperatureRawKnx = currentTargetTemperatureRawKnx;
        forceSend = true;
        logDebugP("%s target temperature too high, set to maximum %0.2f°C", tempType, maxTemperature);
    }
    logDebugP("Finally set %s target temperature to %0.2f°C", tempType, getTemperatureFromRawKnx(targetTemperatureRawKnx));
    return targetTemperatureRawKnx;
}

uint16_t RoomChannel::roundTemperatureAndLimit(float targetTemperature, uint8_t roundingParam)
{
    float roundedTargetTemperature = roundTemperature(targetTemperature, roundingParam);
    if (_useCoolingTargetTemperature)
    {
        if (roundedTargetTemperature < ParamCLI_CHTargetMinCooling)
        {
            roundedTargetTemperature = ParamCLI_CHTargetMinCooling;
            logDebugP("Cooling target temperature too low after rounding, set to minimum %0.2f°C", roundedTargetTemperature);
        }
        else if (roundedTargetTemperature > ParamCLI_CHTargetMaxCooling)
        {
            roundedTargetTemperature = ParamCLI_CHTargetMaxCooling;
            logDebugP("Cooling target temperature too high after rounding, set to maximum %0.2f°C", roundedTargetTemperature);
        }
    }
    else
    {
        if (roundedTargetTemperature < ParamCLI_CHTargetMinHeating)
        {
            roundedTargetTemperature = ParamCLI_CHTargetMinHeating;
            logDebugP("Heating target temperature too low after rounding, set to minimum %0.2f°C", roundedTargetTemperature);
        }
        else if (roundedTargetTemperature > ParamCLI_CHTargetMaxHeating)
        {
            roundedTargetTemperature = ParamCLI_CHTargetMaxHeating;
            logDebugP("Heating target temperature too high after rounding, set to maximum %0.2f°C", roundedTargetTemperature);
        }
    }
    return getRawKnxFromTemperature(roundedTargetTemperature);
}

float RoomChannel::roundTemperature(float temperature, uint8_t roundingParam)
{
    if (roundingParam == 0)
        return temperature; // keine Rundung

    // 1 -> 0.1, 5 -> 0.5, 10 -> 1.0
    const float step = static_cast<float>(roundingParam) / 10.0f;

    return std::round(temperature / step) * step;
}

void RoomChannel::switchToWinter()
{
    switchSaison(ParamCLI_WinterModeChange, "winter");
}

void RoomChannel::switchToSummer()
{
    switchSaison(ParamCLI_SummerModeChange, "summer");
}

void RoomChannel::switchSaison(PT_CLIModeChange modeChange, const char* saison)
{
    switch (modeChange)
    {
        case PT_CLIModeChange::Deactivated:
            logInfoP("No mode change due to %s season", saison);
            break;
        case PT_CLIModeChange::Off:
            logInfoP("Switch to 'Off' mode due to %s season", saison);
            setMode(ClimateModeSelection::Off);
            break;
        case PT_CLIModeChange::Auto:
            logInfoP("Switch to 'Auto' mode due to %s season", saison);
            setMode(ClimateModeSelection::Auto);
            break;
        case PT_CLIModeChange::Heating:
            logInfoP("Switch to heating mode due to %s season", saison);
            setMode(ClimateModeSelection::Heating);
            break;
        case PT_CLIModeChange::Cooling:
            logInfoP("Switch to cooling mode due to %s season", saison);
            setMode(ClimateModeSelection::Cooling);
            break;
        case PT_CLIModeChange::Dehumification:
            logInfoP("Switch to dehumification mode due to %s season", saison);
            setMode(ClimateModeSelection::Dehumification);
            break;
        case PT_CLIModeChange::Fan:
            logInfoP("Switch to fan mode due to %s season", saison);
            setMode(ClimateModeSelection::Fan);
            break;
        default:
            logErrorP("Invalid %s mode change option %d", saison, modeChange);
            break;
    }
}

void RoomChannel::loop()
{
    if (_autoModeFallbackTimer != 0 && millis() - _autoModeFallbackTimer >= ParamCLI_CHFallbackAutoWaitTimeDelayTimeMS)
    {
        logInfoP("Fallback to auto mode after device change");
        _autoModeFallbackTimer = 0;
        setMode(ClimateModeSelection::Auto);
    }
    for (auto &device : _climateDevices)
    {
        device->loop();
    }
    handleWindowOpen();
}

void RoomChannel::handleWindowOpenAction(int actionNumber, uint32_t afterMS, PT_CLIWindowOpenCondition condition, PT_CLIWindowOpenAction action, unsigned long windowOpenSince, uint8_t setPointCorrectionParameter, bool& handled)
{
    if (handled)
        return;
    
    auto activeMode = _modeLockedWhileOpenWindow == ClimateModeSelection::Undefined ?  _currentActiveMode : _modeLockedWhileOpenWindow;
    switch (condition)
    {
        case PT_CLIWindowOpenCondition::Disabled:
            handled = true;
            logDebugP("Action %d disabled", actionNumber);
            return;
        case PT_CLIWindowOpenCondition::IfHeating:
            if (activeMode != ClimateModeSelection::Heating)
            {
                handled = true;
                logDebugP("Action for heating %d not allowed because of %s", actionNumber, ClimateModeSelectionHelper::toString(activeMode));
                return;
            }
            break;
        case PT_CLIWindowOpenCondition::IfCooling:
            if (activeMode != ClimateModeSelection::Cooling)
            {
                handled = true;
                logDebugP("Action for cooling %d not allowed because of %s", actionNumber, ClimateModeSelectionHelper::toString(activeMode));
                return;
            }
            break;
        case PT_CLIWindowOpenCondition::IfHeatingOrCooling:
            if (activeMode != ClimateModeSelection::Cooling && activeMode != ClimateModeSelection::Heating)
            {             
                handled = true;
                logDebugP("Action for heating/cooling %d not allowed because of %s", actionNumber, ClimateModeSelectionHelper::toString(activeMode));
                return;
            } 
            break;
        case PT_CLIWindowOpenCondition::Always:
            // Do nothing here, just continue to handle the action
            break;
    }
    if (windowOpenSince >= afterMS)
    {
        handled = true;
        switch (action)
        {
            case PT_CLIWindowOpenAction::DisableHeatingCooling:
            case PT_CLIWindowOpenAction::DisableHeatingCoolingOnlyInAuto:
                if (_modeLockedWhileOpenWindow != ClimateModeSelection::Undefined)
                {
                    logDebugP("Action %d: Mode already locked to %s, no action taken", actionNumber, ClimateModeSelectionHelper::toString(_modeLockedWhileOpenWindow));
                }
                else if ((_currentActiveMode == ClimateModeSelection::Cooling || _currentActiveMode == ClimateModeSelection::Heating) && (action == PT_CLIWindowOpenAction::DisableHeatingCooling || _currentMode == ClimateModeSelection::Auto))
                {
                    auto tempMode = _currentActiveMode;
                    logDebugP("Action %d: Heating/Cooling disabled", actionNumber);
                    setMode(ClimateModeSelection::Off);
                    _modeLockedWhileOpenWindow = tempMode;
                }
                else
                {
                    logDebugP("Action %d: Heating/Cooling already off, no action taken", actionNumber);
                }
                break;
            case PT_CLIWindowOpenAction::EnableHeatingCooling:
                if (_modeLockedWhileOpenWindow != ClimateModeSelection::Undefined)
                {
                    logDebugP("Action %d: set mode back to %s", actionNumber, ClimateModeSelectionHelper::toString(_modeLockedWhileOpenWindow));
                    setMode(_modeLockedWhileOpenWindow);
                    _modeLockedWhileOpenWindow = ClimateModeSelection::Undefined;
                }
                else
                {
                    logDebugP("Action %d: No mode locked, no action taken", actionNumber);
                }
                break;
            case PT_CLIWindowOpenAction::SetpointAdjustment:
                {
                    float setPointCorrectionDegree = static_cast<float>(setPointCorrectionParameter) / 10.0f;
                    if (_currentMode == ClimateModeSelection::Cooling || _currentMode == ClimateModeSelection::Heating)
                    {
                        if (_targetTemperatureBeforeWindowOpen != std::numeric_limits<uint16_t>::max())
                        {
                            logDebugP("Action %d: Target temperature before window open already stored as %0.1f°C, no action taken", actionNumber, getTemperatureFromRawKnx(_targetTemperatureBeforeWindowOpen));
                        }
                        else
                        {
                            float correctionOffset = _currentActiveMode == ClimateModeSelection::Cooling ? setPointCorrectionDegree : -setPointCorrectionDegree;
                            auto tempTemperatureBeforeWindowOpen = getTargetTemperatureRawKnx();
                            float currentTargetTemp = getTemperatureFromRawKnx(tempTemperatureBeforeWindowOpen);
                            float adjustedTargetTemp = currentTargetTemp + correctionOffset;
                            logDebugP("Action %d: Change current target %0.1f°C to %0.1f°C", actionNumber, currentTargetTemp, adjustedTargetTemp);
                            setTargetTemperatureRawKnx(getRawKnxFromTemperature(adjustedTargetTemp), ChangeSource::Internal);
                            _targetTemperatureBeforeWindowOpen = tempTemperatureBeforeWindowOpen;
                        }
                    }
                    else
                    {
                        logDebugP("Action %d: Current mode is neither heating nor cooling, no action taken", actionNumber);
                    }
                }
                break;
            case PT_CLIWindowOpenAction::RevertSetpointAdjustment:
                if (_targetTemperatureBeforeWindowOpen != std::numeric_limits<uint16_t>::max())
                {
                    logDebugP("Action %d: Revert target temperature to %0.1f°C", actionNumber, getTemperatureFromRawKnx(_targetTemperatureBeforeWindowOpen));
                    setTargetTemperatureRawKnx(_targetTemperatureBeforeWindowOpen, ChangeSource::Internal);
                    _targetTemperatureBeforeWindowOpen = std::numeric_limits<uint16_t>::max();
                }
                else
                {
                    logDebugP("Action %d: No target temperature before window open stored, no action taken", actionNumber);
                }
                break;
            case PT_CLIWindowOpenAction::DoNotForwardRoomTemperatureChange:
                if (KoCLI_CRoomTemp.initialized())
                {
                    if (_roomTemperatureBeforeWindowOpen != std::numeric_limits<uint16_t>::max())
                    {
                        logDebugP("Action %d: Room temperature before window open already stored as %0.1f°C, no action taken", actionNumber, getTemperatureFromRawKnx(_roomTemperatureBeforeWindowOpen));
                    }
                    else
                    {
                        _roomTemperatureBeforeWindowOpen = _roomTemperature;
                        logDebugP("Action %d: Store current room temperature %0.1f°C", actionNumber, getTemperatureFromRawKnx(_roomTemperatureBeforeWindowOpen));
                    }
                }
                else
                {
                    logDebugP("Action %d: Room temperature not initialized, no action taken", actionNumber);
                }
                break;
            case PT_CLIWindowOpenAction::ForwardRoomTemperatureChange:
                if (_roomTemperatureBeforeWindowOpen != std::numeric_limits<uint16_t>::max())
                {
                    _roomTemperatureBeforeWindowOpen = std::numeric_limits<uint16_t>::max();
                    logDebugP("Action %d: Forward room temperature change, set room temperature to %0.1f°C", actionNumber, getTemperatureFromRawKnx(_roomTemperatureBeforeWindowOpen));
                    for (auto &device : _climateDevices)
                    {
                        device->setRoomTemperature(_roomTemperature);
                    }
                    handleAuto();
                }
                else
                {
                    logDebugP("Action %d: No room temperature before window open stored, no action taken", actionNumber);
                }
                break;
            case PT_CLIWindowOpenAction::WindowOpenAlarm:
                logDebugP("Action %d: Trigger window open alarm", actionNumber);
                KoCLI_CWindowOpenAlarm.valueCompare(true, DPT_Switch);
                break;
            case PT_CLIWindowOpenAction::WindowOpenAlarmActiveHeatingCooling:
                {
                    if (activeMode == ClimateModeSelection::Cooling || activeMode == ClimateModeSelection::Heating)
                    {
                        logDebugP("Action %d: Trigger window open alarm because active mode is %s", actionNumber, ClimateModeSelectionHelper::toString(activeMode));
                        KoCLI_CWindowOpenAlarm.valueCompare(true, DPT_Switch);
                    }
                    else
                    {
                        logDebugP("Action %d: Do not trigger window open alarm because active mode is %s", actionNumber, ClimateModeSelectionHelper::toString(activeMode));
                    }
                }
                break;
            default:
                logDebugP("Action %d: Unknown action %d", actionNumber, (int)action);
                break;
        }
    };
}