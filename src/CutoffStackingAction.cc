#include "CutoffStackingAction.hh"

#include <G4Track.hh>

CutoffStackingAction::CutoffStackingAction(std::ostream &trackstream) : trackstream(trackstream) {
	// Write the header of the tracks to the CSV file
	trackstream << "parentid,trackid,pid,particle,Ekin" << G4endl;
}

G4ClassificationOfNewTrack CutoffStackingAction::ClassifyNewTrack(const G4Track* tr) {
	trackstream << tr->GetParentID() << "," << tr->GetTrackID()
	            << "," << tr->GetParticleDefinition()->GetPDGEncoding()
	            << "," << tr->GetParticleDefinition()->GetParticleName()
	            << "," << tr->GetKineticEnergy()/MeV
	            << G4endl;
	//return tr->GetKineticEnergy() < 20*MeV ? fKill : fUrgent;
	return fUrgent;
}
