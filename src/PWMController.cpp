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

/**
 * Set position value
 */
void PWMController::setPositionValue(float positionValue)
{
    _positionValue = std::clamp(positionValue, 0.0f, 100.0f);
}

/**
 * Main loop function - call as frequently as possible
 */
bool PWMController::loop()
{
    const unsigned long periodMs = (unsigned long)_periodSeconds * 1000UL;

    // Elapsed time within current period (unsigned subtraction handles millis() overflow)
    unsigned long elapsed = millis() - _periodStartTime;

    // Start a new period if this is the first call or the current period has ended
    if (_periodStartTime == 0 || elapsed >= periodMs)
    {
        _periodStartTime = millis();
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
