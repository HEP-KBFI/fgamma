#ifndef UserActionManager_h
#define UserActionManager_h

#include <fstream>

#include <G4UserSteppingAction.hh>
#include <G4UserEventAction.hh>
#include <G4UserStackingAction.hh>

#include <G4Event.hh>
#include <G4Step.hh>
#include <G4Track.hh>

#include <gsl/gsl_histogram.h>

struct UserActionsInterface {
	virtual void step(const G4Step * step) = 0;
	virtual void event(const G4Event * ev) = 0;
	virtual void eventEnd(const G4Event * ev) = 0;
	virtual G4ClassificationOfNewTrack classifyTrack(const G4Track* tr) = 0;
};

class UserActionManager : private UserActionsInterface {
	G4UserSteppingAction * userSteppingAction;
	G4UserEventAction * userEventAction;
	G4UserStackingAction * userStackingAction;

	G4int evid;
	std::ofstream event_stream, track_stream;
	gsl_histogram * hE;

	public:
		UserActionManager(bool store_tracks);
		~UserActionManager();

		void saveHistograms();

		G4UserSteppingAction * getUserSteppingAction();
		G4UserEventAction * getUserEventAction();
		G4UserStackingAction * getUserStackingAction();

	private:
		void step(const G4Step * step);
		void event(const G4Event * ev);
		void eventEnd(const G4Event * ev);
		G4ClassificationOfNewTrack classifyTrack(const G4Track* tr);
};

#endif