#include "ClimateDevice.h"
#include "ClimateControlModule.h"
#include "RoomChannel.h"

#define DeviceKoOffset (CLI_KoCDev2Power - CLI_KoCDev1Power)
#undef CLI_KoCalcNumber
#define CLI_KoCalcNumber(index) (index + CLI_KoBlockOffset + _channelIndex * CLI_KoBlockSize + _deviceIndex * DeviceKoOffset)

#define KoCLI_CHAVCOut KoCLI_CKo8 
#define CLI_KoCHVACOut CLI_KoCKo8

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

void ClimateDevice::setMode(ClimateModeSelection mode)
{
    logInfoP("Mode: %s", ClimateModeSelectionHelper::toString(mode));
    _mode = mode;
    switch (_deviceIndex == 1 ? ParamCLI_CHControlMode1 : ParamCLI_CHControlMode2)
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

void ClimateDevice::logStatus()
{
    logInfoP("Mode %s", ClimateModeSelectionHelper::toString(_mode));
}
