#include "ClimateControlModule.h"
#include "RoomChannel.h"
#include "knxprod.h"

ClimateControlModule::ClimateControlModule()
{
    for (int i = 0; i < 24; i++)
    {
        _hourlyTemperaturesRawKnx[i] = std::numeric_limits<uint16_t>::max();
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
    if (_clearFlash)
    {
        logInfoP("Clear flash data");
        return;
    }
    logDebugP("Write data to flash");
    openknx.flash.writeByte(3); // Version
    openknx.flash.writeByte(_hourlyTemperaturesWithValidTime ? 1 : 0);
    openknx.flash.write((uint8_t*)_hourlyTemperaturesRawKnx, sizeof(_hourlyTemperaturesRawKnx));
    openknx.flash.writeByte(_isWinterFallbackActive ? 0 : _isWinter ? 2 : 1);
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
    if (_clearFlash)
        return 0;
    return 1                             /* Version */
           + 1                           /* hourlyTemperaturesWithValidTime */
           + sizeof(_hourlyTemperaturesRawKnx) /* hourlyTemperaturesRawKnx */
           + 1                           /* isWinter */
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
        if (_versionReadFromFlash < 1 || _versionReadFromFlash > 2)
        {
            logWarningP("Unknown flash version %d, ignoring flash data", _versionReadFromFlash);
        }
        else
        {
            _hourlyTemperaturesWithValidTime = openknx.flash.readByte() != 0;
            memcpy(_hourlyTemperaturesRawKnx, openknx.flash.read(sizeof(_hourlyTemperaturesRawKnx)), sizeof(_hourlyTemperaturesRawKnx));
            auto isWinterByte = openknx.flash.readByte();
            if (_waitForIsWinterValid)
            {
                if (isWinterByte == 1)
                    setIsWinter(false, "flash");
                else if (isWinterByte == 2)
                    setIsWinter(true, "flash");
            }
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
            {
                _waitForTemperatureResponse = max(1UL, millis());
                KoCLI_OutsideTemp.requestObjectRead();
            }
            else
                processInputKo(KoCLI_OutsideTemp);
        }
    }
}

