#include "ClimateDeviceKnxActor.h"
#include "ClimateControlModule.h"
#include "RoomChannel.h"
#include "PIController.h"
#include "PWMController.h"
#include "TargetTemperatureManipulationController.h"

#define DeviceKoOffset (CLI_KoCDev2Power - CLI_KoCDev1Power)
#undef CLI_KoCalcNumber
#define CLI_KoCalcNumber(index) (index + CLI_KoBlockOffset + _channelIndex * CLI_KoBlockSize + _deviceIndex * DeviceKoOffset)
#undef CLI_KoCalcIndex
#define CLI_KoCalcIndex(number) ((number >= CLI_KoCalcNumber(0) && number < CLI_KoCalcNumber(CLI_KoBlockSize)) ? (number - CLI_KoBlockOffset) % CLI_KoBlockSize : -1)
#undef CLI_KoCalcChannel
#define CLI_KoCalcChannel(number) ((number >= CLI_KoBlockOffset && number < CLI_KoBlockOffset + CLI_ChannelCount * CLI_KoBlockSize) ? (number - CLI_KoBlockOffset) / CLI_KoBlockSize : -1)

#define DeviceParameterOffset (CLI_CHControlMode2 - CLI_CHControlMode1)
#undef CLI_ParamCalcIndex
#define CLI_ParamCalcIndex(index) (index + CLI_ParamBlockOffset + _channelIndex * CLI_ParamBlockSize + _deviceIndex * DeviceParameterOffset)

#define KoCLI_CHAVCOut KoCLI_CKo13
#define CLI_KoCHVACOut (CLI_KoCKo13 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHAVCOutFeedback KoCLI_CKo14
#define CLI_KoCHVACOutFeedback (CLI_KoCKo14 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHPowerOut KoCLI_CKo15
#define CLI_KoCHPowerOut (CLI_KoCKo15 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHPowerOutFeedback KoCLI_CKo16
#define CLI_KoCHPowerOutFeedback (CLI_KoCKo16 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHCoolingOut KoCLI_CKo13
#define CLI_KoCHCoolingOut (CLI_KoCKo13 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHCoolingOutFeedback KoCLI_CKo14
#define CLI_KoCHCoolingOutFeedback (CLI_KoCKo14 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHHeatinOut KoCLI_CKo15
#define CLI_KoCHHeatingOut (CLI_KoCKo16 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHHeatingOutFeedback KoCLI_CKo16
#define CLI_KoCHHeatingOutFeedback (CLI_KoCKo16 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHDehumificationOut KoCLI_CKo17
#define CLI_KoCHDehumificationOut (CLI_KoCKo17 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHDehumificationOutFeedback KoCLI_CKo18
#define CLI_KoCHDehumificationOutFeedback (CLI_KoCKo18 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHFanOut KoCLI_CKo19
#define CLI_KoCHFanOut (CLI_KoCKo19 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHFanOutFeedback KoCLI_CKo20
#define CLI_KoCHFanOutFeedback (CLI_KoCKo20 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CDevPower KoCLI_CDev1Power
#define CLI_KoCDevPower (CLI_KoCDev1Power + _deviceIndex * DeviceKoOffset)

#define KoCLI_CDevSet KoCLI_CDev1Set
#define CLI_KoCDevSet (CLI_KoCDev1Set + _deviceIndex * DeviceKoOffset)

#define KoCLI_CDevPWM KoCLI_CDev1PWM
#define CLI_KoCDevPWM (CLI_KoCDev1PWM + _deviceIndex * DeviceKoOffset)

#define KoCLI_CDevSetFb KoCLI_CDev1SetFb
#define CLI_KoCDevSetFb (CLI_KoCDev1SetFb + _deviceIndex * DeviceKoOffset)

#define KoCLI_CDevRoomTemp KoCLI_CDev1RoomTemp
#define CLI_KoCDevRoomTemp (CLI_KoCDev1RoomTemp + _deviceIndex * DeviceKoOffset)

#define KoCLI_CDevIsActive KoCLI_CDev1IsActive
#define CLI_KoCDevIsActive (CLI_KoCDev1IsActive + _deviceIndex * DeviceKoOffset)


