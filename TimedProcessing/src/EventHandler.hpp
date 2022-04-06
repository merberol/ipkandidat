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
		if (elevation >= 1000 && gearDeployment > 0.0) {
			worker.HighAlt();
		}
		if (elevation < 500 && gearDeployment < 1.0) {
			worker.LowAlt();
		}
	}

	bool DoStallingEvent() {
		return true;
	}

	void StallingEvent(int stallWarning) {
		if (stallWarning == 1) {
			worker.Stalling();
		}
	}

	bool DoXOutEvents() {
		return true;
	}

	void RedoutEvent(int redout) {
		if (redout == 1) {
			worker.Redout();
		}
	}

	void BlackoutEvent(int blackout) {
		if (blackout == 1) {
			worker.Blackout();
		}
	}

private:
	Worker worker{};

};

