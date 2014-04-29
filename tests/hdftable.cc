#include "../src/HDFTable.hh"

#include <iostream>
#include <cstring>
#include <cmath>

using namespace std;

const size_t NAME_STRLEN = 16;

int main(int argc, char* argv[])
{
	vector<HDFTableField> fields;
	fields.push_back(HDFTableField(H5T_NATIVE_INT, "idx"));
	fields.push_back(HDFTableField(create_hdf5_string(NAME_STRLEN), "name"));
	fields.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "x"));
	fields.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "y"));

	hid_t file = H5Fcreate("tabletest.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	hid_t group = H5Gcreate(file, "subgroup", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	HDFTable table(group, "table-name", fields, 1337);

	int &idx = table.bind<int>("idx");
	double &x = table.bind<double>("x");
	double &y = table.bind<double>("y");
	char (&name)[] = table.bind<char[]>("name");

	string_to_cstr(string("  : qwerty;"), name, NAME_STRLEN);

	for(int i=0; i<10000; i++) {
		idx = 100000 + i;
		x = 0.5*i;
		y = 50*sqrt(i);
		name[0] = 0x40 + i%50;
		table.write();
	}
	table.flush();

	return 0;
}
