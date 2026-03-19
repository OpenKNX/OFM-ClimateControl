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
        ClimateModeSelection _mode = ClimateModeSelection::Off;
        const std::string& logPrefix();
    public:
        ClimateDevice(int channelIndex, int deviceIndex, RoomChannel& roomChannel);
       
        void setMode(ClimateModeSelection mode);
        void logStatus();
};

