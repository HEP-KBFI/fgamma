#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"
#include "UserActionManager.hh"

#include "globals.hh"
#include <G4RunManager.hh>
#include <G4PhysListFactory.hh>
#include <G4NistManager.hh>

#include <G4UImanager.hh>
#include <G4UIExecutive.hh>
#include <G4VisExecutive.hh>
#include <G4VisExtent.hh>

#include <ctime>

using namespace CLHEP;

// ---------------------------------------------------------------------
// Argument parser settings
// ---------------------------------------------------------------------
// The GNU `argp` is used to conveniently parse command line arguments.
// The documentation: http://www.gnu.org/software/libc/manual/html_node/Argp.html
#include <argp.h>
const char* argp_program_version = "fgamma";

// Codes for options without the short name.
#define PC_SEED  1001
#define PC_TRCKS 1002

// Program's arguments - an array of option specifiers
// name, short name, arg. name, flags, doc, group
const argp_option argp_options[] = {
	{0, 0, 0, 0, "General options:", 0},
	//{"quiet", 'q', 0, 0, "reduce verbosity as much as possible", 0},
	{"prefix", 'o', "PREFIX", 0, "set the prefix of the output files", 0},
	{"vis", 'v', 0, 0, "open the GUI instead of running the simulation", 0},

	{"seed", PC_SEED, "SEED", 0,
		"set the seed for the random generators; if this is not"
		" specified, time(0) is used)", 0},
	{"runs", 'n', "N", 0, "run the simulation N times", 0},
	{"tracks", PC_TRCKS, 0, 0, "store tracks in tracks.txt", 0},

	{0, 0, 0, 0, "Options for tweaking the physics:", 2},
	{"model", 'm', "MODELFILE", 0,
		"set the YAML file used to model the geometry (default: model.yml)", 2},

	{0, 0, 0, 0, "Other:", -1},
	{0, 0, 0, 0, 0, 0} // terminates the array
};

// Parameters
int p_seed = 0;
int p_runs = 1;
G4String p_modelfile = "model.yml";
bool p_tracks = false;
G4String p_prefix = "";
bool p_vis  = false; // go to visual mode (i.e. open the GUI instead)

// Argument parser callback called by argp
error_t argp_parser(int key, char *arg, struct argp_state*) {
	switch(key) {
		case 'o':
			p_prefix = arg;
			break;
		case 'n':
			p_runs = std::atoi(arg);
			break;
		case 'm':
			p_modelfile = G4String(arg);
			break;
		case 'v':
			p_vis = true;
			break;
		case PC_SEED:
			p_seed = std::atoi(arg);
			break;
		case PC_TRCKS:
			p_tracks = true;
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

// argp configuration structure passed to the `argp_parse` function
const argp argp_argp = {
	argp_options,
	&argp_parser,
	"ENERGY(GeV) INCIDENCE(pi/2)",
	"Simulation of gamma-rays produced in the atmosphere by cosmic rays.",
	0, 0, 0
};

int main(int argc, char * argv[]) {
	// parse the arguments
	int argp_index;
	argp_parse(&argp_argp, argc, argv, 0, &argp_index, 0);

	// the positional energy argument must be handled manually
	if(argp_index+2 != argc) {
		G4cerr << "Too many or too few arguments!" << G4endl;
		G4cout << " > argp_index = " << argp_index << G4endl;
		G4cout << " > argc = " << argc << G4endl;
		for(int i=0; i<argc; i++) {
			G4cout << " > argv[" << i << "] = `" << argv[i] << "`" << G4endl;
		}
		exit(-1);
	}
	G4double energy = atof(argv[argp_index])*GeV;
	G4double incidence = atof(argv[argp_index+1]);
	G4cout << "E_0 = " << energy/MeV << " MeV" << G4endl;

	// print the table of materials
	G4cout << *(G4Material::GetMaterialTable()) << G4endl;

	// construct the default run manager
	G4RunManager* runManager = new G4RunManager;

	// set mandatory initialization classes
	DetectorConstruction * userDetectorConstruction = new DetectorConstruction(p_modelfile);
	runManager->SetUserInitialization(userDetectorConstruction);

	G4PhysListFactory factory;
	factory.SetVerbose(0);
	G4VUserPhysicsList * physicslist = factory.GetReferencePhysList("QGSP_BERT");
	runManager->SetUserInitialization(physicslist);

	// set user actions
	G4double gunradius = userDetectorConstruction->getWorldRadius();
	runManager->SetUserAction(new PrimaryGeneratorAction(2212, energy, gunradius, incidence*halfpi));

	UserActionManager uam(p_tracks, p_prefix);
	runManager->SetUserAction(uam.getUserEventAction());
	runManager->SetUserAction(uam.getUserSteppingAction());
	runManager->SetUserAction(uam.getUserStackingAction());
	runManager->SetUserAction(uam.getUserTrackingAction());

	G4cout << *(G4Material::GetMaterialTable()) << G4endl;

	// initialize G4 kernel
	runManager->Initialize();
	CLHEP::HepRandom::setTheSeed(p_seed==0 ? std::time(0) : p_seed);

	// start runs or go into visual mode
	if(p_vis) {
		#ifdef G4VIS_USE
			G4VisManager* visManager = new G4VisExecutive;
			visManager->Initialize();
			G4UIExecutive* ui = new G4UIExecutive(argc, argv, "qt");
			G4UImanager::GetUIpointer()->ApplyCommand("/control/execute vis.mac");
			ui->SessionStart();
			delete ui;
			delete visManager;
		#else
			G4err << "No visualization compiled!" << G4endl;
		#endif
	} else {
		G4cout << "Starting simulation: runs = " << p_runs << G4endl;
		runManager->BeamOn(p_runs);
	}

	// store histograms
	//hists.saveHistograms();

	// job termination
	delete runManager;
	return 0;
}
