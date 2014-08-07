#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <sys/stat.h>
#include <hdf5.h>
#include <hdf5_hl.h>

using namespace std;

struct HDFTableInfo
{
	// abstract structure information
	hsize_t nfields;
	char ** field_names;
	hid_t * field_types;
	size_t * field_sizes;

	// instance information
	string name;
	size_t * field_offsets;
	size_t type_size;
	hsize_t nrecords;

	// methods
	HDFTableInfo(hid_t group, string name);
	~HDFTableInfo();
	void printInfo();
	size_t findField(const string & fieldname);
};

HDFTableInfo::HDFTableInfo(hid_t group, string name_)
: name(name_)
{
	H5TBget_table_info(group, name.c_str(), &nfields, &nrecords);

	field_names = new char*[nfields];
	for(size_t i=0; i<nfields; i++) {
		field_names[i] = new char[32];
	}
	field_sizes = new size_t[nfields];
	field_offsets = new size_t[nfields];
	field_types = new hid_t[nfields];

	hid_t dsid = H5Dopen(group, name.c_str(), H5P_DEFAULT);
	hid_t dstype = H5Dget_type(dsid);
	type_size = H5Tget_size(dstype);
	for(size_t i=0; i<nfields; i++) {
		field_names[i] = H5Tget_member_name(dstype, i);
		field_offsets[i] = H5Tget_member_offset(dstype, i);
		field_types[i] = H5Tget_member_type(dstype, i);
		field_sizes[i] = H5Tget_size(field_types[i]);
	}
	H5Dclose(dsid);
}

HDFTableInfo::~HDFTableInfo()
{
	for(size_t i=0; i<nfields; i++) {
		delete[] field_names[i];
	}
	delete[] field_names;
	delete[] field_sizes;
	delete[] field_offsets;
}

size_t HDFTableInfo::findField(const string & fieldname)
{
	for(size_t i=0; i<nfields; i++) {
		if(fieldname == field_names[i]) {
			return i;
		}
	}
	throw out_of_range("Field `"+fieldname+"` not found!");
}

void HDFTableInfo::printInfo()
{
	cout << "Table: " << name << endl;
	cout << "  nfields: " << nfields << endl;
	cout << "  nrecords: " << nrecords << endl;
	cout << "  type_size: " << type_size << endl;
	for(size_t i=0; i<nfields; i++) {
		cout << "   - " << field_names[i] << " <" << field_offsets[i] << ", " << field_sizes[i] << ">" << endl;
	}
}

hid_t create_hdf5_string(size_t length)
{
	hid_t ret = H5Tcopy(H5T_C_S1);
	H5Tset_size(ret, length);
	return ret;
}

void string_to_cstr(const std::string &src, char dst[], size_t target_size)
{
	src.copy(dst, target_size-1);
	for(size_t i=src.size(); i<target_size; i++) {
		dst[i] = '\0';
	}
}

struct Run {
	hsize_t event_first, event_size;
	hsize_t particle_first, particle_size;
	char file_path[64];
	double cutoff, gunradius;
	int seed;
	char model_file[32];
	unsigned int model_crc;

	static const hsize_t nfields = 10;
	static const char * names[nfields];
	static const size_t offsets[nfields];
	static const size_t sizes[nfields];
	static const hid_t types[nfields];
};
const char * Run::names[] = {
	"event_first", "event_size",
	"particle_first", "particle_size",
	"file_path",
	"cutoff", "gunradius", "seed",
	"model_file", "model_crc"
};
const size_t Run::offsets[] = {
	HOFFSET(Run, event_first), HOFFSET(Run, event_size),
	HOFFSET(Run, particle_first), HOFFSET(Run, particle_size),
	HOFFSET(Run, file_path),
	HOFFSET(Run, cutoff), HOFFSET(Run, gunradius), HOFFSET(Run, seed),
	HOFFSET(Run, model_file), HOFFSET(Run, model_crc)
};
const size_t Run::sizes[] = {
	sizeof(Run::event_first), sizeof(Run::event_size),
	sizeof(Run::particle_first), sizeof(Run::particle_size),
	sizeof(Run::file_path),
	sizeof(Run::cutoff), sizeof(Run::gunradius), sizeof(Run::seed),
	sizeof(Run::model_file), sizeof(Run::model_crc)
};
const hid_t Run::types[] = {
	H5T_NATIVE_HSIZE, H5T_NATIVE_HSIZE,
	H5T_NATIVE_HSIZE, H5T_NATIVE_HSIZE,
	create_hdf5_string(sizeof(Run::file_path)),
	H5T_NATIVE_DOUBLE, H5T_NATIVE_DOUBLE, H5T_NATIVE_INT,
	create_hdf5_string(sizeof(Run::model_file)), H5T_NATIVE_UINT
};

