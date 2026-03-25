#pragma once
#include "OpenKNX.h"
#include "ClimateModeSelection.h"

class RoomChannel;

class ClimateDevice
{
    private:
        int _channelIndex;
        int _deviceIndex;
        RoomChannel& _roomChannel;
        std::string _name;
        unsigned long _turnOnTimer = 0;
        volatile bool _inReceiveTempFeedbackKo = false;
        bool _waitForSettingTargetTemperature = false;
        uint16_t _targetTemperatureRawKnx = std::numeric_limits<uint16_t>::max();
        ClimateModeSelection _mode = ClimateModeSelection::Undefined;
        const std::string& logPrefix();
    public:
        ClimateDevice(int channelIndex, int deviceIndex, RoomChannel& roomChannel);
        int deviceNumber() const;
        void loop();
        void setMode(ClimateModeSelection mode);
        void setTargetTemperature(uint16_t targetTemperatureRawKnx);
        void logStatus();
        void processInputKo(GroupObject &ko);
        ClimateModeSelection currentMode();
};

