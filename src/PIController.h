#pragma once
#include "OpenKNX.h"
/**
 * @class HeatingController
 * @brief PI (Proportional-Integral) heating controller for HVAC systems
 * 
 * This class implements a professional-grade PI controller for heating systems.
 * It calculates an actuator value (0-100%) to control heating based on the
 * difference between target and current temperature.
 * 
 * PI-Controller Theory:
 * - Proportional (P): Fast response to temperature deviation
 *   Output_P = error / Xp * 100
 * - Integral (I): Removes steady-state error over time
 *   Output_I = accumulated(error / Tn * cycleTime)
 * - Final Output = Output_P + Output_I, saturated to [0, 100]
 * 
 * Key Parameters:
 * - Xp (Proportional Range) [K]: Temperature range for 100% actuator output
 *   - Smaller Xp → more aggressive control, higher risk of oscillation
 *   - Typical range: 1.0 - 6.0 K
 * - Tn (Integral Time) [s]: Time for integral part to reach P-value
 *   - Smaller Tn → faster integral accumulation
 *   - Typical range: 60 - 600 s
 */

class PIController
{
 

private:
    // PI Controller Parameters
    float _proportionalRange;  // Xp [K] - temperature range for 100% output
    float _integralTime;       // Tn [s] - integral time constant

    // Temperature values
    float _currentTemperature;  // Current room/actual temperature [°C]
    float _targetTemperature;   // Target/setpoint temperature [°C]

    // PI Algorithm State
    float _integralPart;        // Accumulated integral value
    float _lastError;           // Previous error for zero-crossings
    float _positionValue;       // Output value [0..100]

    // Timing
    unsigned long _lastCalculationTime;  // Timestamp of last loop() call
    unsigned long _defaultCycleTimeMs;           // Default cycle time [ms]
    const unsigned long MIN_CYCLE_TIME_MS = 50;   // Minimum safe cycle time
    const unsigned long MAX_CYCLE_TIME_MS = 10000; // Maximum reasonable cycle time

    // Limits
    const float MIN_POSITION_VALUE = 0.0f;
    const float MAX_POSITION_VALUE = 100.0f;

    /**
     * Internal function: Calculate PI control value
     * This is called by loop() based on elapsed cycle time
     */
    void calculateControlValue();

    /**
     * Helper: Get elapsed time since last calculation in seconds
     * Clamps cycle time between MIN and MAX boundaries
     */
    float getElapsedTimeSeconds();

public:
    /**
     * @brief Constructor with explicit PI parameters
     * @param proportionalRange Xp [K] - Recommended: 1.5 - 6.0
     * @param integralTime Tn [s] - Recommended: 60 - 600
     */
    PIController(float proportionalRange, float integralTime);


    /**
     * @brief Set current (actual) temperature
     * @param currentTemperature Current room temperature [°C]
     */
    void setCurrentTemperature(float currentTemperature);

    /**
     * @brief Set target (setpoint) temperature
     * @param targetTemperature Target temperature [°C]
     */
    void setTargetTemperature(float targetTemperature);


    /**
     * @brief Main control loop - calculates actuator value
     * Call this function as frequently as possible (ideally every 50-100ms).
     * Internally uses millis() to determine actual cycle time.
     * Safe to call at variable rates; timing is self-adjusting.
     */
    bool loop();

    /**
     * @brief Get current position output value
     * @return Output in range [0.0 ... 100.0] representing %
     */
    float getPositionValue() const;

    /**
     * @brief Reset controller state
     * Clears integral accumulator and error history.
     * Useful when switching modes or reinitializing the controller.
     */
    void reset();

    /* Log the status to the open knx console */
    void logStatus(std::string prefix);

};
