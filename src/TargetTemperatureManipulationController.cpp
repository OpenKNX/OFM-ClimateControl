#include "TargetTemperatureManipulationController.h"

void TargetTemperatureManipulationController::setTargetTemperature(float targetTemperature)
{
    if (abs(_targetTemperature - targetTemperature) > 0.05f)
    {
        _targetTemperature = targetTemperature;
        _recalc = true;
    }
}

void TargetTemperatureManipulationController::setCurrentRoomTemperature(float currentRoomTemperature)
{
    if (abs(_currentRoomTemperature - currentRoomTemperature) > 0.05f)
    {
        _currentRoomTemperature = currentRoomTemperature;
        _recalc = true;
    }
}

void TargetTemperatureManipulationController::setCorrectionRoomTemperature(float correctionRoomTemperature)
{
    if (abs(_correctionRoomTemperature - correctionRoomTemperature) > 0.05f)
    {
        _correctionRoomTemperature = correctionRoomTemperature;
        _recalc = true;
    }
}

void TargetTemperatureManipulationController::setMode(ClimateModeSelection mode)
{
    if (_mode != mode)
    {
        _mode = mode;
        _recalc = true;
    }
}

float TargetTemperatureManipulationController::getOffset() const
{
    auto baseOffset = _correctionRoomTemperature - _currentRoomTemperature;

    if (_mode == ClimateModeSelection::Cooling)
    {
        return baseOffset;
    }
    if (_mode == ClimateModeSelection::Heating)
    {
        return -baseOffset;
    }

    return 0.0f;
}

bool TargetTemperatureManipulationController::loop(float& adjustedTargetTemperature)
{
    _recalc = false;
    if (_currentRoomTemperature < -50.0f || _correctionRoomTemperature < -50.0f)
    {
        adjustedTargetTemperature = _targetTemperature;
        return false;
    }
    adjustedTargetTemperature = _targetTemperature + getOffset();
    return true;
}