void ClimateControlModule::start()
{
    logInfoP("Starting ClimateControlModule");
    _started = true;
    if (_hourlyTemperaturesWithValidTime != openknx.time.isValid())
    {
        // resset restored temperatures
        for (int i = 0; i < 24; i++)
        {
            _hourlyTemperaturesRawKnx[i] = std::numeric_limits<uint16_t>::max();
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
    setIsWinter(_isWinter, nullptr);
    for (uint8_t _channelIndex = 0; _channelIndex < getNumberOfChannels(); _channelIndex++)
    {
        RoomChannel* channel = (RoomChannel*)getChannel(_channelIndex);
        if (channel != nullptr)
        {
            channel->start();
        }
    }
}

void ClimateControlModule::initializeIsWinterFromDate()
{
    int summerStartMonth = (ParamCLI_SummerTimeStartDay & 0x00FF00) >> 8;
    int summerStartDay = (ParamCLI_SummerTimeStartDay & 0xFF0000) >> 16;
    int winterStartMonth = (ParamCLI_WinterTimeStartDay & 0x00FF00) >> 8;
    int winterStartDay = (ParamCLI_WinterTimeStartDay & 0xFF0000) >> 16;
    logDebugP("Summer: %02d.%02d, Winter: %02d.%02d", summerStartDay, summerStartMonth, winterStartDay, winterStartMonth);
    auto localTime = openknx.time.getLocalTime();
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

void ClimateControlModule::handleWinterSummerMode(OpenKNX::Time::TimeChangedArgs args)
{
    if (!args.isValid)
        return;
    int summerStartMonth = (ParamCLI_SummerTimeStartDay & 0x00FF00) >> 8;
    int summerStartDay = (ParamCLI_SummerTimeStartDay & 0xFF0000) >> 16;
    int winterStartMonth = (ParamCLI_WinterTimeStartDay & 0x00FF00) >> 8;
    int winterStartDay = (ParamCLI_WinterTimeStartDay & 0xFF0000) >> 16;
    auto localTime = args.localTime;
    if (args.events & OpenKNX::Time::TimeChangedEvents::TimeChangedEventValidChanged && _isWinterFallbackActive)
    {
        initializeIsWinterFromDate();        
    }
    if (args.events & OpenKNX::Time::TimeChangedEvents::TimeChangedEventDayChanged)
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
    _isWinterFallbackActive = false;
    if (_isWinter != isWinter || _waitForIsWinterValid != 0)
    {
        _isWinter = isWinter;
        _waitForIsWinterValid = 0;
        if (diagnosticMessage != nullptr) // Null in case of call from start()
        {
            logInfoP("Switching to %s mode because of %s", _isWinter ? "winter" : "summer", diagnosticMessage);
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
    }
    else
    {
        if (diagnosticMessage != nullptr) // Null in case of call from start()
            logDebugP("Already in %s mode, no change needed for %s", _isWinter ? "winter" : "summer", diagnosticMessage);
    }
    if (_started)
    {
        KoCLI_WinterStatus.valueCompare(_isWinter, DPT_Switch);
        if (_forceSendIsWinter)
        {
            KoCLI_WinterStatus.objectWritten();
            _forceSendIsWinter = false;   
        }
    }
}

void ClimateControlModule::loop()
{
    if (_waitForValidDate != 0 && openknx.time.isValid())
    {
        _waitForValidDate = false;
        logDebugP("Date is valid. Stop waiting for valid date");
        if (_waitForIsWinterValid && !ParamCLI_SummerWinterKo && !ParamCLI_SummerWinterDayTemp)
        {
            initializeIsWinterFromDate();
        }
    }
    if (_waitForInitialized != 0)
    {
        // check if module is valid
        bool everythingValid = _waitForIsWinterValid == 0 && _waitForValidDate == 0;
        if (everythingValid)
        {
            // Check if channels are valid
            for (uint8_t _channelIndex = 0; _channelIndex < getNumberOfChannels(); _channelIndex++)
            {
                RoomChannel* channel = (RoomChannel*)getChannel(_channelIndex);
                if (channel != nullptr && channel->isWaiting())
                {
                    everythingValid = false;
                    break;
                }
            }
        }
        if ((millis() - _waitForInitialized >= 10000 || everythingValid))
        {
            _waitForInitialized = 0;
            if (everythingValid)
                logDebugP("ClimateControlModule initialized, starting module");
            else
            {
                logDebugP("ClimateControlModule not fully initialized after 10 seconds, starting module");
                if (_waitForIsWinterValid)
                {
                    if (ParamCLI_SummerWinterDate && openknx.time.isValid())
                    {
                        initializeIsWinterFromDate();
                    }
                    else
                    {
                        setIsWinter(true, "Fallback");
                        _isWinterFallbackActive = true;
                    }
                }
            }
            start();
        }
    }
    if (_waitForTemperatureResponse != 0 && millis() - _waitForTemperatureResponse >= 5000)
    {
        logDebugP("No response for outside temperature read request received within 5 seconds, resetting wait for temperature response");
        _waitForTemperatureResponse = 0;
        int index = getCurrentHourlyTemperatureIndex();
        if (index != -1 && KoCLI_OutsideTemp.initialized())
        {
            _hourlyTemperaturesRawKnx[index] = KoCLI_OutsideTemp.value(DPT_Value_2_Ucount);
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
    _ledFunctionSummerWinterOperation.loop();
}

void ClimateControlModule::showHelp()
{
    openknx.console.printHelpLine("hvac", "Shows the state of the climate control");
    openknx.console.printHelpLine("hvac cl", "Clears the flash data");
    openknx.console.printHelpLine("hvac average", "Shows the average temperature and stored temperatures");
    openknx.console.printHelpLine("hvac<channel>", "Shows the state of the channel");
}

bool ClimateControlModule::processCommand(const std::string cmd, bool diagnoseKo)
{
    if (cmd == "hvac cl")
    {
        _clearFlash = true;
        logInfoP("Clearing flash data and restarting device");
        delay(20);
        openknx.restart();
        return true;
    }
    if (cmd == "hvac average")
    {
        if (_currentAverageTemperature != std::numeric_limits<float>::quiet_NaN())
        {
            logInfoP("Current average temperature: %f °C (%s)", _currentAverageTemperature, _calculationMethod);
            for (int i = 0; i < 24; i++)
            {
                if (_hourlyTemperaturesRawKnx[i] == std::numeric_limits<uint16_t>::max())
                {
                    logInfoP("hour %d: -", i);
                }
                else
                {
                    logInfoP("hour %d: %f °C", i, getTemperatureFromRawKnx(_hourlyTemperaturesRawKnx[i]));
                }
            }
        }
        else
        {
            logInfoP("Current average temperature: not available");
        }
        return true;
    }
    if (cmd == "hvac")
    {
        if (!isStarted())
        {
            logInfoP("Starting...");
            return true;
        }
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
                logInfoP("Current average temperature: %f °C (%s)", _currentAverageTemperature, _calculationMethod);
            else
                logInfoP("Current average temperature: not available");
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
                _forceSendIsWinter = true;
                bool isWinter = ko.value(DPT_Switch);
                setIsWinter(isWinter, "group object");
            }
            break;
        }
        case CLI_KoOutsideTemp:
            if (_waitForTemperatureResponse != 0)
            {
                logDebugP("Received response for outside temperature read request");
                _waitForTemperatureResponse = 0;
            }
            processOutsideTemperatureChange(ko.value(DPT_Value_2_Ucount));
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
            _hourlyTemperaturesRawKnx[index] = KoCLI_OutsideTemp.value(DPT_Value_2_Ucount);
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

                _hourlyTemperaturesRawKnx[index] = KoCLI_OutsideTemp.value(DPT_Value_2_Ucount);
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
            float t7 = getTemperatureFromRawKnx(_hourlyTemperaturesRawKnx[7]);
            float t14 = getTemperatureFromRawKnx(_hourlyTemperaturesRawKnx[14]);
            float t21 = getTemperatureFromRawKnx(_hourlyTemperaturesRawKnx[21]);
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
            uint16_t tempRawKnx = _hourlyTemperaturesRawKnx[i];
            if (tempRawKnx != std::numeric_limits<uint16_t>::max())
            {
                sum += getTemperatureFromRawKnx(tempRawKnx);
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
        uint16_t minTemp = std::numeric_limits<uint16_t>::max();
        uint16_t maxTemp = std::numeric_limits<uint16_t>::lowest();
        for (int i = 0; i < 24; i++)
        {
            if (_hourlyTemperaturesRawKnx[i] != std::numeric_limits<uint16_t>::max())
            {
                if (_hourlyTemperaturesRawKnx[i] < minTemp)
                    minTemp = _hourlyTemperaturesRawKnx[i];
                if (_hourlyTemperaturesRawKnx[i] > maxTemp)
                    maxTemp = _hourlyTemperaturesRawKnx[i];
            }
        }
        if (minTemp != std::numeric_limits<uint16_t>::max() && maxTemp != std::numeric_limits<uint16_t>::lowest())
        {
            float average = (getTemperatureFromRawKnx(minTemp) + getTemperatureFromRawKnx(maxTemp)) / 2.0f;
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

void ClimateControlModule::processOutsideTemperatureChange(uint16_t outsideTempRawKnx)
{
    if (ParamCLI_SummerWinterDayTemp && ParamCLI_AverageTempCalc == PT_CLIAverageTemperatureCalculation::GroupObjectDailyAverage)
    {
        setAverageTemperature(getTemperatureFromRawKnx(outsideTempRawKnx), "KO");
        return;
    }
    logDebugP("Outside temperature changed to %f °C (%d)", getTemperatureFromRawKnx(outsideTempRawKnx), (int) outsideTempRawKnx);
    int index = getCurrentHourlyTemperatureIndex();
    if (index != -1)
    {
        if (_hourlyTemperaturesRawKnx[index] == std::numeric_limits<uint16_t>::max() ||
            ParamCLI_AverageTempCalc == PT_CLIAverageTemperatureCalculation::MinMaxAverage ||
            _waitForTemperatureResponse != 0)
        {
            _waitForTemperatureResponse = 0;
            _hourlyTemperaturesRawKnx[index] = outsideTempRawKnx;
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

float ClimateControlModule::getTemperatureFromRawKnx(uint16_t rawKnx)
{
    if (rawKnx == std::numeric_limits<uint16_t>::max())
        return std::numeric_limits<float>::quiet_NaN();
    uint8_t highByte = (rawKnx & 0xFF00) >> 8;
    uint8_t lowByte = rawKnx & 0x00FF;
    uint16_t changedBytes = (lowByte << 8) | highByte;
    return float16FromPayload((uint8_t*) &changedBytes, 0);
}

uint16_t ClimateControlModule::getRawKnxFromTemperature(float temperature)
{
    if (std::isnan(temperature))
        return std::numeric_limits<uint16_t>::max();
    uint8_t payload[2];    
    float16ToPayload(payload, 2, 0, temperature, 0xFFFF);
    uint8_t highByte = payload[1];
    uint8_t lowByte = payload[0];
    return (lowByte << 8) | highByte;
}

ClimateControlModule openknxClimateControlModule;