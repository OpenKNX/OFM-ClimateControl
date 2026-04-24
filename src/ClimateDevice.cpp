#include "ClimateDevice.h"
#include "RoomChannel.h"

ClimateDevice::ClimateDevice(int channelIndex, int deviceIndex, RoomChannel& roomChannel) :
    _channelIndex(channelIndex),
    _deviceIndex(deviceIndex),
    _roomChannel(roomChannel),
    _name(openknx.logger.buildPrefix(roomChannel.name(), channelIndex + 1) + "Dev" + std::to_string(deviceIndex + 1))
{
  
}

int ClimateDevice::deviceNumber() const
{
    return _deviceIndex + 1;
}

const std::string& ClimateDevice::logPrefix()
{
    return _name;
}
