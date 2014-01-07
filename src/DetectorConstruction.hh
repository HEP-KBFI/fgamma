#ifndef DetectorConstruction_h
#define DetectorConstruction_h

#include <G4VUserDetectorConstruction.hh>

class G4LogicalVolume;
class G4Material;

class DetectorConstruction : public G4VUserDetectorConstruction {
	public:
		// fractions == 0 implies vacuum
		DetectorConstruction(G4double radius = 10e3*km, G4double pressurefactor = 1);

		// methods from base class 
		virtual G4VPhysicalVolume* Construct();

	private:
		G4double fRadius, fPressureFactor;
		G4LogicalVolume* fWorldVolume;

		static G4Material * getVacuumMaterial();
		static G4Material * getSpaceAir(G4double pressurefactor = 1);
};
#endif

