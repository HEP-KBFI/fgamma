#ifndef configuration_h
#define configuration_h

#include <cstddef>
#include <exception>
#include <string>

struct eventconf
{
	int pid;
	double E, aoi;
	size_t n;

	static eventconf parse_string(const std::string &evstr);

	struct parse_error : std::exception
	{
		std::string msg, evstr, token;
		parse_error(const std::string & what, const std::string & evstr);
		parse_error(const std::string & what, const std::string & evstr, const std::string & token);
		virtual ~parse_error() throw() {}
		virtual const char * what() const throw();
	};
};

std::ostream& operator<< (std::ostream &out, const eventconf &ec);

#endif
