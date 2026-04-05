#include "PIController.h"
#include <algorithm>
#include <cmath>

/**
 * Constructor with explicit PI parameters
 */
PIController::PIController(float proportionalRange, float integralTime)
    : _proportionalRange(proportionalRange),
      _integralTime(integralTime),
      _currentTemperature(0.0f),
      _targetTemperature(0.0f),
      _integralPart(0.0f),
      _lastError(0.0f),
      _positionValue(0.0f),
      _operationMode(ClimateModeSelection::Off),
      _lastCalculationTime(0),
      _defaultCycleTimeMs(1000)
{
    // Validate parameters on construction
    _proportionalRange = std::max(proportionalRange, 0.1f);
    _integralTime = std::max(integralTime, 0.1f);
}

/**
 * Set current temperature
 */
void PIController::setCurrentTemperature(float currentTemperature)
{
    // Protect against NaN values
    if (std::isfinite(currentTemperature))
    {
        _currentTemperature = currentTemperature;
    }
}

/**
 * Set target temperature
 */
void PIController::setTargetTemperature(float targetTemperature)
{
    // Protect against NaN values
    if (std::isfinite(targetTemperature))
    {
        _targetTemperature = targetTemperature;
    }
}

void PIController::setOperationMode(ClimateModeSelection operationMode)
{
    if (_operationMode != operationMode)
    {
        _operationMode = operationMode;
        // Reset state to avoid integral carry-over when changing direction.
        reset();
    }
}

/**
 * Main loop function - call as frequently as possible
 */
bool PIController::loop()
{
    if (_lastCalculationTime == 0 || millis() - _lastCalculationTime >= _defaultCycleTimeMs)
    {
        calculateControlValue();
        return true;
    }
    return false;
}

/**
 * Calculate PI control value
 * This is where the PI algorithm is executed
 */
void PIController::calculateControlValue()
{
    if (_operationMode != ClimateModeSelection::Heating && _operationMode != ClimateModeSelection::Cooling)
    {
        _positionValue = 0.0f;
        return;
    }
    // Get elapsed time since last call and update timestamp
    float elapsedSeconds = getElapsedTimeSeconds();
    _lastCalculationTime = max(1UL, millis());

    // Calculate current error
    float currentError = (_operationMode == ClimateModeSelection::Heating)
                             ? (_targetTemperature - _currentTemperature)
                             : (_currentTemperature - _targetTemperature);

    // --- Proportional Component ---
    // P = error / Xp * 100
    // This gives 100% output when error equals Xp
    float proportionalComponent = (currentError / _proportionalRange) * 100.0f;

    // --- Integral Component ---
    // I += (error / Tn) * cycleTime
    // Tn is the time to reach the proportional value through integration alone
    if (_integralTime > 0.0f)
    {
        _integralPart += (currentError / _integralTime) * elapsedSeconds;
    }

    // Anti-windup: Limit integral accumulation to prevent excessive overshoot
    // Integral should not accumulate beyond ±100 (in terms of output)
    const float MAX_INTEGRAL = 100.0f;
    const float MIN_INTEGRAL = -100.0f;
    _integralPart = std::clamp(_integralPart, MIN_INTEGRAL, MAX_INTEGRAL);

    // --- Calculate Total Output ---
    float outputBeforeSaturation = proportionalComponent + _integralPart;

    // --- Saturation: Limit output to [0, 100] ---
    _positionValue = std::clamp(outputBeforeSaturation,
                                MIN_POSITION_VALUE,
                                MAX_POSITION_VALUE);

    // Store current error for next iteration
    _lastError = currentError;
}

/**
 * Get elapsed time since last calculation
 * Returns time in seconds, clamped to reasonable cycle time boundaries
 */
float PIController::getElapsedTimeSeconds()
{
    unsigned long currentTime = millis();
    unsigned long elapsedMs = 0;
    if (_lastCalculationTime != 0)
    {
        // Handle millis() overflow (wraps every ~49 days on 32-bit systems)
        if (currentTime >= _lastCalculationTime)
        {
            elapsedMs = currentTime - _lastCalculationTime;
        }
        else
        {
            // Overflow occurred - assume a default cycle time
            elapsedMs = (unsigned long)_defaultCycleTimeMs;
        }
    }

    // Clamp to reasonable boundaries to avoid numerical issues
    if (elapsedMs < (unsigned long)MIN_CYCLE_TIME_MS)
    {
        elapsedMs = (unsigned long)MIN_CYCLE_TIME_MS;
    }
    if (elapsedMs > (unsigned long)MAX_CYCLE_TIME_MS)
    {
        elapsedMs = (unsigned long)MAX_CYCLE_TIME_MS;
    }

    return elapsedMs / 1000.0f;  // Convert milliseconds to seconds
}

/**
 * Get current actuator value
 */
float PIController::getPositionValue() const
{
    return _positionValue;
}

/**
 * Reset controller state
 */
void PIController::reset()
{
    _integralPart = 0.0f;
    _lastError = 0.0f;
    _positionValue = 0.0f;
    _lastCalculationTime = 0;
}

void PIController::logStatus(std::string prefix)
{
    const char* modeText = ClimateModeSelectionHelper::toString(_operationMode);
    float currentError = (_operationMode == ClimateModeSelection::Heating)
                             ? (_targetTemperature - _currentTemperature)
                             : (_currentTemperature - _targetTemperature);

    logInfo(prefix, "PI(%s): Current=%.2f°C, Target=%.2f°C, Output=%.1f%%, P=%.1f%%, I=%.1f%%",
             modeText,
             _currentTemperature, _targetTemperature, _positionValue,
             currentError / _proportionalRange * 100.0f,
             _integralPart);
}