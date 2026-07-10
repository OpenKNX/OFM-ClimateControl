#pragma once
#include "ClimateDevice.h"

class RoomChannel;
class PIController;
class PWMController;
class TargetTemperatureManipulationController;

class ClimateDeviceKnxActor : public ClimateDevice
{
    private:
        unsigned long _turnOnTimer = 0;
        volatile bool _inReceiveTempFeedbackKo = false;
        volatile bool _inReceiveModeFeedbackKo = false;
        bool _waitForSettingTargetTemperature = false;
        uint16_t _targetTemperatureRawKnx = std::numeric_limits<uint16_t>::max();
        bool _needSendTargetTemperaturFromDevice = false;
        unsigned long _blockForwardTemperatureFeedbackFromDevice = 0;
        uint16_t _roomTemperatureRawKnx = std::numeric_limits<uint16_t>::max();
        ClimateModeSelection _mode = ClimateModeSelection::Undefined;
        bool _isActive = false;
        unsigned long _waitForSingeModeKosTimer = 0;
        PIController* _piController = nullptr;
        PWMController* _pwmController = nullptr;
        TargetTemperatureManipulationController* _targetTemperatureManipulationController = nullptr;
        void setIsActive(bool active);
        void calculateIsActive();
    public:
        ClimateDeviceKnxActor(int channelIndex, int deviceIndex, RoomChannel& roomChannel);
        int deviceNumber() const override;
        void loop() override;
        void setMode(ClimateModeSelection mode) override;
        void setTargetTemperature(uint16_t targetTemperatureRawKnx) override;
        void setRoomTemperature(uint16_t roomTemperatureRawKnx) override;
        void logStatus() override;
        void processInputKo(GroupObject &ko) override;
        uint8_t getRoundingParameter() override;
        bool isActive() override;
        ClimateModeSelection currentMode() override;
};

