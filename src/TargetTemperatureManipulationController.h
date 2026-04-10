#pragma once

#include "ClimateModeSelection.h"

class TargetTemperatureManipulationController
{

	private:
    	float getOffset() const;
	
        bool _recalc = true; 
		float _targetTemperature = 0.0f;
		float _currentRoomTemperature = -100.0f;
		float _correctionRoomTemperature = -100.0f;
		ClimateModeSelection _mode = ClimateModeSelection::Undefined;
	public:
		void setTargetTemperature(float targetTemperature);
		void setCurrentRoomTemperature(float currentRoomTemperature);
		void setCorrectionRoomTemperature(float correctionRoomTemperature);
		void setMode(ClimateModeSelection mode);
    	bool loop(float& adjustedTargetTemperature);

};
