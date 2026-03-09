#include "OpenKNX.h"

class ClimateControlChannel : public OpenKNX::Channel
{
    uint8_t _channelIndex;
  public:
    ClimateControlChannel(uint8_t channelIndex);
    bool processCommand(const std::string cmd, bool diagnoseKo);
};