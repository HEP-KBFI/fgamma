#ifndef HistogrammingUserActions_h
#define HistogrammingUserActions_h

#include <fstream>

#include <G4UserSteppingAction.hh>
#include <G4UserEventAction.hh>

#include <G4Event.hh>
#include <G4Step.hh>

#include <gsl/gsl_histogram.h>

struct UserActionsInterface {
	virtual void step(const G4Step * step) = 0;
	virtual void event(const G4Event * ev) = 0;
	virtual void eventEnd(const G4Event * ev) = 0;
};

class Histogrammer : private UserActionsInterface {
	G4UserSteppingAction * userSteppingAction;
	G4UserEventAction * userEventAction;

	G4int evid;

	std::ostream &event_stream;

	gsl_histogram * hE;

	public:
		Histogrammer(std::ostream &evstream);
		~Histogrammer();

		void saveHistograms();

		G4UserSteppingAction * getUserSteppingAction();
		G4UserEventAction * getUserEventAction();

	private:
		void step(const G4Step * step);
		void event(const G4Event * ev);
		void eventEnd(const G4Event * ev);
};

#endif
