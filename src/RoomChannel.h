#pragma once
#include "OpenKNX.h"
#include "ClimateControlModule.h"
#include "ClimateModeSelection.h"
#include "ClimateDevice.h"


class RoomChannel : public OpenKNX::Channel
{
    enum class ChangeSource
    {
        User,
        Internal,
        Device
    };
    enum class PowerState : uint8_t
    {
        On = 0,
        Off = 1,
        Undefined =255 
    };
    const static ClimateModeSelection DefaultMode = ClimateModeSelection::Auto;
    int _channelIndex;
    bool _waitForTargetTemperature = true;
    bool _waitForMode = true;
    bool _waitForPower = true;
    bool _started = false;
    bool _forceSendPower = true;
    bool _forceSendMode = true;
    bool _useCoolingTargetTemperature = false;
    std::string _name;
    bool _forceSendTargetTemperature = false;
    uint16_t _targetTemperatureSetWhileStartingRawKnx = std::numeric_limits<uint16_t>::max();
    uint16_t _targetTemperatureCoolingRawKnx = std::numeric_limits<uint16_t>::max();
    uint16_t _targetTemperatureHeatingRawKnx = std::numeric_limits<uint16_t>::max();
    ClimateModeSelection _currentMode = ClimateModeSelection::Undefined;
    ClimateModeSelection _currentActiveMode = ClimateModeSelection::Undefined;
    PowerState _currentPower= PowerState::Undefined;
    std::vector<ClimateDevice> _climateDevices;

    bool _windowOpen = false;
    bool _waitForGradientRoomTemperatureChange = false;
    unsigned long _windowOpenTimer = 0;
    bool _waitForRoomTemperatureStable = false;
    uint16_t _lastCurrentRoomTemperatureRawKnx = std::numeric_limits<uint16_t>::max();
    ClimateModeSelection _modeLockedWhileOpenWindow = ClimateModeSelection::Undefined;
    uint16_t _targetTemperatureBeforeWindowOpen = std::numeric_limits<uint16_t>::max();
    uint16_t _roomTemperatureBeforeWindowOpen = std::numeric_limits<uint16_t>::max();
    bool _windowOpenAction1Handled = false;
    bool _windowOpenAction2Handled = false;
    bool _windowOpenAction3Handled = false;
    bool _windowOpenAction4Handled = false;
    bool _windowOpenAction5Handled = false;

    void setWindowOpen(bool open);
    void handleWindowOpen();
    void resetWindowOpenActions();
    void handleWindowOpenAction(int actionNumber, uint32_t afterMS, PT_CLIWindowOpenCondition condition, PT_CLIWindowOpenAction action, unsigned long windowOpenSince, uint8_t setPointCorrectionParameter, bool& handled);
    uint16_t limitSetTemperature(bool& forceSend,  const char* tempType, uint16_t currentTargetTemperatureRawKnx, uint16_t targetTemperatureRawKnx, float minTemperature, float maxTemperature, uint8_t roundingParam);

    void handleMode(ClimateModeSelection mode);
    void handleAuto();
    void setMode(ClimateModeSelection mode);
    void setPower(PowerState power);
    void setInitTargetTemperatur();
    uint16_t getTargetTemperatureRawKnx();
    void setTargetTemperatureRawKnx(uint16_t targetTemperatureRawKnx, ChangeSource changeSource);
    float roundTemperature(float temperature, uint8_t roundingParam);
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
    void loop() override;
    void setTargetTemperatureFromDevice(uint16_t targetTemperatureRawKnx, ClimateDevice& device);
    void setModeFromDevice(ClimateModeSelection mode, ClimateDevice& device);
    float getTemperatureFromRawKnx(uint16_t rawKnx);
    uint16_t getRawKnxFromTemperature(float temperature);
 
};