#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h

#include <globals.hh>
#include <G4VUserPrimaryGeneratorAction.hh>

class G4ParticleGun;
class G4Event;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
	public:
		PrimaryGeneratorAction(
			G4int pid, G4double E, // particle and total energy
			G4double altitude, G4double incidence_angle
		);
		~PrimaryGeneratorAction();

		// methods
		void GeneratePrimaries(G4Event*);

	private:
		// data members
		G4ParticleGun * fPGun; //pointer a to G4 service class
};

#endif


