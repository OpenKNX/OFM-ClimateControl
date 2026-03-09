#pragma once
#include "knxprod.h"
#ifndef ParamFCB_CHFormatStringStr
#error "OpenKNXproducer 3.12.8.0 or higher is required to compile this project. Please update your OpenKNXproducer installation."
#endif
#include "OpenKNX.h"
#include "ChannelOwnerModule.h"

class ClimateModeSelection
{
  private:
    ClimateModeSelection() {}
  public: 
    const static uint8_t Auto = 0;
    const static uint8_t Heating = 1;
    const static uint8_t Cooling = 3;
    const static uint8_t Off = 6;
    const static uint8_t Fan = 9;
    const static uint8_t Dehumification = 14;
};

class ClimateControlModule : public ClimateControlChannelOwnerModule
{
    bool _waitForValidDate = true;
    bool _isSummer = true;
  public:
    ClimateControlModule();
    const std::string name() override;
    void showInformations() override;
    const std::string version() override;
    void setup() override;
    OpenKNX::Channel* createChannel(uint8_t _channelIndex /* this parameter is used in macros, do not rename */) override;
    void showHelp() override;
    bool processCommand(const std::string cmd, bool diagnoseKo) override;
    void handleWinterSummerMode(OpenKNX::DateTime ocalTime);
    void setIsSummer(bool isSummer);
    void processInputKo(GroupObject &ko) override;
    
};

extern ClimateControlModule openknxClimateControlModule;