ClimateDeviceKnxActor::ClimateDeviceKnxActor(
    int channelIndex, 
    int deviceIndex, 
    RoomChannel& roomChannel, 
    bool supportHeating, 
    bool supportCooling, 
    bool supportDehumification, 
    bool supportFan,
    bool supportAuto) : 
    ClimateDevice(channelIndex, deviceIndex, roomChannel),
    _supportHeating(supportHeating),
    _supportCooling(supportCooling),
    _supportDehumification(supportDehumification),
    _supportFan(supportFan),
    _supportAuto(supportAuto)
{
    switch (ParamCLI_CHIsActive1)
    {
        case PT_CLIIsActive::FeedbackOnOff:
        case PT_CLIIsActive::FeedbackPercent:
            KoCLI_CDevIsActive.requestObjectRead();
            break;
    }
    if  (ParamCLI_CHControlTemperature1 == PT_CLIControlTemperature::Setpoint || ParamCLI_CHControlTemperature1 == PT_CLIControlTemperature::PulseWidthModulation)
    {
        // <Enumeration Text="Fußbodenheizung (5K / 160min)" Value="0" Id="%ENID%" op:headerName="FloorHeating" />
        // <Enumeration Text="Radiator (3K / 80min)" Value="1" Id="%ENID%" op:headerName="Radiator" />
        // <Enumeration Text="Luftheizung (2K / 30min)" Value="2" Id="%ENID%" op:headerName="AirHeating" />
        // <Enumeration Text="Benutzerdefiniert" Value="15" Id="%ENID%" op:headerName="Custom" />

        switch (ParamCLI_CHPIPreset1)
        {
            case PT_CLIPIPreset::FloorHeating:
                _piController = new PIController(5.0f, 160.0f * 60.0f);
                break;
            case PT_CLIPIPreset::Radiator:
                _piController = new PIController(3.0f, 80.0f * 60.0f);
                break;
            case PT_CLIPIPreset::AirHeating:
                _piController = new PIController(2.0f, 30.0f * 60.0f);
                break;
            case PT_CLIPIPreset::Custom:
                _piController = new PIController(ParamCLI_CHPII1, ParamCLI_CHPID1 * 60.0f);
                break;
                 
        }
    }
    if (ParamCLI_CHControlTemperature1 == PT_CLIControlTemperature::PulseWidthModulation)
    {
        _pwmController = new PWMController(ParamCLI_CHPWM1);
        KoCLI_CDevPWM.value(false, DPT_Switch);
    }
    if (ParamCLI_CHControlTemperature1 == PT_CLIControlTemperature::FakeSetTemperature)
    {
        _targetTemperatureManipulationController = new TargetTemperatureManipulationController();
        if (KoCLI_CDevRoomTemp.initialized())
        {
            _targetTemperatureManipulationController->setRoomTemperatureFromDevice(KoCLI_CDevRoomTemp.value(DPT_Value_Temp));
        }
        else
        {
            KoCLI_CDevRoomTemp.requestObjectRead();
        }
    }

}


int ClimateDeviceKnxActor::deviceNumber() const
{
    return _deviceIndex + 1;
}

ClimateModeSelection ClimateDeviceKnxActor::currentMode()
{
    return _mode;
}

