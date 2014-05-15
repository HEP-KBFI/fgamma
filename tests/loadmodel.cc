#include "../src/DetectorConstruction.hh"
#include <G4NistManager.hh>
#include <iostream>
using namespace std;

int main() {
	G4Element * e = G4NistManager::Instance()->FindOrBuildElement("O");
	cout << *e << endl;

	G4IsotopeVector * gv = (*e).GetIsotopeVector();
	for(G4IsotopeVector::iterator it = gv->begin(); it!=gv->end(); ++it) {
		G4Isotope * iso = *it;
		cout << iso << endl;
	}

	cout << "---------------------- DetectorConstruction ----------------------" << endl;
	DetectorConstruction("model.yml");
}
