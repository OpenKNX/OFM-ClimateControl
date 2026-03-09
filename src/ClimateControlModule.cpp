#include "ClimateControlModule.h"
#include "ClimateControlChannel.h"
#include "knxprod.h"


ClimateControlModule::ClimateControlModule()
{
}

const std::string ClimateControlModule::name()
{
    return "ClimateControl";
}

void ClimateControlModule::showInformations()
{
}

const std::string ClimateControlModule::version()
{
#ifdef MODULE_ClimateControl_Version
    return MODULE_ClimateControl_Version;
#else
    // hides the module in the version output on the console, because the firmware version is sufficient.
    return "";
#endif
}

void ClimateControlModule::setup()
{
    ClimateControlChannelOwnerModule::setup();
    ClimateControlChannelOwnerModule::initialize(ParamCLI_VisibleChannels);
}

void ClimateControlModule::showHelp()
{
}

bool ClimateControlModule::processCommand(const std::string cmd, bool diagnoseKo)
{
    if (cmd.rfind("hvac", 0) == 0)
    {
        auto channelString = cmd.substr(2);
        if (channelString.length() > 0)
        {
            auto pos = channelString.find_first_of(' ');
            std::string channelNumberString;
            std::string channelCmd;
            if (pos > 0 && pos != std::string::npos)
            {
                channelNumberString = channelString.substr(0, pos);
                channelCmd = channelString.substr(pos + 1);
            }
            else
            {
                channelNumberString = channelString;
                channelCmd = "";
            }
            auto channel = atoi(channelNumberString.c_str());
            if (channel < 1 || channel > getNumberOfChannels())
            {
                logInfoP("Channel %d not found", channel);
                return true;
            } 
            ClimateControlChannel* functionBlock = (ClimateControlChannel*)getChannel(channel - 1);
            if (functionBlock != nullptr)
            {
                if (functionBlock->processCommand(channelCmd, diagnoseKo))
                    return true;
            }
        }
    }
    return false;
}

OpenKNX::Channel* ClimateControlModule::createChannel(uint8_t _channelIndex)
{

    if (ParamCLI_CHChannelDisabled)
    {
        logDebugP("Channel %d is disabled", _channelIndex);
        return nullptr;
    }
    return nullptr;
}

ClimateControlModule openknxClimateControlModule;