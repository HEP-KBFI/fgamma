#ifndef UserActionManager_h
#define UserActionManager_h

#include "HDFTable.hh"
#include "TrackingLog.hh"
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

		void writeAttribute(const G4String & name, const double value);
		void writeAttribute(const G4String & name, const int value);
		void writeAttribute(const G4String & name, const unsigned int value);
		void writeAttribute(const G4String & name, const G4String & value);

		G4UserSteppingAction * getUserSteppingAction();
		G4UserEventAction * getUserEventAction();
		G4UserStackingAction * getUserStackingAction();
		G4UserTrackingAction * getUserTrackingAction();

		struct CommonVariables
		{
			struct hdf_fields_t
			{
				std::vector<HDFTableField> events, particles;
				hdf_fields_t();
			} hdf_fields;

			TrackingLog tracklog;
			Timer& timer;
			double cutoff;

			hid_t hdf_file;

			HDFTable hdf_events;
			struct event_t
			{
				unsigned int & id;
				unsigned int & first;
				unsigned int & size;
				int & pid;
				double &E, &KE;
				double & incidence;

				event_t(const HDFTable &table);
			} event;

			HDFTable hdf_particles;
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

			size_t track_approved_secondaries;

			CommonVariables(const G4String fname, Timer& timer_);
			~CommonVariables();
		};

	private:
		G4UserSteppingAction * userSteppingAction;
		G4UserEventAction * userEventAction;
		G4UserStackingAction * userStackingAction;
		G4UserTrackingAction * userTrackingAction;
		CommonVariables pUAI;
};

#endif
