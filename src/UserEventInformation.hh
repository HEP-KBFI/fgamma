#ifndef UserEventInformation_h
#define UserEventInformation_h

#include <G4VUserEventInformation.hh>

struct UserEventInformation : public G4VUserEventInformation
{
	int pid;
	double E, KE, incidence;

	void Print() const;
};

#endif


