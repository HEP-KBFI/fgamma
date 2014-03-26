#include <iostream>
#include <fstream>

using namespace std;

template<class T>
T read_urandom()
{
	union {
		T value;
		char cs[sizeof(T)];
	} u;

	std::ifstream rfin("/dev/urandom");
	rfin.read(u.cs, sizeof(u.cs));
	rfin.close();

	return u.value;
}

int main(int argc, char * argv[])
{
	cout << read_urandom<unsigned int>() << endl;
}
