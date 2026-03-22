#pragma once
#include "OpenKNX.h"
#include "ClimateControlModule.h"
#include "ClimateModeSelection.h"
#include "ClimateDevice.h"


class RoomChannel : public OpenKNX::Channel
{
  enum class PowerState : uint8_t
  {
      On = 0,
      Off = 1,
      Undefined =255 
  };
  private:
    const static ClimateModeSelection DefaultMode = ClimateModeSelection::Auto;
    int _channelIndex;
    bool _waitForTargetTemperature = true;
    bool _waitForMode = true;
    bool _waitForPower = true;
    bool _started = false;
    bool _forceSendPower = true;
    bool _forceSendMode = true;
    std::string _name;
    uint16_t _targetTemperatureCoolingRawKnx = std::numeric_limits<uint16_t>::max();
    uint16_t _targetTemperatureHeatingRawKnx = std::numeric_limits<uint16_t>::max();
    ClimateModeSelection _currentMode = ClimateModeSelection::Undefined;
    ClimateModeSelection _currentActiveMode = ClimateModeSelection::Undefined;
    PowerState _currentPower= PowerState::Undefined;
    ClimateDevice _climateDevice1;
    ClimateDevice _climateDevice2;
    void handleMode(ClimateModeSelection mode);
    void handleAuto();
    void setMode(ClimateModeSelection mode);
    void setPower(PowerState power);
  public:
    RoomChannel(int channelIndex);
    const std::string name() override;
    void writeFlash();
    static uint16_t flashSize();
    void readFlash(const uint8_t *iBuffer, const uint16_t iSize, uint8_t version);
    void afterReadFlash(uint8_t version);
  
    bool processCommand(const std::string cmd, bool diagnoseKo);
    void processInputKo(GroupObject &ko) override;
    void setup() override;
    void start();
    void logStatus();
    void handle();
    bool isWaiting();

};