#ifndef DetectorConstruction_h
#define DetectorConstruction_h

#include <CLHEP/Units/SystemOfUnits.h>
#include <G4VUserDetectorConstruction.hh>

class G4LogicalVolume;
class G4Material;

class DetectorConstruction : public G4VUserDetectorConstruction {
	public:
		G4Material * fMaterial;
	
		// fractions == 0 implies vacuum
		DetectorConstruction(G4double radius = 1e3*CLHEP::km, G4double pressurefactor = 1);

		// methods from base class 
		virtual G4VPhysicalVolume* Construct();

	private:
		G4double fRadius, fPressureFactor;
		G4LogicalVolume* fWorldVolume;

		static G4Material * getVacuumMaterial();
		static G4Material * getSpaceAir(G4double pressurefactor = 1);
};
#endif

