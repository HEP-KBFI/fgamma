#include "UserActionManager.hh"

#include "Timer.hh"

#include <G4UserSteppingAction.hh>
#include <G4UserEventAction.hh>
#include <G4UserStackingAction.hh>
#include <G4UserTrackingAction.hh>
#include <G4VUserTrackInformation.hh>

#include <G4Event.hh>
#include <G4Track.hh>
#include <G4Gamma.hh>

using namespace CLHEP;

#define STRUCT_SIZEOF(type, member) sizeof(((type *)0)->member)

// ---------------------------------------------------------------------
//                  Geant4 user action classes
// ---------------------------------------------------------------------

class UAIUserSteppingAction : public G4UserSteppingAction
{
	UserActionManager::CommonVariables &pUAI;
	public:
		UAIUserSteppingAction(UserActionManager::CommonVariables& uai) : pUAI(uai) {}
		void UserSteppingAction(const G4Step * step);
};

class UAIUserEventAction : public G4UserEventAction
{
	UserActionManager::CommonVariables &pUAI;
	public:
		UAIUserEventAction(UserActionManager::CommonVariables& uai) : pUAI(uai) {}
		void BeginOfEventAction(const G4Event * ev);
		void EndOfEventAction(const G4Event * ev);
};

class UAIUserStackingAction : public G4UserStackingAction
{
	UserActionManager::CommonVariables &pUAI;
	public:
		UAIUserStackingAction(UserActionManager::CommonVariables& uai) : pUAI(uai) {}
		G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* tr);
};

class UAIUserTrackingAction : public G4UserTrackingAction
{
	UserActionManager::CommonVariables &pUAI;
	public:
		UAIUserTrackingAction(UserActionManager::CommonVariables& uai) : pUAI(uai) {}
		void PostUserTrackingAction(const G4Track* tr);
};

// ---------------------------------------------------------------------
//         Implementations of the Geant4 user action classes
// ---------------------------------------------------------------------

class UserTrackInformation : public G4VUserTrackInformation
{
	public:
		UserTrackInformation() : G4VUserTrackInformation() {
			//
		}
};

void UAIUserEventAction::BeginOfEventAction(const G4Event * ev)
{
	G4cout << "EVENT " << ev->GetEventID() << "      " << pUAI.timer.elapsed() << G4endl;
	pUAI.evid = ev->GetEventID();
}

void UAIUserEventAction::EndOfEventAction(const G4Event*)
{
	pUAI.evid = -1;
}

G4ClassificationOfNewTrack UAIUserStackingAction::ClassifyNewTrack(const G4Track* tr)
{
	pUAI.track_stream << pUAI.evid << ","
	                  << tr->GetParentID() << "," << tr->GetTrackID()
	                  << "," << tr->GetParticleDefinition()->GetPDGEncoding()
	                  << "," << tr->GetParticleDefinition()->GetParticleName()
	                  << "," << tr->GetKineticEnergy()/MeV
	                  << "," << tr->GetPosition().mag()/km
	                  << G4endl;
	return tr->GetKineticEnergy() < pUAI.cutoff ? fKill : fUrgent;
}

void UAIUserSteppingAction::UserSteppingAction(const G4Step * step)
{
	//step->GetTrack()->GetParticleDefinition() != G4Gamma::Definition() // check gammas
	if(step->GetPostStepPoint()->GetStepStatus() != fWorldBoundary) {
		return;
	}

	G4int pid = step->GetTrack()->GetParticleDefinition()->GetPDGEncoding();
	const G4String& name = step->GetTrack()->GetParticleDefinition()->GetParticleName();
	const G4double& mass = step->GetTrack()->GetDynamicParticle()->GetMass();

	G4double vertex_KE = step->GetTrack()->GetVertexKineticEnergy();
	const G4ThreeVector& vertex = step->GetTrack()->GetVertexPosition();
	const G4ThreeVector& vertex_pdir = step->GetTrack()->GetVertexMomentumDirection();

	G4double E = step->GetTrack()->GetTotalEnergy();
	const G4ThreeVector& pos = step->GetPostStepPoint()->GetPosition();
	const G4ThreeVector& pdir = step->GetTrack()->GetMomentumDirection();

	pUAI.event_stream << std::setw(5) << pUAI.evid << ',' << std::setw(4) << pid << ',' << std::setw(12) << name
	                  << std::scientific << std::setprecision(10)
	                  << ',' << mass/MeV
	                  << ',' << vertex_KE/MeV
	                  << ',' << vertex.x()/km << ',' << vertex.y()/km << ',' << vertex.z()/km
	                  << ',' << vertex_pdir.x() << ',' << vertex_pdir.y() << ',' << vertex_pdir.z()
	                  << ',' << E/MeV
	                  << ',' << pos.x()/km << ',' << pos.y()/km << ',' << pos.z()/km
	                  //<< ',' << pos.theta() << ',' << pos.phi()
	                  << ',' << pdir.x() << ',' << pdir.y() << ',' << pdir.z()
	                  //<< ',' << pdir.theta() << ',' << pdir.phi()
	                  << G4endl;

	UserActionManager::CommonVariables::particle_t &p = pUAI.particle;
	p.eventid = pUAI.evid;
	p.pid = pid;
	string_to_cstr(name, p.name, sizeof(p.name));
	p.m = mass/GeV;

	p.vtx.KE = vertex_KE/GeV;
	p.vtx.x = vertex.x()/km; p.vtx.y = vertex.y()/km; p.vtx.z = vertex.z()/km;
	p.vtx.px = vertex_pdir.x(); p.vtx.py = vertex_pdir.y(); p.vtx.pz = vertex_pdir.z();

	p.boundary.KE = vertex_KE/GeV;
	p.boundary.x = pos.x()/km; p.boundary.y = pos.y()/km; p.boundary.z = pos.z()/km;
	p.boundary.px = pdir.x(); p.boundary.py = pdir.y(); p.boundary.pz = pdir.z();

	pUAI.table.write();
}

