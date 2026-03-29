#include "ClimateDevice.h"
#include "ClimateControlModule.h"
#include "RoomChannel.h"

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

#define KoCLI_CHAVCOut KoCLI_CKo8
#define CLI_KoCHVACOut (CLI_KoCKo8 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHAVCOutFeedback KoCLI_CKo9
#define CLI_KoCHVACOutFeedback (CLI_KoCKo9 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHPowerOut KoCLI_CKo10
#define CLI_KoCHPowerOut (CLI_KoCKo10 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHPowerOutFeedback KoCLI_CKo11
#define CLI_KoCHPowerOutFeedback (CLI_KoCKo11 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHCoolingOut KoCLI_CKo8
#define CLI_KoCHCoolingOut (CLI_KoCKo8 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHCoolingOutFeedback KoCLI_CKo9
#define CLI_KoCHCoolingOutFeedback (CLI_KoCKo9 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHHeatinOut KoCLI_CKo10
#define CLI_KoCHHeatingOut (CLI_KoCKo10 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHHeatingOutFeedback KoCLI_CKo11
#define CLI_KoCHHeatingOutFeedback (CLI_KoCKo11 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHDehumificationOut KoCLI_CKo12
#define CLI_KoCHDehumificationOut (CLI_KoCKo12 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHDehumificationOutFeedback KoCLI_CKo13
#define CLI_KoCHDehumificationOutFeedback (CLI_KoCKo13 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHFanOut KoCLI_CKo14
#define CLI_KoCHFanOut (CLI_KoCKo14 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHFanOutFeedback KoCLI_CKo15
#define CLI_KoCHFanOutFeedback (CLI_KoCKo15 + _deviceIndex * DeviceKoOffset)

#define KoCLI_CDevPower KoCLI_CDev1Power
#define CLI_KoCDevPower (CLI_KoCDev1Power + _deviceIndex * DeviceKoOffset)

#define KoCLI_CDevSet KoCLI_CDev1Set
#define CLI_KoCDevSet (CLI_KoCDev1Set + _deviceIndex * DeviceKoOffset)

#define KoCLI_CDevSetFb KoCLI_CDev1SetFb
#define CLI_KoCDevSetFb (CLI_KoCDev1SetFb + _deviceIndex * DeviceKoOffset)

#define KoCLI_CDevRoomTemp KoCLI_CDev1RoomTemp
#define CLI_KoCDevRoomTemp (CLI_KoCDev1RoomTemp + _deviceIndex * DeviceKoOffset)


ClimateDevice::ClimateDevice(
    int channelIndex, 
    int deviceIndex, 
    RoomChannel& roomChannel, 
    bool supportHeating, 
    bool supportCooling, 
    bool supportDehumification, 
    bool supportFan) : 
    _channelIndex(channelIndex),
    _deviceIndex(deviceIndex),
    _roomChannel(roomChannel),
    _supportHeating(supportHeating),
    _supportCooling(supportCooling),
    _supportDehumification(supportDehumification),
    _supportFan(supportFan)
{
    _name = openknx.logger.buildPrefix(roomChannel.name(), _channelIndex + 1) + "Dev" + std::to_string(deviceIndex + 1);
}

const std::string& ClimateDevice::logPrefix()
{
    return _name;
}

int ClimateDevice::deviceNumber() const
{
    return _deviceIndex + 1;
}

ClimateModeSelection ClimateDevice::currentMode()
{
    return _mode;
}

void ClimateDevice::processInputKo(GroupObject& ko)
{
    int koNr = CLI_KoCalcIndex(ko.asap());
    if (koNr == CLI_KoCDevSetFb)
    {

        logInfoP("Device %d received ko %d (%d)", deviceNumber(), (int)ko.asap(), koNr);
        if (ParamCLI_CHControlTemperature1 != PT_CLIControlTemperature::FakeSetTemperature)
        {
            _inReceiveTempFeedbackKo = true;
            _roomChannel.setTargetTemperatureFromDevice(ko.value(DPT_Value_2_Ucount), *this);
            _inReceiveTempFeedbackKo = false;
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
}

void ClimateDevice::setTargetTemperature(uint16_t targetTemperatureRawKnx)
{
    _targetTemperatureRawKnx = targetTemperatureRawKnx;
    logInfoP("Set target temperature to %0.1f °C", _roomChannel.getTemperatureFromRawKnx(targetTemperatureRawKnx));
    if (ParamCLI_CHControlTemperature1 != PT_CLIControlTemperature::FakeSetTemperature)
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
    else
    {
        logDebugP("Not sending target temperature %0.1f °C to device %d because in temperature fake mode", _roomChannel.getTemperatureFromRawKnx(targetTemperatureRawKnx), deviceNumber());
    }
}

void ClimateDevice::loop()
{
    if (_turnOnTimer != 0 && millis() - _turnOnTimer > 500)
    {
        _turnOnTimer = 0;
        if (_waitForSettingTargetTemperature)
        {
            _waitForSettingTargetTemperature = false;
            logDebugP("Turn on delay passed, sending target temperature %0.1f °C to device %d", _roomChannel.getTemperatureFromRawKnx(_targetTemperatureRawKnx), deviceNumber());
            KoCLI_CDevSet.value(_targetTemperatureRawKnx, DPT_Value_2_Ucount);
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

void ClimateDevice::setMode(ClimateModeSelection mode)
{
    logInfoP("Mode: %s", ClimateModeSelectionHelper::toString(mode));
    if (_mode != mode)
    {
        if (_mode == ClimateModeSelection::Undefined || mode == ClimateModeSelection::Off)
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
                KoCLI_CHCoolingOut.value(mode == ClimateModeSelection::Cooling, DPT_Switch);
                KoCLI_CHHeatinOut.value(mode == ClimateModeSelection::Heating, DPT_Switch);
                KoCLI_CHDehumificationOut.value(mode == ClimateModeSelection::Dehumification, DPT_Switch);
                KoCLI_CHFanOut.value(mode == ClimateModeSelection::Fan, DPT_Switch);
                break;
        }
    }
}

void ClimateDevice::setRoomTemperature(uint16_t roomTemperatureRawKnx)
{
    logInfoP("Set room temperature: %0.1f °C", _roomChannel.getTemperatureFromRawKnx(roomTemperatureRawKnx));
    KoCLI_CDevRoomTemp.value(roomTemperatureRawKnx, DPT_Value_2_Ucount);
}

void ClimateDevice::logStatus()
{
    logInfoP("Mode %s", ClimateModeSelectionHelper::toString(_mode));
}

bool ClimateDevice::supportMode(ClimateModeSelection mode)
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
        default:
            return false;
    }
}