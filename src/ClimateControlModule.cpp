#include "ClimateControlModule.h"
#include "RoomChannel.h"
#include "knxprod.h"

ClimateControlModule::ClimateControlModule()
{
    for (int i = 0; i < 24; i++)
    {
        _hourlyTemperatures[i] = std::numeric_limits<float>::quiet_NaN();
    }
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

void ClimateControlModule::writeFlash()
{
    logDebugP("Write data to flash");
    openknx.flash.writeByte(2); // Version
    openknx.flash.writeByte(_hourlyTemperaturesWithValidTime ? 1 : 0);
    openknx.flash.write((uint8_t*)_hourlyTemperatures, sizeof(_hourlyTemperatures));
    openknx.flash.writeWord(RoomChannel::flashSize());
    auto numberOfChannels = getNumberOfChannels();
    openknx.flash.writeByte(numberOfChannels);
    for (uint8_t index = 0; index < numberOfChannels; index++)
    {
        auto channel = (RoomChannel*)getChannel(index);
        openknx.flash.writeByte(channel != nullptr ? 1 : 0);
        if (channel != nullptr)
            channel->writeFlash();
    }
}

uint16_t ClimateControlModule::flashSize()
{
    return 1                             /* Version */
           + 1                           /* hourlyTemperaturesWithValidTime */
           + sizeof(_hourlyTemperatures) /* hourlyTemperatures */
           + 2                           /* channel flash size */
           + 1                           /* numChannels */
           + getNumberOfChannels()       /* channel presence */
           + getNumberOfUsedChannels() * RoomChannel::flashSize();
}

void ClimateControlModule::readFlash(const uint8_t* iBuffer, const uint16_t iSize)
{
    if (iSize != 0)
    {
        _versionReadFromFlash = openknx.flash.readByte(); // Version
        if (_versionReadFromFlash != 2)
        {
            logWarningP("Unknown flash version %d, ignoring flash data", _versionReadFromFlash);
        }
        else
        {
            _hourlyTemperaturesWithValidTime = openknx.flash.readByte() != 0;
            memcpy(_hourlyTemperatures, openknx.flash.read(sizeof(_hourlyTemperatures)), sizeof(_hourlyTemperatures));
            auto channelFlashSize = openknx.flash.readWord();
            auto usedChannels = openknx.flash.readByte();
            for (uint8_t index = 0; index < usedChannels; index++)
            {
                auto channelPresent = openknx.flash.readByte();
                if (channelPresent)
                {
                    auto channel = (RoomChannel*)getChannel(index);
                    if (channel != nullptr)
                    {
                        channel->readFlash(iBuffer, iSize, _versionReadFromFlash);
                    }
                    else
                    {
                        openknx.flash.read(channelFlashSize);
                    }
                }
            }
            logDebugP("Read successfully data from flash");
        }
          
    }
    afterFlashRead(_versionReadFromFlash);
}

void ClimateControlModule::afterFlashRead(uint8_t dataVersion)
{
    for (uint8_t index = 0; index < getNumberOfChannels(); index++)
    {
        auto channel = (RoomChannel*)getChannel(index);
        if (channel != nullptr)
        {
            channel->afterReadFlash(dataVersion);
        }
    }
}

uint8_t ClimateControlModule::getVersionFromFlash()
{
    return _versionReadFromFlash;
}

void ClimateControlModule::setup()
{
    logDebugP("Setup ClimateControlModule");
    _waitForIsWinterValid = ParamCLI_SummerWinterDate || ParamCLI_SummerWinterKo || ParamCLI_SummerWinterDayTemp;
    _waitForValidDate = ParamCLI_SummerWinterDate || (ParamCLI_SummerWinterDayTemp && ParamCLI_AverageTempCalc != PT_CLIAverageTemperatureCalculation::GroupObjectDailyAverage);
    _waitForInitialized = max(1UL, millis());

    ClimateControlChannelOwnerModule::initialize(ParamCLI_VisibleChannels);
    ClimateControlChannelOwnerModule::setup();
    if (ParamCLI_SummerWinterKo)
    {
        if (!KoCLI_Winter.initialized())
            KoCLI_Winter.requestObjectRead();
        else
            processInputKo(KoCLI_Winter);
    }
    if (ParamCLI_SummerWinterDayTemp)
    {
        if (ParamCLI_AverageTempCalc == PT_CLIAverageTemperatureCalculation::GroupObjectDailyAverage)
        {
            if (!KoCLI_DayAverage.initialized())
                KoCLI_DayAverage.requestObjectRead();
        }
        else
        {
            if (!KoCLI_OutsideTemp.initialized())
                KoCLI_OutsideTemp.requestObjectRead();
            else
                processInputKo(KoCLI_OutsideTemp);
        }
    }
}

void ClimateControlModule::start()
{
    _started = true;
    if (_hourlyTemperaturesWithValidTime != openknx.time.isValid())
    {
        // resset restored temperatures
        for (int i = 0; i < 24; i++)
        {
            _hourlyTemperatures[i] = 32767;
        }
        _hourlyTemperaturesWithValidTime = openknx.time.isValid();
        if (ParamCLI_SummerWinterDayTemp && KoCLI_DayAverage.initialized())
        {
            processInputKo(KoCLI_DayAverage);
        }
    }
    if (ParamCLI_SummerWinterDayTemp && ParamCLI_AverageTempCalc != PT_CLIAverageTemperatureCalculation::GroupObjectDailyAverage)
    {
        openknx.time.registerCallback((OpenKNX::Time::TimeChangedEvents)(OpenKNX::Time::TimeChangedEvents::TimeChangedEventValidChanged | OpenKNX::Time::TimeChangedEvents::TimeChangedEventHourChanged), [this](OpenKNX::Time::TimeChangedArgs args) {
            handleAverageTemperatureCalculation(args);
        });
    }
    if (ParamCLI_SummerWinterDate)
    {
        openknx.time.registerCallback((OpenKNX::Time::TimeChangedEvents)(OpenKNX::Time::TimeChangedEvents::TimeChangedEventValidChanged | OpenKNX::Time::TimeChangedEvents::TimeChangedEventHourChanged | OpenKNX::Time::TimeChangedEvents::TimeChangedEventDayChanged), [this](OpenKNX::Time::TimeChangedArgs args) {
            handleWinterSummerMode(args);
        });
    }

    
    recalculateDayAverageTemperature();
    for (uint8_t _channelIndex = 0; _channelIndex < getNumberOfChannels(); _channelIndex++)
    {
        RoomChannel* channel = (RoomChannel*)getChannel(_channelIndex);
        if (channel != nullptr)
        {
            channel->start();
        }
    }
}

void ClimateControlModule::handleWinterSummerMode(OpenKNX::Time::TimeChangedArgs args)
{
    int summerStartMonth = (ParamCLI_SummerTimeStartDay & 0x00FF00) >> 8;
    int summerStartDay = (ParamCLI_SummerTimeStartDay & 0xFF0000) >> 16;
    int winterStartMonth = (ParamCLI_WinterTimeStartDay & 0x00FF00) >> 8;
    int winterStartDay = (ParamCLI_WinterTimeStartDay & 0xFF0000) >> 16;
    auto localTime = args.localTime;
    if (args.isValid)
    {
        logDebugP("Summer: %02d.%02d, Winter: %02d.%02d", summerStartDay, summerStartMonth, winterStartDay, winterStartMonth);
        if ((localTime.month > summerStartMonth && localTime.month < winterStartMonth) ||
            (localTime.month == summerStartMonth && localTime.day >= summerStartDay) ||
            (localTime.month == winterStartMonth && localTime.day < winterStartDay))
        {
            // Summer time
            setIsWinter(false, "date");
        }
        else
        {
            // Winter time
            setIsWinter(true, "date");
        }
    }
    else
    {
        if (localTime.month == summerStartMonth && localTime.day == summerStartDay)
        {
            // Summer time
            setIsWinter(false, "date");
        }
        else if (localTime.month == winterStartMonth && localTime.day == winterStartDay)
        {
            // Winter time
            setIsWinter(true, "date");
        }
    }
}

void ClimateControlModule::setIsWinter(bool isWinter, const char* diagnosticMessage)
{
    if (_isWinter != isWinter || _waitForIsWinterValid != 0)
    {
        _isWinter = isWinter;
        _waitForIsWinterValid = 0;
        logInfoP("Switching to %s mode because of %s", _isWinter ? "winter" : "summer", diagnosticMessage);
        KoCLI_WinterStatus.value(_isWinter, DPT_Switch);
        if (_started)
        {
            for (uint8_t _channelIndex = 0; _channelIndex < getNumberOfChannels(); _channelIndex++)
            {
                RoomChannel* channel = (RoomChannel*)getChannel(_channelIndex);
                if (channel != nullptr)
                {
                    channel->handle();
                }
            }
        }
    }
    else
    {
        logDebugP("Already in %s mode, no change needed for %s", _isWinter ? "winter" : "summer", diagnosticMessage);
    }
    KoCLI_WinterStatus.valueCompare(!_isWinter, DPT_Switch);
   
}

void ClimateControlModule::loop()
{
    bool everythingValid = _waitForIsWinterValid && _waitForValidDate;
    if (_waitForInitialized != 0 && (millis() - _waitForInitialized >= 10000 || everythingValid))
    {
        _waitForInitialized = 0;
        if (everythingValid)
            logDebugP("ClimateControlModule initialized, starting module");
        else
        {
            logDebugP("ClimateControlModule not initialized after 10 seconds, starting module");
            if (_waitForIsWinterValid)
                setIsWinter(true, "Fallback");
        }
        start();
    }
    if (_waitForTemperatureResponse != 0 && millis() - _waitForTemperatureResponse >= 5000)
    {
        logDebugP("No response for outside temperature read request received within 5 seconds, resetting wait for temperature response");
        _waitForTemperatureResponse = 0;
        int index = getCurrentHourlyTemperatureIndex();
        if (index != -1 && KoCLI_OutsideTemp.initialized())
        {
            _hourlyTemperatures[index] = KoCLI_OutsideTemp.value(DPT_Value_2_Count);
            recalculateDayAverageTemperature();
        }
    }
    if (_timeStampIsWinterPossibleForCurrentAverageTemperature != 0 && millis() - _timeStampIsWinterPossibleForCurrentAverageTemperature >= ParamCLI_AverageDaysWinter * 86400000)
    {
        _timeStampIsWinterPossibleForCurrentAverageTemperature = 0;
        setIsWinter(true, "average temperature");
    }
    if (_timeStampIsSummerPossibleForCurrentAverageTemperature != 0 && millis() - _timeStampIsSummerPossibleForCurrentAverageTemperature >= ParamCLI_AverageDaysSummer * 86400000)
    {
        _timeStampIsSummerPossibleForCurrentAverageTemperature = 0;
        setIsWinter(false, "average temperature");
    }
}

void ClimateControlModule::showHelp()
{
    openknx.console.printHelpLine("hvac", "Shows the state of the climate control");
    openknx.console.printHelpLine("hvac<channel>", "Shows the state of the channel");
}

bool ClimateControlModule::processCommand(const std::string cmd, bool diagnoseKo)
{
    if (cmd == "hvac")
    {
        logInfoP("Mode: %s", _isWinter ? "winter" : "summer");
        if (ParamCLI_SummerWinterDayTemp)
        {
            switch (ParamCLI_AverageTempCalc)
            {
                case PT_CLIAverageTemperatureCalculation::EveryHour:
                    logInfoP("Average temperature calculation: Every hour");
                    break;
                case PT_CLIAverageTemperatureCalculation::MannheimHours:
                    logInfoP("Average temperature calculation: Mannheim hours (T7+T14+T21*2)/4");
                    break;
                case PT_CLIAverageTemperatureCalculation::MinMaxAverage:
                    logInfoP("Average temperature calculation: Minimum-Maximum-Mittel");
                    break;
                case PT_CLIAverageTemperatureCalculation::GroupObjectDailyAverage:
                    logInfoP("Average temperature calculation: Tagesmittelwert Temperatur Gruppenobjekt");
                    break;
            }
            if (_currentAverageTemperature != std::numeric_limits<float>::quiet_NaN())
            {
                logInfoP("Current average temperature: %f °C (%s)", _currentAverageTemperature, _calculationMethod);
                for (int i = 0; i < 24; i++)
                {
                    if (_hourlyTemperatures[i] == 32767)
                    {
                        logDebugP("hour %d: -", i);
                    }
                    else
                    {
                        logDebugP("hour %d: %f °C", i, ((float) _hourlyTemperatures[i]) / 100.0f);
                    }
                }
            }
            else
            {
                logInfoP("Current average temperature: not available");
            }
        }
        return true;
    }
    else if (cmd.rfind("hvac", 0) == 0)
    {
        auto channelString = cmd.substr(4);
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
            else
            {
                logInfoP("Channel %d not found", channel);
                return true;
            }
        }
    }
    return false;
}

