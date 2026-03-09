#pragma once
#include "knxprod.h"
#ifndef ParamFCB_CHFormatStringStr
#error "OpenKNXproducer 3.12.8.0 or higher is required to compile this project. Please update your OpenKNXproducer installation."
#endif
#include "OpenKNX.h"
#include "ChannelOwnerModule.h"

class ClimateControlModule : public ClimateControlChannelOwnerModule
{
  public:
    ClimateControlModule();
    const std::string name() override;
    void showInformations() override;
    const std::string version() override;
    void setup() override;
    OpenKNX::Channel* createChannel(uint8_t _channelIndex /* this parameter is used in macros, do not rename */) override;
    void showHelp() override;
    bool processCommand(const std::string cmd, bool diagnoseKo) override;
};

extern ClimateControlModule openknxClimateControlModule;