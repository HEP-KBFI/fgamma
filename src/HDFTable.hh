#ifndef HDFTable_h
#define HDFTable_h

#include <hdf5.h>

#include <vector>
#include <string>
#include <map>
#include <iostream>

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
		bool table_exists;

	std::map<std::string, size_t> offset_map;
	hsize_t nfields, type_size;
	const char ** field_names;
	hid_t * field_types;
	size_t * field_offset;
	size_t * field_sizes;

	unsigned char * data;
	unsigned char * buffer;
	size_t buffer_size, inbuffer;

	public:
		HDFTable(const hid_t h5group, const std::string &tablename, const std::vector<HDFTableField> &fields, size_t buffered_fields = 1);
		template<class T> T& bind(const std::string & name);
		void write();
		void flush();

	private:
		HDFTable(const HDFTable&);
		HDFTable& operator=(HDFTable);
		void writeBuffer();
};

template<class T>
T& HDFTable::bind(const std::string & name)
{
	if(offset_map.count(name) != 1) {
		throw(999);
	}
	return *((T*)(&data[offset_map[name]]));
}

#endif