void ClimateControlModule::processInputKo(GroupObject& ko)
{
    ClimateControlChannelOwnerModule::processInputKo(ko);
    switch (ko.asap())
    {
        case CLI_KoWinter:
        {
            if (ParamCLI_SummerWinterKo)
            {
                bool isWinter = ko.value(DPT_Switch);
                setIsWinter(isWinter, "group object");
            }
            break;
        }
        case CLI_KoOutsideTemp:
            if (_waitForTemperatureResponse != 0)
            {
                logDebugP("Received response for outside temperature read request");
            }
            processOutsideTemperatureChange(ko.value(DPT_Value_Temp));
            break;
    }
}

void ClimateControlModule::handleAverageTemperatureCalculation(OpenKNX::Time::TimeChangedArgs args)
{
    if (args.events & OpenKNX::Time::TimeChangedEvents::TimeChangedEventValidChanged && args.isValid)
    {  
        int index = getCurrentHourlyTemperatureIndex();
        if (index != -1 && KoCLI_OutsideTemp.initialized())
        {
            _hourlyTemperatures[index] = KoCLI_OutsideTemp.value(DPT_Value_2_Count);
        }
    }
    if (args.events & OpenKNX::Time::TimeChangedEvents::TimeChangedEventHourChanged)
    {
        int index = getCurrentHourlyTemperatureIndex();
        if (index != -1)
        {
            if (ParamCLI_SendReadRequest)
            {
                logDebugP("Requesting outside temperature for hourly average temperature calculation");
                _waitForTemperatureResponse = max(1UL, millis());
                KoCLI_OutsideTemp.requestObjectRead();
                return;
            }
            if (KoCLI_OutsideTemp.initialized())
            {

                _hourlyTemperatures[index] = KoCLI_OutsideTemp.value(DPT_Value_2_Count);
            }
        }
    }
    recalculateDayAverageTemperature();
}

