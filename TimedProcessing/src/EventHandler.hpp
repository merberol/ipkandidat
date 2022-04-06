#pragma once

#include "src/worker.hpp"


struct EventHandler
{

	EventHandler() {
		worker.SayHello();
	};

	bool DoGearEvent() {
		return true;
	}

	void GearEvent(double speed, int elevation, float gearDeployment) {

		worker.HighAlt();
	}
private:
	Worker worker{};

};