void ClimateDeviceKnxActor::processInputKo(GroupObject& ko)
{
    int koNr = CLI_KoCalcIndex(ko.asap());
    if (koNr == CLI_KoCDevSetFb)
    {

        logInfoP("Device %d received ko %d (%d)", deviceNumber(), (int)ko.asap(), koNr);
        if (_targetTemperatureManipulationController == nullptr)
        {
            if (_blockForwardTemperatureFeedbackFromDevice != 0)
            {
                _needSendTargetTemperaturFromDevice = true;
            }
            else
            {
                _inReceiveTempFeedbackKo = true;
                _roomChannel.setTargetTemperatureFromDevice(ko.value(DPT_Value_2_Ucount), *this);
                _inReceiveTempFeedbackKo = false;
            }
        }
        else
        {
            logWarningP("Ignore target temperature feedback %0.1f °C from device %d because in temperature fake mode", _roomChannel.getTemperatureFromRawKnx(ko.value(DPT_Value_2_Ucount)), deviceNumber());
        }
    }
    else if (ParamCLI_CHControlMode1 == PT_CLIControlMode::HVAC_AND_POWER && koNr == CLI_KoCHPowerOutFeedback)
    {
        logInfoP("Device %d received ko %d (%d)", deviceNumber(), (int)ko.asap(), koNr);
        bool power = ko.value(DPT_Switch);
        if (power && KoCLI_CHAVCOutFeedback.initialized())
        {
            _inReceiveModeFeedbackKo = true;
            _roomChannel.setModeFromDevice((ClimateModeSelection)(uint8_t) KoCLI_CHAVCOutFeedback.value(DPT_Value_2_Ucount), *this);
            _inReceiveModeFeedbackKo = false;
        }
        else if (!power)
        {
            _inReceiveModeFeedbackKo = true;
            _roomChannel.setModeFromDevice(ClimateModeSelection::Off, *this);
            _inReceiveModeFeedbackKo = false;
        }

    }
    else if ((ParamCLI_CHControlMode1 == PT_CLIControlMode::HVAC || ParamCLI_CHControlMode1 == PT_CLIControlMode::HVAC_AND_POWER) && koNr == CLI_KoCHVACOutFeedback)
    {
        logInfoP("Device %d received ko %d (%d)", deviceNumber(), (int)ko.asap(), koNr);
        auto mode = (ClimateModeSelection)(uint8_t)ko.value(DPT_DecimalFactor);
        _inReceiveModeFeedbackKo = true;
        _roomChannel.setModeFromDevice(mode, *this);
        _inReceiveModeFeedbackKo = false;
    }
    else if (ParamCLI_CHControlMode1 == PT_CLIControlMode::ONE_OBJECT_PER_MODE && (koNr == CLI_KoCHHeatingOutFeedback || koNr == CLI_KoCHCoolingOutFeedback || koNr == CLI_KoCHDehumificationOutFeedback || koNr == CLI_KoCHFanOutFeedback))
    {
        _waitForSingeModeKosTimer = millis();
    }
    else if (_targetTemperatureManipulationController != nullptr && koNr == CLI_KoCDevRoomTemp)
    {
        float roomTemperatureFromDevice = ko.value(DPT_Value_Temp);
        logInfoP("Device %d received ko %d (%d)", deviceNumber(), (int)ko.asap(), koNr);
  
        logInfoP("Device %d received room temperature feedback %0.1f °C from device", deviceNumber(), roomTemperatureFromDevice);
        _targetTemperatureManipulationController->setRoomTemperatureFromDevice(roomTemperatureFromDevice);
    }
    else if (koNr == CLI_KoCDevIsActive)
    {
        switch (ParamCLI_CHIsActive1)
        {
            case PT_CLIIsActive::FeedbackOnOff:
                setIsActive(ko.value(DPT_Switch));
                break;
            case PT_CLIIsActive::FeedbackPercent:
                setIsActive(((float) ko.value(DPT_Scaling)) > 0.001);
                break;
        }
    }
}

void ClimateDeviceKnxActor::calculateIsActive()
{
    if (ParamCLI_CHIsActive1 == PT_CLIIsActive::Calculated && _piController == nullptr && _roomTemperatureRawKnx != std::numeric_limits<uint16_t>::max() && _targetTemperatureRawKnx != std::numeric_limits<uint16_t>::max())
    {
        float targetTemperature = _roomChannel.getTemperatureFromRawKnx(_targetTemperatureRawKnx);
        float roomTemperature = _roomChannel.getTemperatureFromRawKnx(_roomTemperatureRawKnx);
        switch (_mode)
        {
            case ClimateModeSelection::Heating:
                setIsActive(targetTemperature - roomTemperature > 0.01f);
                break;
            case ClimateModeSelection::Cooling:
                setIsActive(targetTemperature - roomTemperature < -0.01f);
                break;
            case ClimateModeSelection::Auto:
                setIsActive (abs(targetTemperature - roomTemperature) > 0.01f);
                break;
            case ClimateModeSelection::Fan:
            case ClimateModeSelection::Dehumification:
                setIsActive(true);
                break;
            default:
                setIsActive(false);
        }
        logDebugP("Calculate isActive to %s with room %0.1f °C and target %0.1f °C for mode %d", _isActive ? "active" : "inactive", roomTemperature, targetTemperature, (int)_mode);
    }
}

