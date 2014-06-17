#include "TrackingLog.hh"

#include <G4VProcess.hh>
#include <G4Track.hh>
#include <G4Step.hh>

using namespace std;
using namespace CLHEP;

TrackingLog::TrackingLog()
: enabled(false)
{}

TrackingLog::~TrackingLog()
{
	trf.close();
}

void TrackingLog::enable(const std::string &filename)
{
	enabled = true;
	trf.open(filename.c_str());
}

void TrackingLog::preTracking(const G4Track * track)
{
	if(!enabled) return;
	trf << "PreTrack " << "[" << track << " " << track->GetTrackID() << "]" << endl;
}

void TrackingLog::postTracking(const G4Track * track, bool on_boundary)
{
	if(!enabled) return;
	trf << "PostTrack"
	    << "[" << track << " " << track->GetTrackID() << "]"
	    << " step=" << track->GetStep()
	    << (on_boundary ? " [BOUNDARY]" : "")
	    << endl;
	trf.flush();
}

void TrackingLog::classification(const G4Track * track)
{
	if(!enabled) return;
	trf << "CLASSIFY: " << track->GetParentID()
	    << "," << track->GetTrackID()
	    << "," << track->GetParticleDefinition()->GetPDGEncoding()
	    << "," << track->GetParticleDefinition()->GetParticleName()
	    << "," << track->GetKineticEnergy()/MeV
	    << "," << track->GetPosition().mag()/km
	    << "," << (track->GetCreatorProcess()==nullptr ? "[NO CREATOR]" : track->GetCreatorProcess()->GetProcessName())
	    << endl;
}

void TrackingLog::stepping(const G4Step * step)
{
	if(!enabled) return;
	G4TrackVector &trv = *const_cast<G4Step*>(step)->GetfSecondary();
	trf << " |-- Step[" << step << "]"
	    << "(" << step->GetTrack()->GetTrackID() << ") "
	    << step->GetTrack()->GetCurrentStepNumber()
	    << " secs=" << trv.size()
	    << " (&trv=" << &trv << ")"
	    << endl;
}

void TrackingLog::stepSecondary(const G4Track * track, bool removed)
{
	if(!enabled) return;
	trf << " |    - " << track->GetTrackID()
	    << "," << track->GetParticleDefinition()->GetPDGEncoding()
	    << "," << track->GetParticleDefinition()->GetParticleName()
	    << "," << track->GetKineticEnergy()/MeV
	    << "," << track->GetPosition().mag()/km
	    << "," << (track->GetCreatorProcess()==nullptr ? "[NO CREATOR]" : track->GetCreatorProcess()->GetProcessName())
	    << (removed ? " [REMOVED]" : "")
	    << endl;
}
