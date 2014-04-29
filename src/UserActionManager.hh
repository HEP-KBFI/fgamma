#ifndef UserActionManager_h
#define UserActionManager_h

#include "HDFTable.hh"
#include <G4String.hh>
#include <fstream>
#include <vector>

class G4UserSteppingAction;
class G4UserEventAction;
class G4UserStackingAction;
class G4UserTrackingAction;
class Timer;

class UserActionManager
{
	public:
		UserActionManager(Timer& timer, bool store_tracks, double cutoff=0.0, G4String prefix = "");
		~UserActionManager();

		G4UserSteppingAction * getUserSteppingAction();
		G4UserEventAction * getUserEventAction();
		G4UserStackingAction * getUserStackingAction();
		G4UserTrackingAction * getUserTrackingAction();

		struct CommonVariables
		{
			G4int evid;
			std::ofstream event_stream;
			std::ofstream track_stream;
			bool store_tracks;
			Timer& timer;
			double cutoff;

			hid_t hdf_file;
			HDFTable table;
			struct particle_t
			{
				unsigned int & eventid;
				char (&name)[16];
				int & pid;
				double & m;
				struct kinematics_t {
					double &KE;
					double &x, &y, &z;
					double &px, &py, &pz;

					kinematics_t(const HDFTable &table, const std::string &prefix);
				} vtx, boundary;

				particle_t(const HDFTable &table);
			} particle;

			CommonVariables(const G4String fname, Timer& timer_);
			std::vector<HDFTableField> hdf_fields();
		};

	private:
		G4UserSteppingAction * userSteppingAction;
		G4UserEventAction * userEventAction;
		G4UserStackingAction * userStackingAction;
		G4UserTrackingAction * userTrackingAction;
		CommonVariables pUAI;
};

#endif
