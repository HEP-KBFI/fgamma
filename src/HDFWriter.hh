#ifndef HDFWriter_h
#define HDFWriter_h

#include <hdf5.h>
#include <vector>
#include <string>

class HDFWriter
{
	hid_t file;
	bool table_exists;

	public:
		struct particle
		{
			unsigned int eventid;
			char name[16];
			int pid;
			double m;
			struct {
				double KE;
				double x,y,z;
				double px,py,pz;
			} vtx;
			struct {
				double KE;
				double x,y,z;
				double px,py,pz;
			} boundary;
		};

		HDFWriter(const std::string fname);
		~HDFWriter();

		void writeEvent(int evid, double E, double incidence, std::vector<particle> ps);

	private:
		HDFWriter(const HDFWriter&);
		HDFWriter& operator=(HDFWriter);

		static const hsize_t nfields;
		static const char * field_names[];
		static const hid_t field_types[];
		static const size_t field_offset[];
		static const size_t field_sizes[];
};

void string_to_cstr(const std::string &src, char dst[], size_t target_size);

#endif
