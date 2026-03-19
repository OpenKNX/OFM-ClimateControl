#pragma once
#include "knxprod.h"
#ifndef ParamFCB_CHFormatStringStr
#error "OpenKNXproducer 3.12.8.0 or higher is required to compile this project. Please update your OpenKNXproducer installation."
#endif
#include "OpenKNX.h"
#include "ChannelOwnerModule.h"
#include "ClimateModeSelection.h"


class ClimateControlModule : public ClimateControlChannelOwnerModule
{
    bool _waitForValidDate = true;
    bool _isWinter = false;
    float _hourlyTemperatures[24];
    bool _hourlyTemperaturesWithValidTime = false;
    float _currentAverageTemperature = std::numeric_limits<float>::quiet_NaN();
    const char* _calculationMethod = "";
    unsigned long _waitForIsWinterValid = 0;
    unsigned long _timeStampIsWinterPossibleForCurrentAverageTemperature = 0;
    unsigned long _timeStampIsSummerPossibleForCurrentAverageTemperature = 0;
    unsigned long _waitForTemperatureResponse = 0;
    int getCurrentHourlyTemperatureIndex();
    void handleWinterSummerMode(OpenKNX::DateTime localTime);
    void handleAverageTemperatureCalculation(OpenKNX::Time::TimeChangedArgs args);
    void setIsWinter(bool isWinter, const char* diagnosticMessage);
    void processOutsideTemperatureChange(float outsideTemp);
    void recalculateDayAverageTemperature();
    void setAverageTemperature(float averageTemp, const char* calculationMethod);
  public:
    ClimateControlModule();
    void loop() override;
    const std::string name() override;
    void showInformations() override;
    const std::string version() override;
    void setup() override;
    OpenKNX::Channel* createChannel(uint8_t _channelIndex /* this parameter is used in macros, do not rename */) override;
    void showHelp() override;
    bool processCommand(const std::string cmd, bool diagnoseKo) override;
    void processInputKo(GroupObject &ko) override;
    bool isWinter();
    bool isSummer();
 
    
};

extern ClimateControlModule openknxClimateControlModule;