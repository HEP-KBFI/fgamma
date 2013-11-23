#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

class G4ParticleGun;
class G4Event;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
	public:
		PrimaryGeneratorAction(
			G4int pid,
			G4double dm_mass,
			G4ThreeVector position= G4ThreeVector(0,0,0),
			G4ThreeVector momentumDirection = G4ThreeVector(0,0,1)
		);
		~PrimaryGeneratorAction();

		// methods
		void GeneratePrimaries(G4Event*);

	private:
		// data members
		G4ParticleGun * fPGun; //pointer a to G4 service class
};

#endif