void UAIUserTrackingAction::PostUserTrackingAction(const G4Track* tr)
{
	pUAI.track_stream << " > PostTrack(" << tr->GetTrackID() << "): stepp - " << tr->GetStep() << G4endl;
	if(tr->GetStep()->GetPostStepPoint()->GetStepStatus() != fWorldBoundary) {
		pUAI.track_stream << "   > BOUNDARY!" << G4endl;
	}
}

// ---------------------------------------------------------------------
//                  UserActionManager implementation
// ---------------------------------------------------------------------

UserActionManager::UserActionManager(Timer& timer, bool store_tracks, double cutoff, G4String prefix)
: pUAI(prefix+".h5", timer)
{
	pUAI.evid = -1;
	pUAI.event_stream.open((prefix+".csv").c_str());
	pUAI.store_tracks = store_tracks;
	pUAI.cutoff = cutoff;

	if(pUAI.store_tracks) {
		pUAI.track_stream.open((prefix+".tracks.csv").c_str());
	}

	userSteppingAction = new UAIUserSteppingAction(pUAI);
	userEventAction = new UAIUserEventAction(pUAI);
	userStackingAction = new UAIUserStackingAction(pUAI);
	userTrackingAction = new UAIUserTrackingAction(pUAI);

	// Write the header of the events CSV file
	pUAI.event_stream << "event,pid,name,m,vKE,vx,vy,vz,vpx,vpy,vpz,E,x,y,z,px,py,pz" << G4endl;
	pUAI.track_stream << "event,parentid,trackid,pid,particle,Ekin" << G4endl;
}

UserActionManager::CommonVariables::CommonVariables(const G4String fname, Timer& timer_)
: timer(timer_),
  hdf_file(H5Fcreate(fname.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)),
  table(hdf_file, "particles", hdf_fields(), 500),
  particle(table)
{}

UserActionManager::CommonVariables::particle_t::particle_t(const HDFTable &table)
: eventid(table.bind<unsigned int>("eventid")),
  name(table.bind<char[16]>("name")),
  pid(table.bind<int>("pid")),
  m(table.bind<double>("mass")),
  vtx(table, "vtx"), boundary(table, "boundary")
{}

UserActionManager::CommonVariables::particle_t::kinematics_t::kinematics_t(const HDFTable &table, const std::string &prefix)
: KE(table.bind<double>(prefix+".KE")),
  x(table.bind<double>(prefix+".x")), y(table.bind<double>(prefix+".y")), z(table.bind<double>(prefix+".z")),
  px(table.bind<double>(prefix+".px")), py(table.bind<double>(prefix+".py")), pz(table.bind<double>(prefix+".pz"))
{}

std::vector<HDFTableField> UserActionManager::CommonVariables::hdf_fields()
{
	std::vector<HDFTableField> ret;
	ret.reserve(18);
	ret.push_back(HDFTableField(H5T_NATIVE_UINT, "eventid"));
	ret.push_back(HDFTableField(H5T_NATIVE_INT, "pid"));
	ret.push_back(HDFTableField(create_hdf5_string(STRUCT_SIZEOF(particle_t,name)), "name"));
	ret.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "mass"));
	ret.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "vtx.KE"));
	ret.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "vtx.x"));
	ret.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "vtx.y"));
	ret.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "vtx.z"));
	ret.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "vtx.px"));
	ret.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "vtx.py"));
	ret.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "vtx.pz"));
	ret.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "boundary.KE"));
	ret.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "boundary.x"));
	ret.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "boundary.y"));
	ret.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "boundary.z"));
	ret.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "boundary.px"));
	ret.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "boundary.py"));
	ret.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "boundary.pz"));
	return ret;
}

UserActionManager::~UserActionManager()
{
	pUAI.table.flush();
	pUAI.track_stream.close();
	pUAI.event_stream.close();
}

G4UserSteppingAction * UserActionManager::getUserSteppingAction()
{
	return userSteppingAction;
}

G4UserEventAction * UserActionManager::getUserEventAction()
{
	return userEventAction;
}

G4UserStackingAction * UserActionManager::getUserStackingAction()
{
	return userStackingAction;
}

G4UserTrackingAction * UserActionManager::getUserTrackingAction()
{
	return userTrackingAction;
}
