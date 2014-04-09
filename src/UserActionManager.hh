#ifndef UserActionManager_h
#define UserActionManager_h

#include "HDFWriter.hh"
#include <G4String.hh>
#include <fstream>
#include <vector>

class G4UserSteppingAction;
class G4UserEventAction;
class G4UserStackingAction;
class G4UserTrackingAction;
class Timer;

class UserActionManager
{
	public:
		UserActionManager(Timer& timer, bool store_tracks, double cutoff=0.0, G4String prefix = "");
		~UserActionManager();

		G4UserSteppingAction * getUserSteppingAction();
		G4UserEventAction * getUserEventAction();
		G4UserStackingAction * getUserStackingAction();
		G4UserTrackingAction * getUserTrackingAction();

		struct CommonVariables
		{
			G4int evid;
			std::ofstream event_stream;
			std::ofstream track_stream;
			bool store_tracks;
			Timer& timer;
			double cutoff;

			HDFWriter hdf;
			std::vector<HDFWriter::particle> ps;

			CommonVariables(const G4String fname, Timer& timer_) : timer(timer_), hdf(fname) {}
		};

	private:
		G4UserSteppingAction * userSteppingAction;
		G4UserEventAction * userEventAction;
		G4UserStackingAction * userStackingAction;
		G4UserTrackingAction * userTrackingAction;
		CommonVariables pUAI;
};

#endif
