#pragma once
#include "OpenKNX.h"
#include "ClimateControlModule.h"
#include "ClimateModeSelection.h"
#include "ClimateDevice.h"

class RoomChannel : public OpenKNX::Channel
{
  private:
    const static ClimateModeSelection DefaultMode = ClimateModeSelection::Auto;
    int _channelIndex;
    std::string _name;
    float _targetTemperatureCooling = 0.0f;
    ClimateModeSelection _currentMode = ClimateModeSelection::Undefined;
    ClimateModeSelection _currentActiveMode = ClimateModeSelection::Undefined;
    ClimateDevice _climateDevice1;
    ClimateDevice _climateDevice2;
    bool handleMode(ClimateModeSelection mode);
    void handleAuto();
  public:
    RoomChannel(int channelIndex);
    bool processCommand(const std::string cmd, bool diagnoseKo);

    const std::string name() override;

    void processInputKo(GroupObject &ko) override;
    void setup() override;
    void handleModeChange(ClimateModeSelection mode);
    void logStatus();
    void handle();

};