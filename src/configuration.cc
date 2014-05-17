#include "configuration.hh"

#include <iostream>
#include <boost/algorithm/string.hpp>

#include <CLHEP/Units/SystemOfUnits.h>

using namespace std;

eventconf eventconf::parse_string(const std::string &evstr)
{
	eventconf ret;

	// all except energy have default values:
	ret.pid = 2212; // proton
	ret.aoi = 0.0;  // perpendicular to the atmosphere's surface
	ret.n = 1;      // run one event with these parameters

	bool isset_E = false;

	vector<string> tokens;
	boost::split(tokens, evstr, boost::is_any_of(","));

	for(string & token : tokens) {
		vector<string> expr;
		boost::split(expr, token, boost::is_any_of("="));
		if(expr.size() != 2) {
			throw parse_error("bad token - not in the form 'name=value'", evstr, token);
		}

		boost::trim(expr[0]);

		if(expr[0] == "E" || expr[0] == "e") {
			ret.E = atof(expr[1].c_str()) * CLHEP::GeV;
			if(ret.E < 0) {
				throw parse_error("E cannot be negative", evstr, token);
			}
			isset_E = true;
		} else if(expr[0] == "aoi") {
			ret.aoi = atof(expr[1].c_str());
			if(ret.aoi < 0 || ret.aoi > 1) {
				throw parse_error("aoi has to be between 0 and 1", evstr, token);
			}
		} else if(expr[0] == "pid") {
			ret.pid = atoi(expr[1].c_str());
		} else if(expr[0] == "n") {
			ret.n = atoi(expr[1].c_str());
			if(ret.n <= 0) {
				throw parse_error("n has to be greater than zero", evstr, token);
			}
		} else {
			throw parse_error("bad token - invalid name", evstr, token);
		}
	}

	if(!isset_E) {
		throw parse_error("energy value (E) not set", evstr);
	}

	return ret;
}

std::ostream& operator<< (std::ostream &out, const eventconf &ec)
{
	return out << "eventconf(pid=" << ec.pid << ", E[GeV]=" << ec.E/CLHEP::GeV << ", aoi=" << ec.aoi << ", n=" << ec.n << ")";
}

// Implementation of exception eventconf::parse_error
eventconf::parse_error::parse_error(const string & what_, const std::string & evstr_)
: msg(what_), evstr(evstr_), token("<no token>") {}
eventconf::parse_error::parse_error(const string & what_, const std::string & evstr_, const std::string & token_)
: msg(what_), evstr(evstr_), token(token_) {}
const char * eventconf::parse_error::what() const throw() {return msg.c_str();}