template<typename T>
T hdf_read_attribute(hid_t loc, const string & name, hid_t type)
{
	hid_t attr = H5Aopen(loc, name.c_str(), H5P_DEFAULT);
	if(attr < 0) {
		throw out_of_range("unable to open attribute ("+name+")");
	}

	T buf;
	H5Aread(attr, type, &buf);
	H5Aclose(attr);
	return buf;
}

string hdf_read_attribute_string(hid_t loc, const string & name)
{
	hid_t attr = H5Aopen(loc, name.c_str(), H5P_DEFAULT);
	if(attr < 0) {
		throw out_of_range("unable to open attribute ("+name+")");
	}

	hid_t type = H5Aget_type(attr);
	size_t type_size = H5Tget_size(type);
	char * buf = new char[type_size+1];
	H5Aread(attr, type, buf);
	H5Tclose(type);
	H5Aclose(attr);
	buf[type_size] = 0;
	string ret(buf);
	delete[] buf;
	return ret;
}

int main(int argc, char * argv[])
{
	// Check that all the input files exists
	if(argc <= 1) {
		cerr << "Error: no input files given." << endl;
		exit(1);
	}
	for(int i=1; i<argc; i++) {
		struct stat statbuf;
		if(stat(argv[i], &statbuf) != 0) {
			cerr << "Error(" << errno << "): stat() failed on " << argv[i] << endl;
			exit(2);
		}
	}

	// Read structural information from the first file
	hid_t fh_first = H5Fopen(argv[1], H5F_ACC_RDONLY, H5P_DEFAULT);
	HDFTableInfo events_info(fh_first, "events");
	HDFTableInfo particles_info(fh_first, "particles");
	cout << "--- Structural information ---" << endl;
	events_info.printInfo();
	particles_info.printInfo();
	H5Fclose(fh_first);

	// Create the output file and tables within
	hid_t fout = H5Fcreate("outfile.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	{
		// create the runs table
		H5TBmake_table("Runs", fout, "runs", Run::nfields,
			0, sizeof(Run), Run::names, Run::offsets, Run::types,
			1000, 0, H5P_DEFAULT, 0
		);
	}
	H5TBmake_table("Particles", fout, "particles", particles_info.nfields,
		0, particles_info.type_size, (const char**)particles_info.field_names,
		particles_info.field_offsets, particles_info.field_types,
		1000, 0, H5P_DEFAULT, 0
	);
	H5TBmake_table("Events", fout, "events", events_info.nfields,
		0, events_info.type_size, (const char**)events_info.field_names,
		events_info.field_offsets, events_info.field_types,
		1000, 0, H5P_DEFAULT, 0
	);

	// Loop over input files and combine them to an output file
	cout << "--- Merging files ---" << endl;
	hsize_t particle_offset = 0, event_offset = 0;
	// 1 MB buffer
	const size_t BUFFER_SIZE = 1024*1024;
	unsigned char * data = new unsigned char[BUFFER_SIZE];

	for(int i=1; i<argc; i++) {
		cout << "Reading: " << argv[i] << endl;
		hid_t fh = H5Fopen(argv[i], H5F_ACC_RDONLY, H5P_DEFAULT);
		HDFTableInfo this_events_info(fh, "events");
		HDFTableInfo this_particles_info(fh, "particles");

		// add the run attributes
		Run run;
		run.event_first = event_offset;
		run.event_size = this_events_info.nrecords;
		run.particle_first = particle_offset;
		run.particle_size = this_particles_info.nrecords;
		string_to_cstr(argv[i], run.file_path, sizeof(Run::file_path));
		try {
			string_to_cstr(hdf_read_attribute_string(fh, "model_file"), run.model_file, sizeof(Run::model_file));
			run.model_crc = hdf_read_attribute<unsigned int>(fh, "model_crc", H5T_NATIVE_UINT);
			run.seed = hdf_read_attribute<int>(fh, "seed", H5T_NATIVE_INT);
			run.cutoff = hdf_read_attribute<double>(fh, "cutoff", H5T_NATIVE_DOUBLE);
			run.gunradius = hdf_read_attribute<double>(fh, "gunradius", H5T_NATIVE_DOUBLE);
		} catch(out_of_range &e) {
			cerr << "Error getting attributes: " << e.what() << endl;
			string_to_cstr("<MERGE ERROR>", run.model_file, sizeof(Run::model_file));
			run.model_crc = 0;
			run.seed = 0;
			run.cutoff = nan("");
			run.gunradius = nan("");
		}
		H5TBappend_records(fout, "runs", 1, sizeof(Run), Run::offsets, Run::sizes, &run);

		{
			// Copy events to the new file
			size_t first_idx = this_events_info.findField("first");
			const size_t first_offset = this_events_info.field_offsets[first_idx];
			size_t eventid_idx = this_particles_info.findField("eventid");
			const size_t eventid_offset = this_particles_info.field_offsets[eventid_idx];
			const size_t type_size = this_events_info.type_size;
			hsize_t buffered_records = BUFFER_SIZE/type_size;
			for(hsize_t record=0, delta; record < this_events_info.nrecords; record+=delta) {
				delta = min(this_events_info.nrecords-record, buffered_records);
				cout << " > copying " << delta << " records." << endl;
				H5TBread_records(fh, "events", record, delta, this_events_info.type_size,
					this_events_info.field_offsets, this_events_info.field_sizes,
					data
				);
				// update the event IDs
				for(hsize_t j=0; j<delta; j++) {
					unsigned int & first = *((unsigned int*)(data + j*type_size + first_offset));
					first += particle_offset;

					unsigned int & eventid = *((unsigned int*)(data + j*type_size + eventid_offset));
					eventid += event_offset;
				}
				// write the buffer
				H5TBappend_records(fout, "events", delta, this_events_info.type_size,
					this_events_info.field_offsets, this_events_info.field_sizes,
					data
				);
			}
		}
		{
			// Copy particles to the new file
			size_t eventid_idx = this_particles_info.findField("eventid");
			const size_t eventid_offset = this_particles_info.field_offsets[eventid_idx];
			const size_t type_size = this_particles_info.type_size;
			hsize_t buffered_records = BUFFER_SIZE/type_size;
			for(hsize_t record=0, delta; record < this_particles_info.nrecords; record+=delta) {
				delta = min(this_particles_info.nrecords-record, buffered_records);
				cout << " > copying " << delta << " records." << endl;
				H5TBread_records(fh, "particles", record, delta, this_particles_info.type_size,
					this_particles_info.field_offsets, this_particles_info.field_sizes,
					data
				);
				// update the event IDs
				for(hsize_t j=0; j<delta; j++) {
					unsigned int & eventid = *((unsigned int*)(data + j*type_size + eventid_offset));
					eventid += event_offset;
				}
				// write the buffer
				H5TBappend_records(fout, "particles", delta, this_particles_info.type_size,
					this_particles_info.field_offsets, this_particles_info.field_sizes,
					data
				);
			}
		}

		event_offset += this_events_info.nrecords;
		particle_offset += this_particles_info.nrecords;

		H5Fclose(fh);
	}
	delete[] data;

	H5Fclose(fout);

	return 0;
}
