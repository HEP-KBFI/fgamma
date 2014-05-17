#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h

#include "configuration.hh"

#include <globals.hh>
#include <G4VUserPrimaryGeneratorAction.hh>

#include <vector>

class G4ParticleGun;
class G4Event;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
	public:
		PrimaryGeneratorAction(G4double altitude, const std::vector<eventconf> &events);
		~PrimaryGeneratorAction();

		// methods
		void GeneratePrimaries(G4Event*);

	private:
		// data members
		G4ParticleGun * fPGun; //pointer a to G4 service class
		std::vector<eventconf> events;
		size_t eventconf_id, event_id;
};

#endif


