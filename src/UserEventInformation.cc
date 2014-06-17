#include "UserEventInformation.hh"

#include <globals.hh>

void UserEventInformation::Print() const
{
	G4cout << "Eventinfo: pid=" << pid << ", E=" << E << ", KE=" << KE << ", inc=" << incidence << G4endl;
}
