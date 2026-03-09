#include "OpenKNX.h"
#include "ClimateControlModule.h"

class RoomChannel : public OpenKNX::Channel
{
  private:
    const static uint8_t DefaultMode = ClimateModeSelection::Auto;
    uint8_t _channelIndex;
    uint8_t _currentMode = 255;
    std::string _name;
  public:
    RoomChannel(uint8_t channelIndex);
    bool processCommand(const std::string cmd, bool diagnoseKo);

    const std::string name() override;

    void processInputKo(GroupObject &ko) override;
    void setup() override;
    void handleModeChange(uint8_t mode);
    void handlePowerChange(bool power);

};