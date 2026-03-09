#include "ClimateControlChannel.h"

ClimateControlChannel::ClimateControlChannel(uint8_t channelIndex) : _channelIndex(channelIndex)
{
}


bool ClimateControlChannel::processCommand(const std::string cmd, bool diagnoseKo)
{
    return false; 
}