void ClimateControlModule::recalculateDayAverageTemperature()
{
    if (!isStarted())
        return;
    logDebugP("Recalculating average temperature");
    bool useEveryHour = ParamCLI_AverageTempCalc == PT_CLIAverageTemperatureCalculation::EveryHour;
    if (ParamCLI_AverageTempCalc == PT_CLIAverageTemperatureCalculation::MannheimHours)
    {
        if (!_hourlyTemperaturesWithValidTime)
        {
            useEveryHour = true;
            logDebugP("Mannheim average temperature calculation not possible because time is not valid, using every hour average instead");
        }
        else
        {
            //  (T7+T14+T21*2)/4
            float t7 = ((float) _hourlyTemperatures[7]) / 100.f;
            float t14 = ((float) _hourlyTemperatures[14]) / 100.f;
            float t21 = ((float) _hourlyTemperatures[21]) / 100.f;
            if (std::isnan(t7) || std::isnan(t14) || std::isnan(t21))
            {
                useEveryHour = true;
                logDebugP("Mannheim average temperature calculation not possible because some hourly temperatures are not valid, using every hour average instead");
            }
            else
            {
                float average = (t7 + t14 + 2 * t21) / 4.0f;
                logDebugP("Calculated Mannheim average temperature: %f", average);
                setAverageTemperature(average, "Mannheim");
            }
        }
    }
    if (useEveryHour)
    {
        float sum = 0;
        int count = 0;
        for (int i = 0; i < 24; i++)
        {
            if (_hourlyTemperatures[i] != 32767)
            {
                sum += ((float) _hourlyTemperatures[i]) / 100.f;
                count++;
            }
        }
        if (count > 0)
        {
            float average = sum / count;
            logDebugP("Calculated average temperature from every hour: %f", average);
            if (ParamCLI_AverageTempCalc == PT_CLIAverageTemperatureCalculation::EveryHour)
                setAverageTemperature(average, "Every Hour");
            else
                setAverageTemperature(average, "Fallback - Every Hour");
        }
        else
        {
            logDebugP("No valid temperatures available to calculate average");
        }
    }
    else if (ParamCLI_AverageTempCalc == PT_CLIAverageTemperatureCalculation::MinMaxAverage)
    {
        uint8_t minTemp = std::numeric_limits<uint8_t>::max();
        uint8_t maxTemp = std::numeric_limits<uint8_t>::lowest();
        for (int i = 0; i < 24; i++)
        {
            if (_hourlyTemperatures[i] != 32767)
            {
                if (_hourlyTemperatures[i] < minTemp)
                    minTemp = _hourlyTemperatures[i];
                if (_hourlyTemperatures[i] > maxTemp)
                    maxTemp = _hourlyTemperatures[i];
            }
        }
        if (minTemp != std::numeric_limits<uint8_t>::max() && maxTemp != std::numeric_limits<uint8_t>::lowest())
        {
            float average = (((float)minTemp) / 100.f + ((float)maxTemp) / 100.f) / 2.0f;
            logDebugP("Calculated MinMax average temperature: %f", average);
            setAverageTemperature(average, "Min/Max");
        }
        else
        {
            logDebugP("No valid temperatures available to calculate MinMax average");
        }
    }
}

