#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <sys/stat.h>
#include <hdf5.h>
#include <hdf5_hl.h>

using namespace std;

const string datatype_class_string(const hid_t type)
{
	switch(H5Tget_class(type)) {
		case H5T_INTEGER: return "H5T_INTEGER"; break;
		case H5T_FLOAT: return "H5T_FLOAT"; break;
		case H5T_STRING: return "H5T_STRING"; break;
		case H5T_BITFIELD: return "H5T_BITFIELD"; break;
		case H5T_OPAQUE: return "H5T_OPAQUE"; break;
		case H5T_COMPOUND: return "H5T_COMPOUND"; break;
		case H5T_REFERENCE: return "H5T_REFERENCE"; break;
		case H5T_ENUM: return "H5T_ENUM"; break;
		case H5T_VLEN: return "H5T_VLEN"; break;
		case H5T_ARRAY: return "H5T_ARRAY"; break;
		default: return "UNKNOWN"; break;
	}
}

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
		cout << "   - " << field_names[i] << " (" << datatype_class_string(field_types[i]) << ") <"
		     << field_offsets[i] << ", " << field_sizes[i] << ">" << endl;
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
	if(argc != 2) {
		cerr << "Error: bad number of arguments." << endl;
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
	hid_t fh = H5Fopen(argv[1], H5F_ACC_RDONLY, H5P_DEFAULT);
	HDFTableInfo events_info(fh, "events");
	HDFTableInfo particles_info(fh, "particles");
	cout << "--- Structural information ---" << endl;
	events_info.printInfo();
	particles_info.printInfo();

	// field offsets
	size_t eventid_offset = particles_info.field_offsets[events_info.findField("eventid")];
	//size_t first_offset = events_info.field_offsets[events_info.findField("first")];
	//size_t size_offset = events_info.field_offsets[events_info.findField("size")];
	size_t x_offset = particles_info.field_offsets[particles_info.findField("boundary.x")];
	size_t y_offset = particles_info.field_offsets[particles_info.findField("boundary.y")];
	size_t z_offset = particles_info.field_offsets[particles_info.findField("boundary.z")];

	// 1 MB buffer
	const size_t BUFFER_SIZE = 1024*1024*1024;
	unsigned char * data = new unsigned char[BUFFER_SIZE];
	hsize_t maxrecords = max<size_t>(1, BUFFER_SIZE/particles_info.type_size);

	hsize_t N=0, Ngr=0, Nsp=0;

	for(hsize_t record=0, data_offset=-maxrecords; record<particles_info.nrecords; record++) {
		if(data_offset+maxrecords == record) {
			cout << "Loading records: " << record << " (" << 100*double(record)/particles_info.nrecords << "%)" << endl;
			H5TBread_records(fh, "particles", record, min(maxrecords, particles_info.nrecords-record),
				particles_info.type_size, particles_info.field_offsets, particles_info.field_sizes,
				data
			);
			data_offset = record;
		}
		size_t offset = (record - data_offset) * particles_info.type_size;

		unsigned int & eventid = *(unsigned int*)(data + offset + eventid_offset);
		//unsigned int & first = *(unsigned int*)(data + offset + first_offset);
		//unsigned int & size = *(unsigned int*)(data + offset + size_offset);
		double & x = *(double*)(data + offset + x_offset);
		double & y = *(double*)(data + offset + y_offset);
		double & z = *(double*)(data + offset + z_offset);

		double R = sqrt(x*x + y*y + z*z);

		//cout << offset << ": " << eventid << " <" << x << ", " << y << ", " << z << "> R=" << R << endl;
		N++;
		if(R < 6500) Ngr++;
		else Nsp++;
	}

	// Totals
	cout << endl;
	cout << "=== TOTALS ===" << endl;
	cout << "N: " << N << endl;
	cout << "Ngr: " << Ngr << endl;
	cout << "Nsp: " << Nsp << endl;

	// Clean up
	delete[] data;
	H5Fclose(fh);

	return 0;
}
