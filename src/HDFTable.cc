#include "HDFTable.hh"

#include <cstring>
#include <iostream>
#include <hdf5_hl.h>

using namespace std;

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

// ---------------------------------------------------------------------
//                    struct HDFTableField
// ---------------------------------------------------------------------

HDFTableField::HDFTableField(const hid_t field_type, const std::string &field_name)
: type(field_type), name(field_name), size(H5Tget_size(type))
{
	cout << "Add field: " << name << " (size: " << size << ", hid: " << type << ")" << endl;
}

// ---------------------------------------------------------------------
//                      class HDFTable
// ---------------------------------------------------------------------

HDFTable::HDFTable(const hid_t h5group, const std::string &tablename, const std::vector<HDFTableField> &fields, size_t buffered_fields)
: group(h5group), tname(tablename), table_exists(false), nfields(fields.size()), buffer_size(buffered_fields), inbuffer(0)
{
	field_names  = new const char*[nfields];
	field_offset = new size_t[nfields];
	field_sizes  = new size_t[nfields];
	field_types  = new hid_t[nfields];

	size_t offset = 0;
	for(size_t i=0; i<fields.size(); i++) {
		const HDFTableField &field = fields[i];

		field_names[i] = field.name.c_str();
		field_offset[i] = offset;
		field_sizes[i] = field.size;
		field_types[i] = field.type;

		offset_map.insert(pair<string,size_t>(field.name, offset));
		offset += field.size;
	}
	type_size = offset;

	data = new unsigned char[type_size];
	if(buffer_size == 1) {
		buffer = data;
	} else {
		buffer = new unsigned char[type_size*buffer_size];
	}
}

void HDFTable::writeBuffer()
{
	if(table_exists) {
		H5TBappend_records(
			group, tname.c_str(), inbuffer,
			type_size, field_offset, field_sizes,
			buffer
		);
	} else {
		H5TBmake_table(
			"Particles in an event.", group, tname.c_str(),
			nfields, inbuffer, type_size,
			field_names, field_offset, field_types,
			1, 0, H5P_DEFAULT, buffer
		);
		table_exists = true;
	}

	inbuffer = 0;
}

void HDFTable::write()
{
	memcpy(buffer+type_size*inbuffer, data, type_size);
	inbuffer++;
	if(inbuffer == buffer_size) {
		writeBuffer();
	}
}

void HDFTable::flush()
{
	if(inbuffer > 0) {
		writeBuffer();
	}
}
