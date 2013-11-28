#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"
#include "HistogrammingUserActions.hh"

#include <G4RunManager.hh>
#include <G4PhysListFactory.hh>
#include <G4NistManager.hh>

int main(int argc, char * argv[]) {
	// print the table of materials
	G4cout << *(G4Material::GetMaterialTable()) << G4endl;

	for(int i=0; i<argc; i++) {
		std::cout << i << ": " << argv[i] << std::endl;
	}

	// construct the default run manager
	G4RunManager* runManager = new G4RunManager;

	// set mandatory initialization classes
	runManager->SetUserInitialization(new DetectorConstruction(10*km));

	G4PhysListFactory factory;
	factory.SetVerbose(0);
	G4VUserPhysicsList * physicslist = factory.GetReferencePhysList("QGSP_BERT");
	runManager->SetUserInitialization(physicslist);

	// set user actions
	runManager->SetUserAction(new PrimaryGeneratorAction(2212, 10*GeV));

	std::ofstream eventfile("events.txt");
	Histogrammer hists(eventfile);
	runManager->SetUserAction(hists.getUserEventAction());
	runManager->SetUserAction(hists.getUserSteppingAction());

	// initialize G4 kernel
	runManager->Initialize();

	// start a run
	runManager->BeamOn(3);

	// store histograms
	hists.saveHistograms();

	// job termination
	delete runManager;
	return 0;
}
