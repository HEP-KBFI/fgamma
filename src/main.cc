#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"
#include "UserActionManager.hh"

#include <G4RunManager.hh>
#include <G4PhysListFactory.hh>
#include <G4NistManager.hh>

#include <ctime>

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
	//{"ofile", 'o', "OFILE", 0, "write to OFILE; default is ???", 0},

	{"seed", PC_SEED, "SEED", 0,
		"set the seed for the random generators; if this is not"
		" specified, time(0) is used)", 0},
	{"runs", 'n', "N", 0, "run the simulation N times", 0},
	{"tracks", PC_TRCKS, 0, 0, "store tracks in tracks.txt", 0},

	{0, 0, 0, 0, "Options for tweaking the physics:", 2},
	{"radius", 'r', "R", 0,
		"set the radius of the sphere in km (default is 10 km)", 2},

	{0, 0, 0, 0, "Other:", -1},
	{0, 0, 0, 0, 0, 0} // terminates the array
};

// Parameters
int p_seed = 0;
int p_runs = 1;
G4double p_radius = 10.0*km;
bool p_tracks = false;

// Argument parser callback called by argp
error_t argp_parser(int key, char *arg, struct argp_state*) {
	switch(key) {
		case 'n':
			p_runs = std::atoi(arg);
			break;
		case 'r':
			p_radius = std::atof(arg)*m;
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
	"<ENERGY in GeV>",
	"Simulation of gamma-rays produced in the atmosphere by cosmic rays.",
	0, 0, 0
};

int main(int argc, char * argv[]) {
	// parse the arguments
	int argp_index;
	argp_parse(&argp_argp, argc, argv, 0, &argp_index, 0);

	// the positional energy argument must be handled manually
	if(argp_index+1 != argc) {
		G4cerr << "Too many or too few arguments!" << G4endl;
		G4cout << " > argp_index = " << argp_index << G4endl;
		G4cout << " > argc = " << argc << G4endl;
		for(int i=0; i<argc; i++) {
			G4cout << " > argv[" << i << "] = `" << argv[i] << "`" << G4endl;
		}
		exit(-1);
	}
	G4double energy = atof(argv[argp_index])*GeV;
	G4cout << "E_0 = " << energy/MeV << " MeV" << G4endl;

	// print the table of materials
	G4cout << *(G4Material::GetMaterialTable()) << G4endl;

	// construct the default run manager
	G4RunManager* runManager = new G4RunManager;

	// set mandatory initialization classes
	runManager->SetUserInitialization(new DetectorConstruction(p_radius));

	G4PhysListFactory factory;
	factory.SetVerbose(0);
	G4VUserPhysicsList * physicslist = factory.GetReferencePhysList("QGSP_BERT");
	runManager->SetUserInitialization(physicslist);

	// set user actions
	runManager->SetUserAction(new PrimaryGeneratorAction(2212, energy));

	UserActionManager uam(p_tracks);
	runManager->SetUserAction(uam.getUserEventAction());
	runManager->SetUserAction(uam.getUserSteppingAction());
	runManager->SetUserAction(uam.getUserStackingAction());

	// initialize G4 kernel
	runManager->Initialize();
	CLHEP::HepRandom::setTheSeed(p_seed==0 ? std::time(0) : p_seed);

	// start a run
	runManager->BeamOn(p_runs);

	// store histograms
	//hists.saveHistograms();

	// job termination
	delete runManager;
	return 0;
}
