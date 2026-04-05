#pragma once
#include "OpenKNX.h"

/**
 * @class PWMController
 * @brief Pulse Width Modulation controller for binary actuators
 *
 * Converts a continuous position value (0-100%) into a time-proportional
 * binary ON/OFF signal based on a configurable period duration.
 *
 * PWM Principle:
 * - Period is split into ON-time and OFF-time based on positionValue
 * - ON-time = (positionValue / 100) * periodSeconds
 * - OFF-time = periodSeconds - ON-time
 * - Each period starts with the ON-phase
 *
 * Examples (period = 10s):
 *   positionValue =   0% → always OFF
 *   positionValue =  25% → ON 2.5s, OFF 7.5s
 *   positionValue =  50% → ON 5s,   OFF 5s
 *   positionValue = 100% → always ON
 */
class PWMController
{

private:
    // Configuration
    uint16_t _periodSeconds;       // Period duration [s], minimum 1
    float    _positionValue;       // Setpoint [0.0..100.0] %

    // State
    bool          _output;          // Current binary output
    unsigned long _periodStartTime; // millis() timestamp of period start

public:
    /**
     * @brief Constructor
     * @param periodSeconds Period duration in seconds (minimum 1)
     */
    PWMController(uint16_t periodSeconds);

    void reset();

    /**
     * @brief Set the position/setpoint value
     * @param positionValue Desired output level [0.0..100.0] %
     *        Values > 100.0 are clamped to 100.0.
     */
    void setPositionValue(float positionValue);

    /**
     * @brief Main control loop — call as frequently as possible
     * Evaluates the current position within the PWM period and
     * updates the binary output accordingly.
     * @return true if the output state changed in this call
     */
    bool loop();

    /**
     * @brief Get current binary PWM output
     * @return true = ON, false = OFF
     */
    bool getPWMOutput() const;

    /**
     * @brief Log current controller state to the OpenKNX console
     * @param prefix Log message prefix
     */
    void logStatus(std::string prefix);
};
