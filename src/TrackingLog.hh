#ifndef TrackingLog_h
#define TrackingLog_h

#include <string>
#include <fstream>

class G4Track;
class G4Step;

class TrackingLog
{
	public:
		TrackingLog();
		~TrackingLog();
		void enable(const std::string &filename);

		// logging functions
		void preTracking(const G4Track * track);
		void postTracking(const G4Track * track, bool on_boundary);
		void classification(const G4Track * track);
		void stepping(const G4Step * step);
		void stepSecondary(const G4Track * track, bool removed);

	private:
		bool enabled;
		std::ofstream trf;
};

#endif
