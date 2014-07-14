#include <iostream>
#include <string>
#include <vector>
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
};

HDFTableInfo::HDFTableInfo(hid_t group, string name_)
: name(name_)
{
	cout << "  name: " << name << endl;
	H5TBget_table_info(group, name.c_str(), &nfields, &nrecords);
	cout << "  nfields: " << nfields << endl;
	cout << "  nrecords: " << nrecords << endl;

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
	cout << "  type_size: " << type_size << endl;
	for(size_t i=0; i<nfields; i++) {
		field_names[i] = H5Tget_member_name(dstype, i);
		field_offsets[i] = H5Tget_member_offset(dstype, i);
		field_types[i] = H5Tget_member_type(dstype, i);
		field_sizes[i] = H5Tget_size(field_types[i]);
		cout << "  - " << field_names[i] << " <" << field_offsets[i] << ", " << field_sizes[i] << ">" << endl;
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
	H5Fclose(fh_first);

	// Create the output file and tables within
	hid_t fout = H5Fcreate("outfile.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
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
	cout << "Input files:" << endl;
	for(int i=1; i<argc; i++) {
		cout << argv[i] << endl;
		hid_t fh = H5Fopen(argv[i], H5F_ACC_RDONLY, H5P_DEFAULT);
		{
			HDFTableInfo this_events_info(fh, "events");
			unsigned char * data = new unsigned char[this_events_info.type_size*this_events_info.nrecords];
			H5TBread_table(fh, "events", this_events_info.type_size,
				this_events_info.field_offsets, this_events_info.field_sizes,
				data
			);
			H5TBappend_records (fout, "events", this_events_info.nrecords, this_events_info.type_size,
				this_events_info.field_offsets, this_events_info.field_sizes,
				data
			);
			delete[] data;
		}
		{
			HDFTableInfo this_particles_info(fh, "particles");
			unsigned char * data = new unsigned char[this_particles_info.type_size*this_particles_info.nrecords];
			H5TBread_table(fh, "particles", this_particles_info.type_size,
				this_particles_info.field_offsets, this_particles_info.field_sizes,
				data
			);
			H5TBappend_records (fout, "particles", this_particles_info.nrecords, this_particles_info.type_size,
				this_particles_info.field_offsets, this_particles_info.field_sizes,
				data
			);
			delete[] data;
		}

		H5Fclose(fh);
	}

	H5Fclose(fout);

	return 0;
}
