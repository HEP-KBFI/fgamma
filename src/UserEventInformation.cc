#include "UserEventInformation.hh"

#include <globals.hh>
#include <CLHEP/Units/SystemOfUnits.h>

void UserEventInformation::Print() const
{
	G4cout << "Eventinfo: pid=" << pid << ", E=" << E << ", KE=" << KE << ", inc=" << incidence << G4endl;
}

std::ostream& operator<< (std::ostream &out, const UserEventInformation &eventinfo)
{
	return out << "("
	           << "pid=" << eventinfo.pid
	           << ", E[GeV]=" << eventinfo.E/CLHEP::GeV
	           << ", KE[GeV]=" << eventinfo.KE/CLHEP::GeV
	           << ", aoi=" << eventinfo.incidence
	           << ")";
}
