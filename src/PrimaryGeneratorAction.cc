#include "PrimaryGeneratorAction.hh"

#include "UserEventInformation.hh"

#include <G4Event.hh>
#include <G4ParticleGun.hh>
#include <G4ParticleTable.hh>
#include <G4ThreeVector.hh>

PrimaryGeneratorAction::PrimaryGeneratorAction(G4int pid, G4double E, G4double altitude, G4double incidence_angle)
: G4VUserPrimaryGeneratorAction(),
  pid_(pid), E_(E), incidence(incidence_angle)
{
	fPGun = new G4ParticleGun(1);
	fPGun->SetParticlePosition(G4ThreeVector(0,0,altitude));
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
  delete fPGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
	G4ParticleDefinition * pdef = G4ParticleTable::GetParticleTable()->FindParticle(pid_);

	UserEventInformation * eventinfo = new UserEventInformation;
	eventinfo->pid = pid_;
	eventinfo->E = E_;
	eventinfo->KE = E_ - pdef->GetPDGMass();
	eventinfo->incidence = incidence;

	fPGun->SetParticleDefinition(pdef);
	fPGun->SetParticleEnergy(eventinfo->KE);
	fPGun->SetParticleMomentumDirection(G4ThreeVector(sin(incidence),0,(-1)*cos(incidence)));

	anEvent->SetUserInformation(eventinfo);
	fPGun->GeneratePrimaryVertex(anEvent);
}