void ClimateDeviceKnxActor::setTargetTemperature(uint16_t targetTemperatureRawKnx)
{
    _targetTemperatureRawKnx = targetTemperatureRawKnx;
    if (!_inReceiveTempFeedbackKo)
        _blockForwardTemperatureFeedbackFromDevice = max(1UL, millis());
    auto targetTemperature = _roomChannel.getTemperatureFromRawKnx(targetTemperatureRawKnx);   
    logInfoP("Set target temperature to %0.1f °C", targetTemperature);
    calculateIsActive();
 
    if (_piController != nullptr)
    {
        _piController->setTargetTemperature(targetTemperature);
    }
    else if (_targetTemperatureManipulationController != nullptr)
    {
        _waitForSettingTargetTemperature = true;
        _targetTemperatureManipulationController->setTargetTemperature(targetTemperature);
    }
    else if (ParamCLI_CHControlTemperature1 == PT_CLIControlTemperature::TargetTemperature)
    {
        if (_turnOnTimer == 0)
        {
            logDebugP("Sending target temperature %0.1f °C to device %d", _roomChannel.getTemperatureFromRawKnx(targetTemperatureRawKnx), deviceNumber());
            KoCLI_CDevSet.valueNoSend(targetTemperatureRawKnx, DPT_Value_2_Ucount);
            if (!_inReceiveTempFeedbackKo)
                KoCLI_CDevSet.objectWritten();
        }
        else
        {
            logDebugP("Target temperature will be set to %0.1f °C after turn on delay", _roomChannel.getTemperatureFromRawKnx(targetTemperatureRawKnx));
            _waitForSettingTargetTemperature = true;
        }
    }
}

void ClimateDeviceKnxActor::setIsActive(bool active)
{
    if (_isActive != active)
    {
        _isActive = active;
        logInfoP("Device %d is now %s", deviceNumber(), active ? "active" : "inactive");
        _roomChannel.isActiveChangedFromDevice();
    }
}

