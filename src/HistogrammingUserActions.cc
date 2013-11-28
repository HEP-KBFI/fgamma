#include "HistogrammingUserActions.hh"

#include <G4Gamma.hh>

// ---------------------------------------------------------------------
//                  Geant4 user action classes
// ---------------------------------------------------------------------

class HistUserSteppingAction : public G4UserSteppingAction {
	UserActionsInterface * uai;
	public:
		HistUserSteppingAction(UserActionsInterface * uai) : uai(uai) {}
		void UserSteppingAction(const G4Step * step) {uai->step(step);}
};

class HistUserEventAction : public G4UserEventAction {
	UserActionsInterface * uai;
	public:
		HistUserEventAction(UserActionsInterface * uai) : uai(uai) {}
		void BeginOfEventAction(const G4Event * ev) {uai->event(ev);}
		void EndOfEventAction(const G4Event * ev) {uai->eventEnd(ev);}
};

// ---------------------------------------------------------------------
//                    Histogrammer class
// ---------------------------------------------------------------------
Histogrammer::Histogrammer(std::ostream &evstream) :
	evid(-1), event_stream(evstream)
{
	userSteppingAction = new HistUserSteppingAction(this);
	userEventAction = new HistUserEventAction(this);
}

G4UserSteppingAction * Histogrammer::getUserSteppingAction() {
	return userSteppingAction;
}

G4UserEventAction * Histogrammer::getUserEventAction() {
	return userEventAction;
}

void Histogrammer::event(const G4Event * ev) {
	G4cout << "EVENT: " << ev->GetEventID() << G4endl;
	evid = ev->GetEventID();
}

void Histogrammer::eventEnd(const G4Event*) {
	evid = -1;
}

void Histogrammer::step(const G4Step * step) {
	if(step->GetPostStepPoint()->GetStepStatus() != fWorldBoundary ||
	   step->GetTrack()->GetParticleDefinition() != G4Gamma::Definition()) {
		return;
	}

	G4double E = step->GetTrack()->GetTotalEnergy()/MeV;
	G4double theta = step->GetPostStepPoint()->GetPosition().theta();
	G4double phi = step->GetPostStepPoint()->GetPosition().phi();
	G4int pid = step->GetTrack()->GetParticleDefinition()->GetPDGEncoding();
	G4String name = step->GetTrack()->GetParticleDefinition()->GetParticleName();

	event_stream << evid << ',' << pid << ',' << name << ','
	             << E << ',' << theta << ',' << phi << G4endl;
}
