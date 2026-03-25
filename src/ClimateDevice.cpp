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
#define CLI_KoCHVACOut CLI_KoCKo8

#define KoCLI_CHAVCOutFeedback KoCLI_CKo9 
#define CLI_KoCHVACOutFeedback CLI_KoCKo9

#define KoCLI_CHPowerOut KoCLI_CKo10 
#define CLI_KoCHPowerOut CLI_KoCKo10

#define KoCLI_CHCoolingOut KoCLI_CKo8 
#define CLI_KoCHCoolingOut CLI_KoCKo8

#define KoCLI_CHHeatinOut KoCLI_CKo10
#define CLI_KoCHHeatingOut CLI_KoCKo10

#define KoCLI_CHDehumificationOut KoCLI_CKo12
#define CLI_KoCHDehumificationOut CLI_KoCKo12

#define KoCLI_CHFanOut KoCLI_CKo14
#define CLI_KoCHFanOut CLI_KoCKo14

ClimateDevice::ClimateDevice(int channelIndex, int deviceIndex, RoomChannel& roomChannel) : _channelIndex(channelIndex), _deviceIndex(deviceIndex), _roomChannel(roomChannel)
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

void ClimateDevice::processInputKo(GroupObject &ko)
{
    auto koNr = CLI_KoCalcIndex(ko.asap());
    if (koNr == (_deviceIndex ? CLI_KoCDev2SetFb : CLI_KoCDev1SetFb))
    {
        logInfoP("Device %d received ko %d (%d)", deviceNumber(), (int) ko.asap(), koNr);
        if (ParamCLI_CHControlTemperature1 != PT_CLIControlTemperature::FakeSetTemperature)
        {
            _inReceiveTempFeedbackKo = true;
            _roomChannel.setTargetTemperatureRawKnxFeedback(ko.value(DPT_Value_2_Ucount), *this);
            _inReceiveTempFeedbackKo = false;
        }
        else
        {
            logWarningP("Ignore target temperature feedback %0.1f °C from device %d because in temperature fake mode", _roomChannel.getTemperatureFromRawKnx(ko.value(DPT_Value_2_Ucount)), deviceNumber());
        }
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
            KoCLI_CDev1Set.valueNoSend(targetTemperatureRawKnx, DPT_Value_2_Ucount);
            if (!_inReceiveTempFeedbackKo)
                KoCLI_CDev1Set.objectWritten();
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
            KoCLI_CDev1Set.value(_targetTemperatureRawKnx, DPT_Value_2_Ucount);
        }
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
                KoCLI_CHAVCOut.value((uint8_t) mode, DPT_DecimalFactor);
                break;
            case PT_CLIControlMode::HVAC_AND_POWER:
                KoCLI_CHAVCOut.value((uint8_t) mode, DPT_DecimalFactor);
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

void ClimateDevice::logStatus()
{
    logInfoP("Mode %s", ClimateModeSelectionHelper::toString(_mode));
}
