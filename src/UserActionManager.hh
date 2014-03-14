#ifndef UserActionManager_h
#define UserActionManager_h

#include <G4String.hh>
#include <fstream>
#include <gsl/gsl_histogram.h>

class G4UserSteppingAction;
class G4UserEventAction;
class G4UserStackingAction;
class G4UserTrackingAction;

class UserActionManager {
	public:
		UserActionManager(bool store_tracks, G4String prefix = "");
		~UserActionManager();

		void saveHistograms();

		G4UserSteppingAction * getUserSteppingAction();
		G4UserEventAction * getUserEventAction();
		G4UserStackingAction * getUserStackingAction();
		G4UserTrackingAction * getUserTrackingAction();

		struct CommonVariables {
			G4int evid;
			gsl_histogram * hE;
			std::ofstream event_stream;
			std::ofstream track_stream;
		};

	private:
		G4UserSteppingAction * userSteppingAction;
		G4UserEventAction * userEventAction;
		G4UserStackingAction * userStackingAction;
		G4UserTrackingAction * userTrackingAction;
		CommonVariables pUAI;
};

#endif
