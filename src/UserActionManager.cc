#include "UserActionManager.hh"

#include <G4UserSteppingAction.hh>
#include <G4UserEventAction.hh>
#include <G4UserStackingAction.hh>
#include <G4UserTrackingAction.hh>
#include <G4VUserTrackInformation.hh>

#include <G4Event.hh>
#include <G4Track.hh>
#include <G4Gamma.hh>

using namespace CLHEP;

// ---------------------------------------------------------------------
//                  Geant4 user action classes
// ---------------------------------------------------------------------

class UAIUserSteppingAction : public G4UserSteppingAction {
	UserActionManager::CommonVariables &pUAI;
	public:
		UAIUserSteppingAction(UserActionManager::CommonVariables& uai) : pUAI(uai) {}
		void UserSteppingAction(const G4Step * step);
};

class UAIUserEventAction : public G4UserEventAction {
	UserActionManager::CommonVariables &pUAI;
	public:
		UAIUserEventAction(UserActionManager::CommonVariables& uai) : pUAI(uai) {}
		void BeginOfEventAction(const G4Event * ev);
		void EndOfEventAction(const G4Event * ev);
};

class UAIUserStackingAction : public G4UserStackingAction {
	UserActionManager::CommonVariables &pUAI;
	public:
		UAIUserStackingAction(UserActionManager::CommonVariables& uai) : pUAI(uai) {}
		G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* tr);
};

class UAIUserTrackingAction : public G4UserTrackingAction {
	UserActionManager::CommonVariables &pUAI;
	public:
		UAIUserTrackingAction(UserActionManager::CommonVariables& uai) : pUAI(uai) {}
		void PostUserTrackingAction(const G4Track* tr);
};

// ---------------------------------------------------------------------
//         Implementations of the Geant4 user action classes
// ---------------------------------------------------------------------

class UserTrackInformation : public G4VUserTrackInformation {
	public:
		UserTrackInformation() : G4VUserTrackInformation() {
			//
		}
};

void UAIUserEventAction::BeginOfEventAction(const G4Event * ev) {
	G4cout << "EVENT: " << ev->GetEventID() << G4endl;
	pUAI.evid = ev->GetEventID();
}

void UAIUserEventAction::EndOfEventAction(const G4Event*) {
	pUAI.evid = -1;
}

G4ClassificationOfNewTrack UAIUserStackingAction::ClassifyNewTrack(const G4Track* tr) {
	pUAI.track_stream << pUAI.evid << ","
	                  << tr->GetParentID() << "," << tr->GetTrackID()
	                  << "," << tr->GetParticleDefinition()->GetPDGEncoding()
	                  << "," << tr->GetParticleDefinition()->GetParticleName()
	                  << "," << tr->GetKineticEnergy()/MeV
	                  << "," << tr->GetPosition().mag()/km
	                  << G4endl;
	return tr->GetKineticEnergy() < 20*MeV ? fKill : fUrgent;
	//return fUrgent;
}

void UAIUserSteppingAction::UserSteppingAction(const G4Step * step) {
	//step->GetTrack()->GetParticleDefinition() != G4Gamma::Definition() // check gammas
	if(step->GetPostStepPoint()->GetStepStatus() != fWorldBoundary) {
		return;
	}

	G4int pid = step->GetTrack()->GetParticleDefinition()->GetPDGEncoding();
	const G4String& name = step->GetTrack()->GetParticleDefinition()->GetParticleName();
	const G4double& mass = step->GetTrack()->GetDynamicParticle()->GetMass();

	G4double vertex_KE = step->GetTrack()->GetVertexKineticEnergy();
	const G4ThreeVector& vertex = step->GetTrack()->GetVertexPosition();
	const G4ThreeVector& vertex_pdir = step->GetTrack()->GetVertexMomentumDirection();

	G4double E = step->GetTrack()->GetTotalEnergy();
	const G4ThreeVector& pos = step->GetPostStepPoint()->GetPosition();
	const G4ThreeVector& pdir = step->GetTrack()->GetMomentumDirection();

	pUAI.event_stream << std::setw(5) << pUAI.evid << ',' << std::setw(4) << pid << ',' << std::setw(12) << name
	                  << std::scientific << std::setprecision(10)
	                  << ',' << mass/MeV
	                  << ',' << vertex_KE
	                  << ',' << vertex.x()/km << ',' << vertex.y()/km << ',' << vertex.z()/km
	                  << ',' << vertex_pdir.x() << ',' << vertex_pdir.y() << ',' << vertex_pdir.z()
	                  << ',' << E/MeV
	                  << ',' << pos.x()/km << ',' << pos.y()/km << ',' << pos.z()/km
	                  //<< ',' << pos.theta() << ',' << pos.phi()
	                  << ',' << pdir.x() << ',' << pdir.y() << ',' << pdir.z()
	                  //<< ',' << pdir.theta() << ',' << pdir.phi()
	                  << G4endl;

	// update the histogram
	gsl_histogram_increment(pUAI.hE, E);
}

void UAIUserTrackingAction::PostUserTrackingAction(const G4Track* tr) {
	pUAI.track_stream << " > PostTrack(" << tr->GetTrackID() << "): stepp - " << tr->GetStep() << G4endl;
	if(tr->GetStep()->GetPostStepPoint()->GetStepStatus() != fWorldBoundary) {
		pUAI.track_stream << "   > BOUNDARY!" << G4endl;
	}
}

// ---------------------------------------------------------------------
//                  UserActionManager implementation
// ---------------------------------------------------------------------

UserActionManager::UserActionManager(bool store_tracks, G4String prefix) {
	pUAI.evid = -1;
	pUAI.event_stream.open((prefix+"events.txt").c_str());
	pUAI.track_stream.open(store_tracks ? (prefix+"tracks.txt").c_str() : "/dev/null");
	pUAI.store_tracks = store_tracks;

	userSteppingAction = new UAIUserSteppingAction(pUAI);
	userEventAction = new UAIUserEventAction(pUAI);
	userStackingAction = new UAIUserStackingAction(pUAI);
	userTrackingAction = new UAIUserTrackingAction(pUAI);

	pUAI.hE = gsl_histogram_alloc(10);
	double rngs[] = {0,1e-3,1e-2,1e-1,1e0,1e1,1e2,1e3,1e4,1e5,1e6};
	gsl_histogram_set_ranges(pUAI.hE, rngs, 11);
	for(unsigned int i=0; i<pUAI.hE->n+1; i++) {
		G4cout << i << ' ' << pUAI.hE->range[i] << G4endl;
	}

	// Write the header of the events CSV file
	pUAI.event_stream << "event,pid,name,m,vKE,vx,vy,vz,vpx,vpy,vpz,E,x,y,z,px,py,pz" << G4endl;
	pUAI.track_stream << "event,parentid,trackid,pid,particle,Ekin" << G4endl;
}

UserActionManager::~UserActionManager() {
	pUAI.track_stream.close();
	pUAI.event_stream.close();
	gsl_histogram_free(pUAI.hE);
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

void UserActionManager::saveHistograms() {
	std::ofstream fout("histos.txt");
	fout << pUAI.hE->n << std::endl;
	for(unsigned int i=0; i<pUAI.hE->n+1; i++) {
		fout << (i==0?"":" ") << pUAI.hE->range[i];
	}
	fout << std::endl;
	for(unsigned int i=0; i<pUAI.hE->n; i++) {
		fout << (i==0?"":" ") << pUAI.hE->bin[i];
	}
	fout << std::endl;
	fout.close();
}
