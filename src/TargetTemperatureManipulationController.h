#pragma once

#include "ClimateModeSelection.h"

class TargetTemperatureManipulationController
{

	private:
    	float getOffset() const;
	
        bool _recalc = true; 
		float _targetTemperature = 0.0f;
		float _currentRoomTemperature = -100.0f;
		float _roomTemperatureFromDevice = -100.0f;
		ClimateModeSelection _operationMode = ClimateModeSelection::Undefined;
	public:
		void setTargetTemperature(float targetTemperature);
		void setCurrentRoomTemperature(float currentRoomTemperature);
		void setRoomTemperatureFromDevice(float roomTemperatureFromDevice);
		void setOperationMode(ClimateModeSelection mode);
    	bool loop(float& adjustedTargetTemperature);

};