void ClimateControlModule::setAverageTemperature(float averageTemp, const char* calulcationMethod)
{
    _calculationMethod = calulcationMethod;
    logDebugP("Setting average temperature to %f calculated by %s", averageTemp, _calculationMethod);
    if (ParamCLI_SummerWinterDayTemp && ParamCLI_AverageTempCalc != PT_CLIAverageTemperatureCalculation::GroupObjectDailyAverage)
    {
        KoCLI_DayAverage.valueCompare(averageTemp, DPT_Value_Temp);
    }
    if (averageTemp != _currentAverageTemperature)
    {
        _currentAverageTemperature = averageTemp;
        if (ParamCLI_SummerWinterDayTemp)
        {
            auto currentMiilis = max(1UL, millis());
            if (_isWinter)
            {
                // Check for swiching to summer mode
                if (_currentAverageTemperature >= ParamCLI_AverageTempSummer)
                {
                    if (_timeStampIsSummerPossibleForCurrentAverageTemperature == 0)
                    {
                        _timeStampIsSummerPossibleForCurrentAverageTemperature = currentMiilis;
                    }
                    logDebugP("Summer temperature (%f) since %lu", _currentAverageTemperature, currentMiilis - _timeStampIsSummerPossibleForCurrentAverageTemperature);
                }
                else
                {
                    _timeStampIsSummerPossibleForCurrentAverageTemperature = 0;
                    logDebugP("No summer temperature (%f)", _currentAverageTemperature);
                }
            }
            else
            {
                // Check for swiching to winter mode
                if (_currentAverageTemperature <= ParamCLI_AverageTempWinter)
                {
                    if (_timeStampIsWinterPossibleForCurrentAverageTemperature == 0)
                    {
                        _timeStampIsWinterPossibleForCurrentAverageTemperature = currentMiilis;
                    }
                    logDebugP("Winter temperature (%f) since %lu", _currentAverageTemperature, currentMiilis - _timeStampIsWinterPossibleForCurrentAverageTemperature);
                }
                else
                {
                    _timeStampIsWinterPossibleForCurrentAverageTemperature = 0;
                    logDebugP("No winter temperature (%f)", _currentAverageTemperature);
                }
            }
        }
    }
}

