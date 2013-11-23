#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"

#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4PhysListFactory.hh"

int main(int argc, char * argv[]) {
	for(int i=0; i<argc; i++) {
		std::cout << i << ": " << argv[i] << std::endl;
	}

	std::ofstream g4log("g4log.log");
	g4log << "Starting log :)))" << std::endl;
	G4cout.rdbuf(g4log.rdbuf()); // send G4cout stuff to files

	// construct the default run manager
	G4RunManager* runManager = new G4RunManager;

	// set mandatory initialization classes
	runManager->SetUserInitialization(new DetectorConstruction);

	G4PhysListFactory factory;
	factory.SetVerbose(0);
	G4VUserPhysicsList * physicslist = factory.GetReferencePhysList("QGSP_BERT");
	runManager->SetUserInitialization(physicslist);

	// set mandatory user action class
	runManager->SetUserAction(new PrimaryGeneratorAction(2212, 1*GeV));

	// initialize G4 kernel
	runManager->Initialize();

	// get the pointer to the UI manager and set verbosities
	G4UImanager* UI = G4UImanager::GetUIpointer();
	UI->ApplyCommand("/run/verbose 1");
	UI->ApplyCommand("/event/verbose 1");
	UI->ApplyCommand("/tracking/verbose 1");

	// start a run
	int numberOfEvent = 3;
	runManager->BeamOn(numberOfEvent);

	// job termination
	delete runManager;
	g4log.close();
	return 0;
}
