#ifndef UserEventInformation_h
#define UserEventInformation_h

#include <G4VUserEventInformation.hh>
#include <ostream>

struct UserEventInformation : public G4VUserEventInformation
{
	int pid;
	double E, KE, incidence;

	void Print() const;
};

std::ostream& operator<< (std::ostream &out, const UserEventInformation &eventinfo);

#endif