void ClimateDeviceKnxActor::loop()
{
    if (_blockForwardTemperatureFeedbackFromDevice != 0 && millis() - _blockForwardTemperatureFeedbackFromDevice > 5000)
    {
        _blockForwardTemperatureFeedbackFromDevice = 0;
        if (_needSendTargetTemperaturFromDevice)
        {
            _needSendTargetTemperaturFromDevice = false;
            _inReceiveTempFeedbackKo = true;
            _roomChannel.setTargetTemperatureFromDevice(KoCLI_CDevSetFb.value(DPT_Value_2_Ucount), *this);
            _inReceiveTempFeedbackKo = false;

        }
    }
    if (_piController != nullptr && _roomTemperatureRawKnx != std::numeric_limits<uint16_t>::max() && _targetTemperatureRawKnx != std::numeric_limits<uint16_t>::max())
    {
        if (_piController->loop())
        {
            float positionValue = _piController->getPositionValue();
            if (KoCLI_CDevSet.valueCompare(positionValue, DPT_Scaling))
            {
                logDebugP("Setting position value to %d for device %d", (uint8_t) positionValue, deviceNumber());
            }
            bool isActive = positionValue > 0.001;
            setIsActive(isActive);
            if (_pwmController != nullptr)
            {
                _pwmController->setPositionValue(positionValue);
            }
        }
        if (_pwmController != nullptr)
        {
            if (_pwmController->loop())
            {
                bool pwmOutput = _pwmController->getPWMOutput();
                if (KoCLI_CDevPWM.valueCompare(pwmOutput, DPT_Switch))
                {
                    logDebugP("Setting PWM output to %d for device %d", pwmOutput, deviceNumber());
                    KoCLI_CDevPWM.value(pwmOutput, DPT_Switch);
                }
            }
        }
    }
    if (_turnOnTimer != 0 && millis() - _turnOnTimer > 500)
    {
        _turnOnTimer = 0;
        // Handle turn on delay end
        if (_waitForSettingTargetTemperature)
        {
            _waitForSettingTargetTemperature = false;
            if (_targetTemperatureManipulationController != nullptr)
            {
                float targetTemperature = _roomChannel.getTemperatureFromRawKnx(_targetTemperatureRawKnx);
                _targetTemperatureManipulationController->setTargetTemperature(targetTemperature);
                float adjustedTargetTemperature;
                _targetTemperatureManipulationController->loop(adjustedTargetTemperature);
                uint16_t limitedTargetTemperature = _roomChannel.roundTemperatureAndLimit(adjustedTargetTemperature, ParamCLI_CHTargetTempRounding1);
                logDebugP("Turn on delay passed, ajusted target temperature %0.1f °C (original %0.1f °C) to device %d", _roomChannel.getTemperatureFromRawKnx(limitedTargetTemperature), _roomChannel.getTemperatureFromRawKnx(_targetTemperatureRawKnx), deviceNumber());
                KoCLI_CDevSet.value(limitedTargetTemperature, DPT_Value_2_Ucount);
             
            }
            else
            {
                logDebugP("Turn on delay passed, sending target temperature %0.1f °C to device %d", _roomChannel.getTemperatureFromRawKnx(_targetTemperatureRawKnx), deviceNumber());
                KoCLI_CDevSet.value(_targetTemperatureRawKnx, DPT_Value_2_Ucount);
            }
        }
    }
    if (_targetTemperatureManipulationController != nullptr && _turnOnTimer == 0)
    {
        float adjustedTargetTemperature;
        if (_targetTemperatureManipulationController->loop(adjustedTargetTemperature))
        {
            logInfoP("Adjusted target temperature to %0.1f °C for device %d", adjustedTargetTemperature, deviceNumber());
            uint16_t limitedTargetTemperature = _roomChannel.roundTemperatureAndLimit(adjustedTargetTemperature, ParamCLI_CHTargetTempRounding1);
            logInfoP("Rounding target temperature to %0.1f °C (original %0.1f °C) for device %d", _roomChannel.getTemperatureFromRawKnx(limitedTargetTemperature), _roomChannel.getTemperatureFromRawKnx(_targetTemperatureRawKnx), deviceNumber());
            if (KoCLI_CDevSet.valueCompare(limitedTargetTemperature, DPT_Value_2_Ucount))
                logDebugP("Send target temperature %0.1f °C for device %d", _roomChannel.getTemperatureFromRawKnx(limitedTargetTemperature), deviceNumber());
        }
    }
    if (_waitForSingeModeKosTimer != 0 && millis() - _waitForSingeModeKosTimer > 500)
    {
        _waitForSingeModeKosTimer = 0;
        ClimateModeSelection mode = ClimateModeSelection::Off;
        if (_supportCooling && KoCLI_CHCoolingOutFeedback.value(DPT_Switch))
            mode = ClimateModeSelection::Cooling;
        else if (_supportHeating && KoCLI_CHHeatingOutFeedback.value(DPT_Switch))
            mode = ClimateModeSelection::Heating;
        else if (_supportDehumification && KoCLI_CHDehumificationOutFeedback.value(DPT_Switch))
            mode = ClimateModeSelection::Dehumification;
        else if (_supportFan && KoCLI_CHFanOutFeedback.value(DPT_Switch))
            mode = ClimateModeSelection::Fan;
        KoCLI_CHCoolingOutFeedback.valueNoSend(false, DPT_Switch);
        KoCLI_CHHeatingOutFeedback.valueNoSend(false, DPT_Switch);
        KoCLI_CHDehumificationOutFeedback.valueNoSend(false, DPT_Switch);
        KoCLI_CHFanOutFeedback.valueNoSend(false, DPT_Switch);

        logInfoP("Received single mode kos for device %d, setting mode %s", deviceNumber(), ClimateModeSelectionHelper::toString(mode));
        _inReceiveModeFeedbackKo = true;
        _roomChannel.setModeFromDevice(mode, *this);
        _inReceiveModeFeedbackKo = false;
    }
}

