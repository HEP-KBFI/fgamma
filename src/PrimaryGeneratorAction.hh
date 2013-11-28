#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h

#include <globals.hh>
#include <G4VUserPrimaryGeneratorAction.hh>

class G4ParticleGun;
class G4Event;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
	public:
		PrimaryGeneratorAction(
			G4int pid,
			G4double dm_mass
		);
		~PrimaryGeneratorAction();

		// methods
		void GeneratePrimaries(G4Event*);

	private:
		// data members
		G4ParticleGun * fPGun; //pointer a to G4 service class
};

#endif


