#include "PrimaryGeneratorAction.hh"

#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction(
	G4int pid,
	G4double dm_mass,
	G4ThreeVector position,
	G4ThreeVector momentumDirection
) : G4VUserPrimaryGeneratorAction() {
	G4int nofParticles = 1;
	fPGun = new G4ParticleGun(nofParticles);

	double kin_energy_p1 = dm_mass - G4ParticleTable::GetParticleTable()->FindParticle(pid)->GetPDGMass();

	// default particle kinematics
	fPGun->SetParticleDefinition( G4ParticleTable::GetParticleTable()->FindParticle(pid) );
	fPGun->SetParticleEnergy(kin_energy_p1);
	fPGun->SetParticlePosition(position);
	fPGun->SetParticleMomentumDirection(momentumDirection);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
  delete fPGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
	// this function is called at the begining of event
	fPGun->GeneratePrimaryVertex(anEvent);
}
