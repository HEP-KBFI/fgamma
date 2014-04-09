#include "HDFWriter.hh"

#include <iostream>
#include <hdf5_hl.h>

using namespace std;

#define S_SIZEOF(type, member) sizeof(((type *)0)->member)

void string_to_cstr(const std::string &src, char dst[], size_t target_size)
{
	src.copy(dst, target_size-1);
	for(size_t i=src.size(); i<target_size; i++) {
		dst[i] = '\0';
	}
}

hid_t create_hdf5_string(size_t length)
{
	hid_t ret = H5Tcopy(H5T_C_S1);
	H5Tset_size(ret, length);
	return ret;
}
const hid_t H5T_C_S16 = create_hdf5_string(16);

const hsize_t HDFWriter::nfields = 18;
const char * HDFWriter::field_names[] = {
	"eventid",
	"pid", "name", "mass",
	"vtx.KE",
	"vtx.x", "vtx.y", "vtx.z",
	"vtx.px", "vtx.py", "vtx.pz",
	"boundary.KE",
	"boundary.x", "boundary.y", "boundary.z",
	"boundary.px", "boundary.py", "boundary.pz"
};
const hid_t HDFWriter::field_types[] = {
	H5T_NATIVE_UINT,
	H5T_NATIVE_INT, H5T_C_S16, H5T_NATIVE_DOUBLE,
	H5T_NATIVE_DOUBLE,
	H5T_NATIVE_DOUBLE, H5T_NATIVE_DOUBLE, H5T_NATIVE_DOUBLE,
	H5T_NATIVE_DOUBLE, H5T_NATIVE_DOUBLE, H5T_NATIVE_DOUBLE,
	H5T_NATIVE_DOUBLE,
	H5T_NATIVE_DOUBLE, H5T_NATIVE_DOUBLE, H5T_NATIVE_DOUBLE,
	H5T_NATIVE_DOUBLE, H5T_NATIVE_DOUBLE, H5T_NATIVE_DOUBLE
};
const size_t HDFWriter::field_offset[] = {
	HOFFSET(particle, eventid),
	HOFFSET(particle, pid), HOFFSET(particle, name), HOFFSET(particle, m),
	HOFFSET(particle, vtx.KE),
	HOFFSET(particle, vtx.x), HOFFSET(particle, vtx.y), HOFFSET(particle, vtx.z),
	HOFFSET(particle, vtx.px), HOFFSET(particle, vtx.py), HOFFSET(particle, vtx.pz),
	HOFFSET(particle, boundary.KE),
	HOFFSET(particle, boundary.x), HOFFSET(particle, boundary.y), HOFFSET(particle, boundary.z),
	HOFFSET(particle, boundary.px), HOFFSET(particle, boundary.py), HOFFSET(particle, boundary.pz)
};
const size_t HDFWriter::field_sizes[] = {
	S_SIZEOF(particle, eventid),
	S_SIZEOF(particle, pid), S_SIZEOF(particle, name), S_SIZEOF(particle, m),
	S_SIZEOF(particle, vtx.KE),
	S_SIZEOF(particle, vtx.x), S_SIZEOF(particle, vtx.y), S_SIZEOF(particle, vtx.z),
	S_SIZEOF(particle, vtx.px), S_SIZEOF(particle, vtx.py), S_SIZEOF(particle, vtx.pz),
	S_SIZEOF(particle, boundary.KE),
	S_SIZEOF(particle, boundary.x), S_SIZEOF(particle, boundary.y), S_SIZEOF(particle, boundary.z),
	S_SIZEOF(particle, boundary.px), S_SIZEOF(particle, boundary.py), S_SIZEOF(particle, boundary.pz)
};

HDFWriter::HDFWriter(const string fname)
: table_exists(false)
{
	cout << "HDF5 file: " << fname << endl;
	file = H5Fcreate(fname.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
}

HDFWriter::~HDFWriter()
{
	H5Fclose(file);
}

void HDFWriter::writeEvent(int evid, double E, double incidence, std::vector<particle> ps)
{
	const char dset_name[] = "particles";

	if(table_exists) {
		H5TBappend_records(
			file, dset_name, ps.size(),
			sizeof(particle), field_offset, field_sizes,
			&ps[0]
		);
	} else {
		H5TBmake_table(
			"Particles in an event.", file, dset_name,
			HDFWriter::nfields, ps.size(), sizeof(particle),
			HDFWriter::field_names, HDFWriter::field_offset, field_types,
			ps.size(), 0, H5P_DEFAULT, &ps[0]
		);
		table_exists = true;
	}
}
