#ifndef HDFTable_h
#define HDFTable_h

#include <hdf5.h>

#include <vector>
#include <string>
#include <map>
#include <iostream>

void string_to_cstr(const std::string &src, char dst[], size_t target_size);
hid_t create_hdf5_string(size_t length);

// ---------------------------------------------------------------------
//                    struct HDFTableField
// ---------------------------------------------------------------------
struct HDFTableField
{
	hid_t type;
	std::string name;
	size_t size;

	HDFTableField(const hid_t field_type, const std::string &field_name);
};

// ---------------------------------------------------------------------
//                      class HDFTable
// ---------------------------------------------------------------------
class HDFTable
{
	const hid_t group;
	const std::string tname;

	std::map<std::string, size_t> offset_map;
	hsize_t nfields, type_size;
	const char ** field_names;
	hid_t * field_types;
	size_t * field_offset;
	size_t * field_sizes;

	unsigned char * data;
	unsigned char * buffer;
	size_t buffer_size, inbuffer, totalrows;

	public:
		HDFTable(const hid_t h5group, const std::string &tablename, const std::vector<HDFTableField> &fields, size_t buffered_fields = 1);
		template<class T> T& bind(const std::string & name) const;
		template<class T> void setAttribute(hid_t type, const std::string & name, T value);
		void write();
		void flush();
		size_t nrows() const;

	private:
		HDFTable(const HDFTable&);
		HDFTable& operator=(HDFTable);
		void writeBuffer();
};

template<class T>
T& HDFTable::bind(const std::string & name) const
{
	std::map<std::string, size_t>::const_iterator it = offset_map.find(name);
	if(it == offset_map.end()) {
		// TODO: DO SOMETHING ABOUT THIS EXCEPTION!!
		throw(999);
	}
	size_t offset = it->second;
	return *((T*)(data+offset));
}

template<class T>
void HDFTable::setAttribute(hid_t type, const std::string & name, T value)
{
	const hsize_t dims[] = {1};
	hid_t table = H5Dopen(group, tname.c_str(), H5P_DEFAULT);
	hid_t sid = H5Screate_simple(1, dims, NULL);
	hid_t aid = H5Acreate(table, name.c_str(), type, sid, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(aid, type, &value);
	H5Aclose(aid);
	H5Sclose(sid);
	H5Dclose(table);
}

#endif
