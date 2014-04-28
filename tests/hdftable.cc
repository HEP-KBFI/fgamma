#include "../src/HDFTable.hh"

#include <iostream>
#include <cmath>

using namespace std;

int main(int argc, char* argv[])
{
	vector<HDFTableField> fields;
	fields.push_back(HDFTableField(H5T_NATIVE_INT, "idx"));
	fields.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "x"));
	fields.push_back(HDFTableField(H5T_NATIVE_DOUBLE, "y"));

	hid_t file = H5Fcreate("tabletest.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	hid_t group = H5Gcreate(file, "subgroup", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	HDFTable table(group, "table-name", fields, 1337);

	int &idx = table.bind<int>("idx");
	double &x = table.bind<double>("x");
	double &y = table.bind<double>("y");

	for(int i=0; i<10000; i++) {
		idx = 100000 + i;
		x = 0.5*i;
		y = 50*sqrt(i);
		table.write();
	}
	table.flush();

	return 0;
}
