#include "UserActionManager.hh"

#include "UserEventInformation.hh"
#include "Timer.hh"

#include <G4UserSteppingAction.hh>
#include <G4UserEventAction.hh>
#include <G4UserStackingAction.hh>
#include <G4UserTrackingAction.hh>
#include <G4VUserTrackInformation.hh>
#include <G4VProcess.hh>
#include <G4Event.hh>
#include <G4Track.hh>

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
		void PreUserTrackingAction(const G4Track* tr);
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
	UserEventInformation & eventinfo = *static_cast<UserEventInformation*>(ev->GetUserInformation());

	G4cout << "% event " << ev->GetEventID()
	       << "    " << pUAI.timer.elapsed()
	       << "    " << eventinfo
	       << G4endl;

	pUAI.event.id = ev->GetEventID();
	pUAI.event.first = pUAI.hdf_particles.nrows();
	pUAI.event.size = 0;
	pUAI.event.pid = eventinfo.pid;
	pUAI.event.E = eventinfo.E/GeV;
	pUAI.event.KE = eventinfo.KE/GeV;
	pUAI.event.incidence = eventinfo.incidence;
	pUAI.event.discarded = 0;
}

void UAIUserEventAction::EndOfEventAction(const G4Event*)
{
	pUAI.hdf_events.write();
}

G4ClassificationOfNewTrack UAIUserStackingAction::ClassifyNewTrack(const G4Track* tr)
{
	pUAI.tracklog.classification(tr);
	return fUrgent;
}

void UAIUserSteppingAction::UserSteppingAction(const G4Step * step)
{
	pUAI.tracklog.stepping(step);
	G4TrackVector &trv = *const_cast<G4Step*>(step)->GetfSecondary();
	trv.erase(
		remove_if(
			trv.begin()+pUAI.track_approved_secondaries,
			trv.end(),
			[this](G4Track * track) {
				bool remove = (track->GetKineticEnergy()<pUAI.cutoff);
				pUAI.tracklog.stepSecondary(track, remove);
				if(remove) {
					delete track;
					return true;
				}
				return false;
			}
		),
		trv.end()
	);
	pUAI.track_approved_secondaries = trv.size();
}

void UAIUserTrackingAction::PreUserTrackingAction(const G4Track* tr)
{
	pUAI.tracklog.preTracking(tr);
	pUAI.track_approved_secondaries = 0;
}

void UAIUserTrackingAction::PostUserTrackingAction(const G4Track* tr)
{
	bool on_boundary = (tr->GetStep()->GetPostStepPoint()->GetStepStatus() == fWorldBoundary);
	pUAI.tracklog.postTracking(tr, on_boundary);

	if(!on_boundary) return;

	G4int pid = tr->GetParticleDefinition()->GetPDGEncoding();
	const G4String& name = tr->GetParticleDefinition()->GetParticleName();
	const G4double& mass = tr->GetDynamicParticle()->GetMass();

	G4double vertex_KE = tr->GetVertexKineticEnergy();
	const G4ThreeVector& vertex = tr->GetVertexPosition();
	const G4ThreeVector& vertex_pdir = tr->GetVertexMomentumDirection();

	const G4ThreeVector& pos = tr->GetStep()->GetPostStepPoint()->GetPosition();
	const G4ThreeVector& pdir = tr->GetMomentumDirection();

	UserActionManager::CommonVariables::particle_t &p = pUAI.particle;
	p.eventid = pUAI.event.id;
	p.pid = pid;
	string_to_cstr(name, p.name, sizeof(p.name));
	p.m = mass/GeV;

	p.vtx.KE = vertex_KE/GeV;
	p.vtx.x = vertex.x()/km; p.vtx.y = vertex.y()/km; p.vtx.z = vertex.z()/km;
	p.vtx.px = vertex_pdir.x(); p.vtx.py = vertex_pdir.y(); p.vtx.pz = vertex_pdir.z();

	p.boundary.KE = vertex_KE/GeV;
	p.boundary.x = pos.x()/km; p.boundary.y = pos.y()/km; p.boundary.z = pos.z()/km;
	p.boundary.px = pdir.x(); p.boundary.py = pdir.y(); p.boundary.pz = pdir.z();

	if(!isnan(pUAI.acceptradius)) {
		double R = sqrt(pos.x()*pos.x() + pos.y()*pos.y() + pos.z()*pos.z());
		if(fabs(R-pUAI.acceptradius) < 0.1*km) {
			pUAI.hdf_particles.write();
			pUAI.event.size++;
		} else {
			pUAI.event.discarded++;
		}
	} else {
		pUAI.hdf_particles.write();
		pUAI.event.size++;
	}
}