int ClimateControlModule::getCurrentHourlyTemperatureIndex()
{
    if (ParamCLI_SummerWinterDayTemp)
    {
        switch (ParamCLI_AverageTempCalc)
        {
            case PT_CLIAverageTemperatureCalculation::EveryHour:
                return openknx.time.getUtcTime().hour;
            case PT_CLIAverageTemperatureCalculation::MannheimHours:
                return openknx.time.getLocalTime().hour;
            case PT_CLIAverageTemperatureCalculation::MinMaxAverage:
                return openknx.time.getUtcTime().hour;
            case PT_CLIAverageTemperatureCalculation::GroupObjectDailyAverage:
                return -1; // not supported
        }
    }
    return -1;
}
void ClimateControlModule::processOutsideTemperatureChange(int16_t outsideTemp)
{
    if (ParamCLI_SummerWinterDayTemp && ParamCLI_AverageTempCalc == PT_CLIAverageTemperatureCalculation::GroupObjectDailyAverage)
    {
        setAverageTemperature(((float) outsideTemp) / 100.f, "KO");
        return;
    }
    int index = getCurrentHourlyTemperatureIndex();
    if (index != -1)
    {
        if (_hourlyTemperatures[index] == 32767 ||
            ParamCLI_AverageTempCalc == PT_CLIAverageTemperatureCalculation::MinMaxAverage ||
            _waitForTemperatureResponse != 0)
        {
            _waitForTemperatureResponse = 0;
            _hourlyTemperatures[index] = outsideTemp;
            recalculateDayAverageTemperature();
        }
    }
}

OpenKNX::Channel* ClimateControlModule::createChannel(uint8_t _channelIndex)
{
    if (ParamCLI_CHChannelDisabled)
    {
        logDebugP("Channel %d is disabled", _channelIndex + 1);
        return nullptr;
    }
    logDebugP("Creating channel %d", _channelIndex + 1);
    return new RoomChannel(_channelIndex);
}

bool ClimateControlModule::isWinter()
{
    return _isWinter;
}

bool ClimateControlModule::isSummer()
{
    return !_isWinter;
}

bool ClimateControlModule::isStarted()
{
    return _started;
}

ClimateControlModule openknxClimateControlModule;