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
#define PC_VIS   1003

// Program's arguments - an array of option specifiers
// name, short name, arg. name, flags, doc, group
const argp_option argp_options[] = {
	{0, 0, 0, 0, "General options:", 0},
	{"verbosity", 'v', "LEVEL", 0, "set the verbosity level (0 - minimal,"
		" 1 - a bit (default), 2 - a lot)", 0},
	{"prefix", 'o', "PREFIX", 0, "set the prefix of the output files", 0},
	{"vis", PC_VIS, 0, 0, "open the GUI instead of running the simulation", 0},

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
G4String p_prefix = "fgamma";
bool p_vis  = false; // go to visual mode (i.e. open the GUI instead)
int p_verbosity = 1;

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
			p_verbosity = std::atoi(arg);
			break;
		case PC_VIS:
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

	// the positional arguments have to be handled manually though
	if(argc-argp_index != 2) {
		G4cerr << "ERROR: Wrong number of arguments - got " << argc-argp_index << ", expected 2!" << G4endl;
		G4cerr << "  argp_index = " << argp_index << ", argc = " << argc << G4endl;
		for(int i=0; i<argc; i++) {
			G4cerr << "  argv[" << i << "] = `" << argv[i] << "`" << G4endl;
		}
		exit(1);
	}
	G4double energy = atof(argv[argp_index])*GeV;
	G4double incidence = atof(argv[argp_index+1]);
	if(incidence < 0 || incidence > 1) {
		G4cerr << "ERROR: Incidence has to be between 0 and 1 (got " << incidence << " from `" << argv[argp_index+1] << "`)" << G4endl;
		exit(-1);
	}

	G4cout << "% fgamma" << G4endl;

	// verbosity of Geant4 classes -- let's make sure they dont spam if p_verbosity == 1
	int geant_verbosity = p_verbosity==0 ? 0 : p_verbosity-1;
	G4cout << "verbosity: " << p_verbosity << G4endl;
	G4cout << "Geant4 verbosity: " << geant_verbosity << G4endl;

	// print the important simulation parameters (prefixed by a % for easy grepping)
	G4cout << "% energy " << energy/MeV << " MeV" << G4endl;
	G4cout << "% incidence " << incidence << G4endl;
	G4cout << "% modelfile " << p_modelfile << G4endl;
	G4cout << "% prefix " << p_prefix << G4endl;

	// construct the default run manager
	G4RunManager* runManager = new G4RunManager;
	runManager->SetVerboseLevel(geant_verbosity);

	// set mandatory initialization classes
	DetectorConstruction * userDetectorConstruction = new DetectorConstruction(p_modelfile, p_verbosity);
	runManager->SetUserInitialization(userDetectorConstruction);

	// load the physics list
	G4PhysListFactory factory;
	factory.SetVerbose(geant_verbosity);
	G4VUserPhysicsList * physicslist = factory.GetReferencePhysList("QGSP_BERT");
	runManager->SetUserInitialization(physicslist);

	// set user actions
	G4double gunradius = userDetectorConstruction->getWorldRadius();
	runManager->SetUserAction(new PrimaryGeneratorAction(2212, energy, gunradius, incidence*halfpi));
	G4cout << "% gunradius " << gunradius/km << " km" << G4endl;

	UserActionManager uam(p_tracks, p_prefix);
	runManager->SetUserAction(uam.getUserEventAction());
	runManager->SetUserAction(uam.getUserSteppingAction());
	runManager->SetUserAction(uam.getUserStackingAction());
	runManager->SetUserAction(uam.getUserTrackingAction());

	// print the table of materials
	if(p_verbosity>1){G4cout << *(G4Material::GetMaterialTable()) << G4endl;}

	// set the random seed
	p_seed = p_seed==0 ? std::time(0) : p_seed;
	G4cout << "% seed " << p_seed << G4endl;
	CLHEP::HepRandom::setTheSeed(p_seed);

	// initialize G4 kernel
	runManager->Initialize();

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
		G4cout << "% runs " << p_runs << G4endl;
		runManager->BeamOn(p_runs);
	}

	// job termination
	delete runManager;

	G4cout << "% done" << G4endl;
	return 0;
}
