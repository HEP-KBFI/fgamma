#include "../src/configuration.hh"

#include <iostream>

using namespace std;

int main(int argc, char * argv[])
{
	for(int i=1; i<argc; i++) {
		cout << "parsing: " << argv[i] << endl;
		try {
			eventconf ec = eventconf::parse_string(argv[i]);
			cout << "Success: " << ec << endl;
		} catch(eventconf::parse_error &e) {
			cout << "Error(eventconf::parse_error): " << e.what() << endl;
			cout << "  evstr: `" << e.evstr << "`" << endl;
			cout << "  token: `" << e.token << "`" << endl;
		}
	}
}
