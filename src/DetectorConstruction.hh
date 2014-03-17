#ifndef DetectorConstruction_h
#define DetectorConstruction_h

#include <G4VUserDetectorConstruction.hh>

class G4LogicalVolume;
class G4Material;
class G4CSGSolid;

class DetectorConstruction : public G4VUserDetectorConstruction {
	public:
		DetectorConstruction(G4String modelfile, unsigned int verbosity=1);

		// methods from base class
		virtual G4VPhysicalVolume* Construct();
		double getWorldRadius();

	private:
		struct layer {
			double thickness;
			G4Material * material;
			G4String name;

			G4CSGSolid * dSolid;
			G4LogicalVolume * dLogicalVolume;
			G4double dStartRadius, dEndRadius;
		};

		bool mFromCenter;
		double mStartRadius, mTotalThickness;
		std::vector<layer> layers;

		G4LogicalVolume* fWorldVolume;

		static G4Material * getVacuumMaterial();
		static G4Material * getSpaceAir(G4double density, G4double temp);
};
#endif