void ClimateDeviceKnxActor::setMode(ClimateModeSelection mode)
{
    logInfoP("Mode: %s", ClimateModeSelectionHelper::toString(mode));
    if (mode != _mode && (_mode == ClimateModeSelection::Undefined || _mode == ClimateModeSelection::Off))
    {
        _turnOnTimer = max(1UL, millis());
    }
    _mode = mode;
    switch (ParamCLI_CHControlMode1)
    {
        case PT_CLIControlMode::HVAC:
            KoCLI_CHAVCOut.value((uint8_t)mode, DPT_DecimalFactor);
            break;
        case PT_CLIControlMode::HVAC_AND_POWER:
            KoCLI_CHAVCOut.value((uint8_t)mode, DPT_DecimalFactor);
            KoCLI_CHPowerOut.value(mode != ClimateModeSelection::Off, DPT_Switch);
            break;
        case PT_CLIControlMode::ONE_OBJECT_PER_MODE:
            if (mode == ClimateModeSelection::Auto)
            {
                KoCLI_CHCoolingOut.value(true, DPT_Switch);
                KoCLI_CHHeatinOut.value(true, DPT_Switch);
            }
            else
            {
                KoCLI_CHCoolingOut.value(mode == ClimateModeSelection::Cooling, DPT_Switch);
                KoCLI_CHHeatinOut.value(mode == ClimateModeSelection::Heating, DPT_Switch);
            }
            KoCLI_CHDehumificationOut.value(mode == ClimateModeSelection::Dehumification, DPT_Switch);
            KoCLI_CHFanOut.value(mode == ClimateModeSelection::Fan, DPT_Switch);
            break;
    }
    if (_piController != nullptr)
    {
        _piController->setOperationMode(mode);
    }
    else if (_targetTemperatureManipulationController != nullptr)
    {
        _targetTemperatureManipulationController->setOperationMode(mode);
    }
    calculateIsActive();
}

void ClimateDeviceKnxActor::setRoomTemperature(uint16_t roomTemperatureRawKnx)
{
    _roomTemperatureRawKnx = roomTemperatureRawKnx;
    auto roomTemperature = _roomChannel.getTemperatureFromRawKnx(roomTemperatureRawKnx);
    logInfoP("Set room temperature: %0.1f °C", roomTemperature);
    calculateIsActive();
    if (_targetTemperatureManipulationController != nullptr)
    {
        _targetTemperatureManipulationController->setCurrentRoomTemperature(roomTemperature);
    }
    else
    {
        KoCLI_CDevRoomTemp.value(roomTemperatureRawKnx, DPT_Value_2_Ucount);
        if (_piController != nullptr)
        {
            _piController->setCurrentTemperature(roomTemperature);
        }
    }
}

bool ClimateDeviceKnxActor::isActive()
{
    return _isActive;
}

void ClimateDeviceKnxActor::logStatus()
{
    logInfoP("Mode %s", ClimateModeSelectionHelper::toString(_mode));
    logInfoP("Is active: %s", _isActive ? "yes" : "no");
    if (_targetTemperatureRawKnx != std::numeric_limits<uint16_t>::max())
        logInfoP("Target temperature: %0.1f°C", _roomChannel.getTemperatureFromRawKnx(_targetTemperatureRawKnx));
    if (_roomTemperatureRawKnx != std::numeric_limits<uint16_t>::max())
        logInfoP("Room temperature: %0.1f°C", _roomChannel.getTemperatureFromRawKnx(_roomTemperatureRawKnx));
    if (_piController != nullptr)
    {
        _piController->logStatus(logPrefix());
    }
    if (_pwmController != nullptr)
    {
        _pwmController->logStatus(logPrefix());
    }
    if (_targetTemperatureManipulationController != nullptr)
    {
        _targetTemperatureManipulationController->logStatus(logPrefix());
    }
}

bool ClimateDeviceKnxActor::supportMode(ClimateModeSelection mode)
{
    switch (mode)
    {
        case ClimateModeSelection::Cooling:
            return _supportCooling;
        case ClimateModeSelection::Heating:
            return _supportHeating;
        case ClimateModeSelection::Dehumification:
            return _supportDehumification;
        case ClimateModeSelection::Fan:
            return _supportFan;
        case ClimateModeSelection::Auto:
            return _supportAuto;
        default:
            return false;
    }
}

uint8_t ClimateDeviceKnxActor::getRoundingParameter()
{
    switch (ParamCLI_CHControlTemperature1)
    {
        case PT_CLIControlTemperature::TargetTemperature:
            return ParamCLI_CHTargetTempRounding1;
        default:
            return 0;
    }
}