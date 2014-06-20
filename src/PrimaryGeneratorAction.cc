#include "PrimaryGeneratorAction.hh"

#include "UserEventInformation.hh"

#include <G4Event.hh>
#include <G4ParticleGun.hh>
#include <G4ParticleTable.hh>
#include <G4ThreeVector.hh>

PrimaryGeneratorAction::PrimaryGeneratorAction(G4double altitude, const std::vector<eventconf> &events_)
: G4VUserPrimaryGeneratorAction(),
  events(events_), eventconf_id(0), event_id(0)
{
	fPGun = new G4ParticleGun(1);
	fPGun->SetParticlePosition(G4ThreeVector(0,0,altitude));
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
  delete fPGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
	if(eventconf_id >= events.size() || event_id >= events[eventconf_id].n) {
		G4cerr << "ERROR: bad event!" << G4endl;
		return;
	}

	const eventconf & ec = events[eventconf_id];
	//G4cout << " > " << eventconf_id << ","<< event_id << ": " << ec << G4endl;

	event_id++;
	if(event_id >= ec.n) {
		eventconf_id++;
		event_id = 0;
	}

	G4ParticleDefinition * pdef = G4ParticleTable::GetParticleTable()->FindParticle(ec.pid);

	UserEventInformation * eventinfo = new UserEventInformation;
	eventinfo->pid = ec.pid;
	eventinfo->E = ec.E;
	eventinfo->KE = ec.E - pdef->GetPDGMass();
	eventinfo->incidence = ec.aoi;

	fPGun->SetParticleDefinition(pdef);
	fPGun->SetParticleEnergy(eventinfo->KE);
	fPGun->SetParticleMomentumDirection(G4ThreeVector(sin(ec.aoi),0,(-1)*cos(ec.aoi)));

	anEvent->SetUserInformation(eventinfo);
	fPGun->GeneratePrimaryVertex(anEvent);
}
