#include "UserActionManager.hh"

#include <G4Gamma.hh>

using namespace CLHEP;

// ---------------------------------------------------------------------
//                  Geant4 user action classes
// ---------------------------------------------------------------------

class UAIUserSteppingAction : public G4UserSteppingAction {
	UserActionsInterface * uai;
	public:
		UAIUserSteppingAction(UserActionsInterface * uai) : uai(uai) {}
		void UserSteppingAction(const G4Step * step) {uai->step(step);}
};

class UAIUserEventAction : public G4UserEventAction {
	UserActionsInterface * uai;
	public:
		UAIUserEventAction(UserActionsInterface * uai) : uai(uai) {}
		void BeginOfEventAction(const G4Event * ev) {uai->event(ev);}
		void EndOfEventAction(const G4Event * ev) {uai->eventEnd(ev);}
};

class UAIUserStackingAction : public G4UserStackingAction {
	UserActionsInterface * uai;
	public:
		UAIUserStackingAction(UserActionsInterface * uai) : uai(uai) {}
		G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* tr) {
			return uai->classifyTrack(tr);
		}
};

class UAIUserTrackingAction : public G4UserTrackingAction {
	UserActionsInterface * uai;
	public:
		UAIUserTrackingAction(UserActionsInterface * uai) : uai(uai) {}
		void PostUserTrackingAction(const G4Track* tr) {
			uai->postTracking(tr);
		}
};

// ---------------------------------------------------------------------
//                    Histogrammer class
// ---------------------------------------------------------------------
UserActionManager::UserActionManager(bool store_tracks, G4String prefix) :
	evid(-1), event_stream((prefix+"events.txt").c_str()), track_stream(store_tracks ? (prefix+"tracks.txt").c_str() : "/dev/null")
{
	userSteppingAction = new UAIUserSteppingAction(this);
	userEventAction = new UAIUserEventAction(this);
	userStackingAction = new UAIUserStackingAction(this);
	userTrackingAction = new UAIUserTrackingAction(this);

	hE = gsl_histogram_alloc(10);
	double rngs[] = {0,1e-3,1e-2,1e-1,1e0,1e1,1e2,1e3,1e4,1e5,1e6};
	gsl_histogram_set_ranges(hE, rngs, 11);
	for(unsigned int i=0; i<hE->n+1; i++) {
		G4cout << i << ' ' << hE->range[i] << G4endl;
	}

	// Write the header of the events CSV file
	event_stream << "event,pid,name,E,x,y,z,theta,phi,px,py,pz,p_theta,p_phi" << G4endl;
	track_stream << "event,parentid,trackid,pid,particle,Ekin" << G4endl;
}

UserActionManager::~UserActionManager() {
	track_stream.close();
	event_stream.close();
	gsl_histogram_free(hE);
}

G4UserSteppingAction * UserActionManager::getUserSteppingAction() {
	return userSteppingAction;
}

G4UserEventAction * UserActionManager::getUserEventAction() {
	return userEventAction;
}

G4UserStackingAction * UserActionManager::getUserStackingAction() {
	return userStackingAction;
}

G4UserTrackingAction * UserActionManager::getUserTrackingAction() {
	return userTrackingAction;
}

void UserActionManager::event(const G4Event * ev) {
	G4cout << "EVENT: " << ev->GetEventID() << G4endl;
	evid = ev->GetEventID();
}

void UserActionManager::eventEnd(const G4Event*) {
	evid = -1;
}

G4ClassificationOfNewTrack UserActionManager::classifyTrack(const G4Track* tr) {
	track_stream << evid << ","
	             << tr->GetParentID() << "," << tr->GetTrackID()
	             << "," << tr->GetParticleDefinition()->GetPDGEncoding()
	             << "," << tr->GetParticleDefinition()->GetParticleName()
	             << "," << tr->GetKineticEnergy()/MeV
	             << "," << tr->GetPosition().mag()/km
	             << G4endl;
	return tr->GetKineticEnergy() < 20*MeV ? fKill : fUrgent;
	//return fUrgent;
}

void UserActionManager::step(const G4Step * step) {
	//step->GetTrack()->GetParticleDefinition() != G4Gamma::Definition() // check gammas
	if(step->GetPostStepPoint()->GetStepStatus() != fWorldBoundary) {
		return;
	}

	G4int pid = step->GetTrack()->GetParticleDefinition()->GetPDGEncoding();
	G4String name = step->GetTrack()->GetParticleDefinition()->GetParticleName();

	G4double E = step->GetTrack()->GetTotalEnergy();
	G4ThreeVector pos = step->GetPostStepPoint()->GetPosition();
	G4ThreeVector pdir = step->GetTrack()->GetMomentumDirection();


	event_stream << std::setw(5) << evid << ',' << std::setw(4) << pid << ',' << std::setw(12) << name
	             << std::scientific << std::setprecision(10)
	             << ',' << E/MeV
	             << ',' << pos.x()/km << ',' << pos.y()/km << ',' << pos.z()/km
	             << ',' << pos.theta() << ',' << pos.phi()
	             << ',' << pdir.x() << ',' << pdir.y() << ',' << pdir.z()
	             << ',' << pdir.theta() << ',' << pdir.phi()
	             << G4endl;

	// update the histogram
	gsl_histogram_increment(hE, E);
}

void UserActionManager::postTracking(const G4Track* tr) {
	track_stream << " > PostTrack(" << tr->GetTrackID() << "): stepp - " << tr->GetStep() << G4endl;
	if(tr->GetStep()->GetPostStepPoint()->GetStepStatus() != fWorldBoundary) {
		track_stream << "   > BOUNDARY!" << G4endl;
	}
}

void UserActionManager::saveHistograms() {
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
