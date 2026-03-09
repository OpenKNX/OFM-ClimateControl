#include "ClimateControlModule.h"
#include "RoomChannel.h"
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

    setIsSummer(false);
  
    if (ParamCLI_SummerWinterDate)
    {
        openknx.time.registerCallback((OpenKNX::Time::TimeChangedEvents) (OpenKNX::Time::TimeChangedEvents::TimeChangedEventValidChanged | OpenKNX::Time::TimeChangedEvents::TimeChangedEventDayChanged),  [this](OpenKNX::Time::TimeChangedArgs args){
            if (args.isValid)
            {
                handleWinterSummerMode(args.localTime);
            }
        });
    }
    if (ParamCLI_SummerWinterKo)
    {
        if (!KoCLI_Summer.initialized())
            KoCLI_Summer.requestObjectRead();
        else
            processInputKo(KoCLI_Summer);
    }
}

void ClimateControlModule::handleWinterSummerMode(OpenKNX::DateTime localTime)
{
    int summerStartDay = (ParamCLI_SummerTimeStartDay & 0x00FF00) >> 8;
    int summerStartMonth = (ParamCLI_SummerTimeStartDay & 0x0000FF);
    int winterStartDay = (ParamCLI_WinterTimeStartDay & 0x00FF00) >> 8;
    int winterStartMonth = (ParamCLI_WinterTimeStartDay & 0x0000FF);
    if (_waitForValidDate)
    {
        _waitForValidDate = false;
        if ((localTime.month > summerStartMonth && localTime.month < winterStartMonth) 
            || (localTime.month == summerStartMonth && localTime.day >= summerStartDay) 
            || (localTime.month == winterStartMonth && localTime.day < winterStartDay))
        {          
            // Summer time
            setIsSummer(true);
        }
        else
        {   
            // Winter time
            setIsSummer(false);
        }
    }
    else
    {
        if (localTime.month == summerStartMonth && localTime.day == summerStartDay) 
        {
            // Summer time
            setIsSummer(true);
        }
        else if (localTime.month == winterStartMonth && localTime.day == winterStartDay)
        {
            // Winter time
            setIsSummer(false);
        }
       
    }
}

void ClimateControlModule::setIsSummer(bool isSummer)
{
    if (_isSummer != isSummer)
    {
        _isSummer = isSummer;
        logInfoP("Switching to %s mode", _isSummer ? "summer" : "winter");
        KoCLI_SummerStatus.value(_isSummer, DPT_Switch);
    }
    else if (!KoCLI_SummerStatus.initialized())
    {
        KoCLI_SummerStatus.value(_isSummer, DPT_Switch);
    }
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
            RoomChannel* roomChannel = (RoomChannel*)getChannel(channel - 1);
            if (roomChannel != nullptr)
            {
                if (roomChannel->processCommand(channelCmd, diagnoseKo))
                    return true;
            }
        }
    }
    return false;
}

void ClimateControlModule::processInputKo(GroupObject &ko)
{
    ClimateControlChannelOwnerModule::processInputKo(ko);
    switch (ko.asap())
    {
        case CLI_KoSummer: {
            _waitForValidDate = false;
            bool isSummer = ko.value(DPT_Switch);
            logInfoP("Received summer/winter mode change via KNX, switching to %s mode", isSummer ? "summer" : "winter");
            setIsSummer(isSummer);
            break;
        }
    }
}

OpenKNX::Channel* ClimateControlModule::createChannel(uint8_t _channelIndex)
{

    if (ParamCLI_CHChannelDisabled)
    {
        logDebugP("Channel %d is disabled", _channelIndex);
        return nullptr;
    }
    return new RoomChannel(_channelIndex);
}

ClimateControlModule openknxClimateControlModule;