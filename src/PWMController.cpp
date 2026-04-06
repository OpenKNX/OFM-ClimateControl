#include "PWMController.h"
#include <algorithm>

/**
 * Constructor
 */
PWMController::PWMController(uint16_t periodSeconds)
    : _periodSeconds(std::max((uint16_t)1, periodSeconds)),
      _positionValue(0.0f),
      _output(false),
      _periodStartTime(0)
{
}

void PWMController::reset()
{
    _positionValue = 0.0f;
    _output = false;
    _periodStartTime = 0;
}
/**
 * Set position value
 */
void PWMController::setPositionValue(float positionValue)
{
    const float minSwitchTimeSeconds = 3.0f; // Shortest ON or OFF phase
    float onePercentTimeMs = _periodSeconds / 100.0f;
    float minPeriodInPercantage = minSwitchTimeSeconds / onePercentTimeMs;

    // Prevent to short ON of OFF phase
    if (positionValue < 1.f)
    {
        positionValue = 0.f;
    }
    else if (positionValue > 99.f)
    {
        positionValue = 100.0f;
    }
    else if (positionValue < minPeriodInPercantage)
    {
        positionValue = minPeriodInPercantage;
    }
    else if (positionValue > 100.f - minPeriodInPercantage)
    {
        positionValue = 100.f - minPeriodInPercantage;
    }
    _positionValue = std::clamp(positionValue, 0.0f, 100.0f);
}

/**
 * Main loop function - call as frequently as possible
 */
bool PWMController::loop()
{
    const unsigned long periodMs = (unsigned long)_periodSeconds * 1000UL;

    // Elapsed time within current period (unsigned subtraction handles millis() overflow)
    unsigned long elapsed = _periodStartTime > 0 ? millis() - _periodStartTime : 0;

    // Start a new period if this is the first call or the current period has ended
    if (_periodStartTime == 0 || elapsed >= periodMs)
    {
        _periodStartTime = max(1UL, millis());
        elapsed = 0;
    }

    const bool prevOutput = _output;

    // Determine output state
    if (_positionValue <= 0.0f)
    {
        // Always OFF
        _output = false;
    }
    else if (_positionValue >= 100.0f)
    {
        // Always ON
        _output = true;
    }
    else
    {
        // ON-phase at the beginning of each period
        const unsigned long onTimeMs = (unsigned long)((_positionValue / 100.0f) * (float)periodMs);
        _output = (elapsed < onTimeMs);
    }
    return (_output != prevOutput);
}

/**
 * Get current binary PWM output
 */
bool PWMController::getPWMOutput() const
{
    return _output;
}

/**
 * Log current controller state
 */
void PWMController::logStatus(std::string prefix)
{
    logInfo(prefix, "PWM: Period=%us, Position=%.1f%%, Output=%s",
            _periodSeconds, _positionValue, _output ? "ON" : "OFF");
}
