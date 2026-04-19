#pragma once
#include "OpenKNX.h"
#include "ClimateModeSelection.h"

class RoomChannel;
class PIController;
class PWMController;
class TargetTemperatureManipulationController;

class ClimateDevice
{
    private:
        int _channelIndex;
        int _deviceIndex;
        RoomChannel& _roomChannel;
        std::string _name;
        unsigned long _turnOnTimer = 0;
        volatile bool _inReceiveTempFeedbackKo = false;
        volatile bool _inReceiveModeFeedbackKo = false;
        bool _waitForSettingTargetTemperature = false;
        uint16_t _targetTemperatureRawKnx = std::numeric_limits<uint16_t>::max();
        bool _needSendTargetTemperaturFromDevice = false;
        unsigned long _blockForwardTemperatureFeedbackFromDevice = 0;
        uint16_t _roomTemperatureRawKnx = std::numeric_limits<uint16_t>::max();
        ClimateModeSelection _mode = ClimateModeSelection::Undefined;
        const std::string& logPrefix();
        bool _supportHeating;
        bool _supportCooling;
        bool _supportDehumification;
        bool _supportFan;
        bool _supportAuto;
        bool _isActive = false;
        unsigned long _waitForSingeModeKosTimer = 0;
        PIController* _piController = nullptr;
        PWMController* _pwmController = nullptr;
        TargetTemperatureManipulationController* _targetTemperatureManipulationController = nullptr;
        void setIsActive(bool active);
        void calculateIsActive();
    public:
        ClimateDevice(int channelIndex, int deviceIndex, RoomChannel& roomChannel, bool supportHeating, bool supportCooling, bool supportDehumification, bool supportFan, bool supportAuto);
        bool supportMode(ClimateModeSelection mode);
        int deviceNumber() const;
        void loop();
        void setMode(ClimateModeSelection mode);
        void setTargetTemperature(uint16_t targetTemperatureRawKnx);
        void setRoomTemperature(uint16_t roomTemperatureRawKnx);
        void logStatus();
        void processInputKo(GroupObject &ko);
        uint8_t getRoundingParameter();
        bool isActive();
        ClimateModeSelection currentMode();
};

