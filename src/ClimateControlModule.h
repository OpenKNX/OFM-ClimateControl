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
    bool _started = false;
    bool _waitForValidDate = false;
    bool _waitForIsWinterValid = false;
    unsigned long _waitForInitialized = 0;
    uint8_t _versionReadFromFlash = 0;
    bool _isWinter = false;
    bool _isWinterFallbackActive = false;
    uint16_t _hourlyTemperaturesRawKnx[24];
    bool _hourlyTemperaturesWithValidTime = false;
    float _currentAverageTemperature = std::numeric_limits<float>::quiet_NaN();
    const char* _calculationMethod = "";
    unsigned long _timeStampIsWinterPossibleForCurrentAverageTemperature = 0;
    unsigned long _timeStampIsSummerPossibleForCurrentAverageTemperature = 0;
    unsigned long _waitForTemperatureResponse = 0;
    int getCurrentHourlyTemperatureIndex();
    void handleWinterSummerMode(OpenKNX::Time::TimeChangedArgs args);
    void initializeIsWinterFromDate();
    void handleAverageTemperatureCalculation(OpenKNX::Time::TimeChangedArgs args);
    void setIsWinter(bool isWinter, const char* diagnosticMessage);
    void processOutsideTemperatureChange(uint16_t outsideTempRawKnx);
    void recalculateDayAverageTemperature();
    void setAverageTemperature(float averageTemp, const char* calculationMethod);
    void start();
  public:
    ClimateControlModule();
    void readFlash(const uint8_t *iBuffer, const uint16_t iSize) override;
    void writeFlash() override;
    uint16_t flashSize() override;
    uint8_t getVersionFromFlash();
    void afterFlashRead(uint8_t dataVersion);
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
    bool isStarted();
    float getTemperatureFromRawKnx(uint16_t rawKnx);
   
 
    
};

extern ClimateControlModule openknxClimateControlModule;