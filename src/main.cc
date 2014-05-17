#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"
#include "UserActionManager.hh"
#include "Timer.hh"
#include "configuration.hh"

#include "globals.hh"
#include <G4RunManager.hh>
#include <G4PhysListFactory.hh>
#include <G4NistManager.hh>

#include <G4UImanager.hh>
#include <G4UIExecutive.hh>
#include <G4VisExecutive.hh>
#include <G4VisExtent.hh>

#include <fstream>
#include <string>

template<class T>
T read_urandom()
{
	union {
		T value;
		char cs[sizeof(T)];
	} u;

	std::ifstream rfin("/dev/urandom");
	rfin.read(u.cs, sizeof(u.cs));
	rfin.close();

	return u.value;
}

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
#define PC_CUT   1004

// Program's arguments - an array of option specifiers
// name, short name, arg. name, flags, doc, group
const argp_option argp_options[] = {
	{0, 0, 0, 0, "General options:", 0},
	{"verbosity", 'v', "LEVEL", 0, "set the verbosity level (0 - minimal,"
		" 1 - a bit (default), 2 - a lot)", 0},
	{"prefix", 'o', "PREFIX", 0, "set the prefix of the output files", 0},
	{"vis", PC_VIS, 0, 0, "open the GUI instead of running the simulation", 0},
	{"eventfile", 'f', "FILE", 0, "file with event parameters"
		" (each line with eventconf syntax)", 0},

	{"seed", PC_SEED, "SEED", 0,
		"set the seed for the random generators; if this is not"
		" specified, time(0) is used)", 0},
	{"tracks", PC_TRCKS, 0, 0, "store tracks in tracks.txt", 0},

	{0, 0, 0, 0, "Options for tweaking the physics:", 2},
	{"model", 'm', "MODELFILE", 0,
		"set the YAML file used to model the geometry (default: model.yml)", 2},
	{"cutoff", PC_CUT, "CUT", 0,
		"define an energy cutoff (in GeVs)", 2},

	{0, 0, 0, 0, "Other:", -1},
	{0, 0, 0, 0, 0, 0} // terminates the array
};

// Parameters
int p_seed = 0;
G4String p_eventfile = "";
G4String p_modelfile = "model.yml";
bool p_tracks = false;
G4String p_prefix = "fgamma";
bool p_vis  = false; // go to visual mode (i.e. open the GUI instead)
int p_verbosity = 1;
double p_cutoff = 0.0;

// Argument parser callback called by argp
error_t argp_parser(int key, char *arg, struct argp_state*) {
	switch(key) {
		case 'o':
			p_prefix = arg;
			break;
		case 'f':
			p_eventfile = arg;
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
		case PC_CUT:
			p_cutoff = std::atof(arg)*GeV;
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
	"[EVENT...]",
	"Simulation of gamma-rays produced in the atmosphere by cosmic rays.",
	0, 0, 0
};

int main(int argc, char * argv[]) {
	Timer timer;

	// parse the arguments
	int argp_index;
	argp_parse(&argp_argp, argc, argv, 0, &argp_index, 0);

	G4cout << "% fgamma" << G4endl;

	// verbosity of Geant4 classes -- let's make sure they dont spam if p_verbosity == 1
	int geant_verbosity = p_verbosity==0 ? 0 : p_verbosity-1;
	G4cout << "verbosity: " << p_verbosity << G4endl;
	G4cout << "Geant4 verbosity: " << geant_verbosity << G4endl;

	// print the important simulation parameters (prefixed by a % for easy grepping)
	G4cout << "% modelfile " << p_modelfile << G4endl;
	G4cout << "% prefix " << p_prefix << G4endl;
	G4cout << "% cutoff " << p_cutoff/MeV << " MeV" << G4endl;

	// print timer start values
	if(p_verbosity > 1) {G4cout << "timer.start: " << timer.start << G4endl;}

	// we'll parse the event parameters from both arguments and from a file
	std::vector<eventconf> events;
	try {
		for(int i=argp_index; i<argc; i++) {
			events.push_back(eventconf::parse_string(argv[i]));
			if(p_verbosity > 1) {G4cout << "CLI(" << (i-argp_index) << "):  " << events.back() << G4endl;}
		}

		if(p_eventfile.size() > 0) {
			G4cout << "% eventfile " << p_eventfile << G4endl;
			std::ifstream fin(p_eventfile);
			int lineno = 1;
			for (std::string line; std::getline(fin, line); lineno++) {
				events.push_back(eventconf::parse_string(line));
				if(p_verbosity > 1) {G4cout << "file(" << lineno << "): " << events.back() << G4endl;}
			}
			fin.close();
		}
	} catch(eventconf::parse_error &e) {
		G4cerr << "ERROR: Bad event parameters: " << e.what() << G4endl;
		G4cerr << "  evstr: `" << e.evstr << "`, token: `" << e.token << "`" << G4endl;
		G4cerr << "  argp_index = " << argp_index << ", argc = " << argc << G4endl;
		for(int i=0; i<argc; i++) {
			G4cerr << "  argv[" << i << "] = `" << argv[i] << "`" << G4endl;
		}
		exit(1);
	}

	size_t total_events = 0;
	for(eventconf &ec : events) {
		total_events += ec.n;
	}
	if(total_events == 0) {
		G4cerr << "ERROR: no events!" << G4endl;
		exit(1);
	}
	G4cout << "% events " << total_events << G4endl;

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
	runManager->SetUserAction(new PrimaryGeneratorAction(gunradius, events));
	G4cout << "% gunradius " << gunradius/km << " km" << G4endl;

	UserActionManager uam(timer, p_tracks, p_cutoff, p_prefix);
	runManager->SetUserAction(uam.getUserEventAction());
	runManager->SetUserAction(uam.getUserSteppingAction());
	runManager->SetUserAction(uam.getUserStackingAction());
	runManager->SetUserAction(uam.getUserTrackingAction());

	// print the table of materials
	if(p_verbosity>1){G4cout << *(G4Material::GetMaterialTable()) << G4endl;}

	// set the random seed
	p_seed = p_seed==0 ? abs(read_urandom<int>()) : p_seed;
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
		runManager->BeamOn(total_events);
	}

	// job termination
	delete runManager;

	G4cout << "% done " << timer.elapsed() << G4endl;

	return 0;
}
