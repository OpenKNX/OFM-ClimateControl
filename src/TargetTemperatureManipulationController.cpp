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

void TargetTemperatureManipulationController::setRoomTemperatureFromDevice(float roomTemperatureFromDevice)
{
    if (abs(_roomTemperatureFromDevice - roomTemperatureFromDevice) > 0.05f)
    {
        _roomTemperatureFromDevice = roomTemperatureFromDevice;
        _recalc = true;
    }
}

void TargetTemperatureManipulationController::setOperationMode(ClimateModeSelection mode)
{
    if (_operationMode != mode)
    {
        _operationMode = mode;
        _recalc = true;
    }
}

float TargetTemperatureManipulationController::getOffset() const
{
    auto baseOffset = _roomTemperatureFromDevice - _currentRoomTemperature;

    if (_operationMode == ClimateModeSelection::Cooling)
    {
        return baseOffset;
    }
    if (_operationMode == ClimateModeSelection::Heating)
    {
        return -baseOffset;
    }
    return 0.0f;
}

bool TargetTemperatureManipulationController::loop(float& adjustedTargetTemperature)
{
    bool changed = _recalc;
    _recalc = false;
    if (_currentRoomTemperature < -50.0f || _roomTemperatureFromDevice < -50.0f)
    {
        adjustedTargetTemperature = _targetTemperature;
        return false;
    }
    adjustedTargetTemperature = _targetTemperature + getOffset();
    return changed;
}

void TargetTemperatureManipulationController::logStatus(const std::string& logPrefix)
{
    logInfo(logPrefix.c_str(), "Manipulation: Target=%.2f, Room=%.2f, Room from Devide=%.2f, Offset=%.2f, Mode=%s", _targetTemperature, _currentRoomTemperature, _roomTemperatureFromDevice, getOffset(), ClimateModeSelectionHelper::toString(_operationMode));
}   
