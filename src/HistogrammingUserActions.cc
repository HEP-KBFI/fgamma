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

	hE = gsl_histogram_alloc(10);
	double rngs[] = {0,1e-3,1e-2,1e-1,1e0,1e1,1e2,1e3,1e4,1e5,1e6};
	gsl_histogram_set_ranges(hE, rngs, 11);
	for(unsigned int i=0; i<hE->n+1; i++) {
		G4cout << i << ' ' << hE->range[i] << G4endl;
	}
}

Histogrammer::~Histogrammer() {
	gsl_histogram_free(hE);
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

	// update the histogram
	gsl_histogram_increment(hE, E);
}

void Histogrammer::saveHistograms() {
	std::ofstream fout("histos.txt");
	fout << hE->n << std::endl;
	for(unsigned int i=0; i<hE->n+1; i++) {
		fout << (i==0?"":" ") << hE->range[i];
	}
	fout << std::endl;
	for(unsigned int i=0; i<hE->n; i++) {
		fout << (i==0?"":" ") << hE->bin[i];
	}
	fout << std::endl;
	fout.close();
}
