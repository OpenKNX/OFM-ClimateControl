#include "ClimateDevice.h"
#include "RoomChannel.h"

ClimateDevice::ClimateDevice(int channelIndex, int deviceIndex, RoomChannel& roomChannel) :
    _channelIndex(channelIndex),
    _deviceIndex(deviceIndex),
    _roomChannel(roomChannel),
    _name(openknx.logger.buildPrefix(roomChannel.name(), channelIndex + 1) + "Dev" + std::to_string(deviceIndex + 1))
{
  
}

void ClimateDevice::init(bool supportHeating, bool supportCooling, bool supportDehumification, bool supportFan, bool supportAuto)
{
    _supportHeating = supportHeating;
    _supportCooling = supportCooling;
    _supportDehumification = supportDehumification;
    _supportFan = supportFan;
    _supportAuto = supportAuto;
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
        case ClimateModeSelection::Auto:
            return _supportAuto;
        default:
            return false;
    }
}

int ClimateDevice::deviceNumber() const
{
    return _deviceIndex + 1;
}

const std::string& ClimateDevice::logPrefix()
{
    return _name;
}