// ---------------------------------------------------------------------
//                  UserActionManager implementation
// ---------------------------------------------------------------------

UserActionManager::UserActionManager(Timer& timer, bool store_tracks, double cutoff, G4String prefix, double acceptradius)
: pUAI(prefix+".h5", timer)
{
	pUAI.event.id = -1;
	pUAI.cutoff = cutoff;
	pUAI.acceptradius = acceptradius;

	if(store_tracks) {
		pUAI.tracklog.enable(prefix+".tracks.csv");
	}

	userSteppingAction = new UAIUserSteppingAction(pUAI);
	userEventAction = new UAIUserEventAction(pUAI);
	userStackingAction = new UAIUserStackingAction(pUAI);
	userTrackingAction = new UAIUserTrackingAction(pUAI);

	writeAttribute("cutoff", cutoff/GeV);
}

UserActionManager::CommonVariables::CommonVariables(const G4String fname, Timer& timer_)
: timer(timer_),
  hdf_file(H5Fcreate(fname.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)),
  hdf_events(hdf_file, "events", hdf_fields.events, 1), event(hdf_events),
  hdf_particles(hdf_file, "particles", hdf_fields.particles, 500), particle(hdf_particles)
{}

UserActionManager::CommonVariables::~CommonVariables()
{
	hdf_events.flush();
	hdf_particles.flush();

	H5Fclose(hdf_file);
}

UserActionManager::CommonVariables::event_t::event_t(const HDFTable &table)
: id(table.bind<unsigned int>("eventid")),
  first(table.bind<unsigned int>("first")),
  size(table.bind<unsigned int>("size")),
  pid(table.bind<int>("pid")),
  E(table.bind<double>("E")), KE(table.bind<double>("KE")),
  incidence(table.bind<double>("incidence")),
  discarded(table.bind<unsigned int>("discarded"))
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

UserActionManager::CommonVariables::hdf_fields_t::hdf_fields_t()
{
	events.reserve(5);
	events.push_back(HDFTableField(H5T_NATIVE_UINT, "eventid"));
	events.push_back(HDFTableField(H5T_NATIVE_UINT, "first"));
	events.push_back(HDFTableField(H5T_NATIVE_UINT, "size"));
	events.push_back(HDFTableField(H5T_NATIVE_INT, "pid"));
	events.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "E"));
	events.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "KE"));
	events.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "incidence"));
	events.push_back(HDFTableField(H5T_NATIVE_UINT, "discarded"));

	particles.reserve(18);
	particles.push_back(HDFTableField(H5T_NATIVE_UINT, "eventid"));
	particles.push_back(HDFTableField(H5T_NATIVE_INT, "pid"));
	particles.push_back(HDFTableField(create_hdf5_string(STRUCT_SIZEOF(particle_t,name)), "name"));
	particles.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "mass"));
	particles.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "vtx.KE"));
	particles.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "vtx.x"));
	particles.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "vtx.y"));
	particles.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "vtx.z"));
	particles.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "vtx.px"));
	particles.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "vtx.py"));
	particles.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "vtx.pz"));
	particles.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "boundary.KE"));
	particles.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "boundary.x"));
	particles.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "boundary.y"));
	particles.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "boundary.z"));
	particles.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "boundary.px"));
	particles.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "boundary.py"));
	particles.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "boundary.pz"));
}

UserActionManager::~UserActionManager()
{
	pUAI.hdf_events.flush();
	pUAI.hdf_particles.flush();
}

void UserActionManager::writeAttribute(const G4String & name, const double value)
{
	write_hdf5_attribute(pUAI.hdf_file, name, value);
}

void UserActionManager::writeAttribute(const G4String & name, const int value)
{
	write_hdf5_attribute(pUAI.hdf_file, name, value);
}

void UserActionManager::writeAttribute(const G4String & name, const unsigned int value)
{
	write_hdf5_attribute(pUAI.hdf_file, name, value);
}

void UserActionManager::writeAttribute(const G4String & name, const long value)
{
	write_hdf5_attribute(pUAI.hdf_file, name, value);
}

void UserActionManager::writeAttribute(const G4String & name, const unsigned long value)
{
	write_hdf5_attribute(pUAI.hdf_file, name, value);
}

void UserActionManager::writeAttribute(const G4String & name, const G4String & value)
{
	const hsize_t dims[] = {1};
	hid_t type = create_hdf5_string(value.size());
	hid_t sid = H5Screate_simple(1, dims, NULL);
	hid_t aid = H5Acreate(pUAI.hdf_file, name.c_str(), type, sid, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(aid, type, value.c_str());
	H5Aclose(aid);
	H5Sclose(sid);
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
