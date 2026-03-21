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
    int16_t _targetTemperatureCooling = 32767;
    int16_t _targetTemperatureHeating = 32767;
    ClimateModeSelection _currentMode = ClimateModeSelection::Undefined;
    ClimateModeSelection _currentActiveMode = ClimateModeSelection::Undefined;
    ClimateDevice _climateDevice1;
    ClimateDevice _climateDevice2;
    bool handleMode(ClimateModeSelection mode);
    void handleAuto();
  public:
    RoomChannel(int channelIndex);
    const std::string name() override;
    void writeFlash();
    static uint16_t flashSize();
    void readFlash(const uint8_t *iBuffer, const uint16_t iSize, uint8_t version);
    void afterReadFlash(uint8_t version);
  
    bool processCommand(const std::string cmd, bool diagnoseKo);
    void processInputKo(GroupObject &ko) override;
    void start();
    void handleModeChange(ClimateModeSelection mode);
    void logStatus();
    void handle();

};