#pragma once
#include "OpenKNX.h"
#include "ClimateModeSelection.h"

class RoomChannel;

class ClimateDevice
{
    protected:
        const int _channelIndex;
        const int _deviceIndex;
        RoomChannel& _roomChannel;
        const std::string _name;
        virtual const std::string& logPrefix();
    
    public:
        ClimateDevice(int channelIndex, int deviceIndex, RoomChannel& roomChannel);
        virtual ~ClimateDevice() = default;
        virtual int deviceNumber() const;
     
     
        virtual bool supportMode(ClimateModeSelection mode) = 0;
        virtual void loop() = 0;
        virtual void setMode(ClimateModeSelection mode) = 0;
        virtual void setTargetTemperature(uint16_t targetTemperatureRawKnx) = 0;
        virtual void setRoomTemperature(uint16_t roomTemperatureRawKnx) = 0;
        virtual void logStatus() = 0;
        virtual void processInputKo(GroupObject &ko) = 0;
        virtual uint8_t getRoundingParameter() = 0;
        virtual bool isActive() = 0;
        virtual ClimateModeSelection currentMode() = 0;

};
