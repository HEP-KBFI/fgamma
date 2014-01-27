#include "DetectorConstruction.hh"

#include <G4Orb.hh>
#include <G4Sphere.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Material.hh>
#include <G4NistManager.hh>

using namespace CLHEP;

DetectorConstruction::DetectorConstruction(G4double radius)
	: G4VUserDetectorConstruction(),fRadius(radius) {}

G4VPhysicalVolume* DetectorConstruction::Construct() {
	G4Material* fMaterial = getSpaceAir(4.339e-10*g/cm3, 182.1*kelvin);
	G4cout << "==================  Material  ==================" << G4endl;
	G4cout << (*fMaterial) << G4endl;

	// World
	//G4CSGSolid* sWorld = new G4Orb("World", fRadius);
	G4CSGSolid* sWorld = new G4Sphere("World", 6371*km, (6371+565)*km, 0, 2*pi, 0, pi);

	// Logical World Volume. Arguments: // shape, material, name
	fWorldVolume = new G4LogicalVolume(sWorld, getVacuumMaterial(), "World");

	G4VPhysicalVolume* pWorld = new G4PVPlacement(
		0,                      // no rotation
		G4ThreeVector(),        // at (0,0,0)
		fWorldVolume,           // logical volume
		"World",                // name
		0,                      // no mother volume
		false,                  // no boolean operation
		0                       // copy number
	);
	
	// World
	//G4CSGSolid* sWorld = new G4Orb("World", fRadius);
	G4CSGSolid* sAtm = new G4Sphere("Atmosphere", 6371*km, (6371+100)*km, 0, 2*pi, 0, pi);

	// Logical World Volume. Arguments: // shape, material, name
	G4LogicalVolume* atmVolume = new G4LogicalVolume(sAtm, fMaterial, "Atmosphere");

	//G4VPhysicalVolume* pAtm = new G4PVPlacement(
	new G4PVPlacement(
		0,                      // no rotation
		G4ThreeVector(),        // at (0,0,0)
		atmVolume,              // logical volume
		"Atmosphere",           // name
		fWorldVolume,           // mother volume
		false,                  // no boolean operation
		0                       // copy number
	);

	return pWorld; // always return the root volume
}

G4Material * DetectorConstruction::getSpaceAir(G4double density, G4double temp) {
	/*G4double temp=700.*kelvin;
	G4double pressure=1.*atmosphere;
	return G4NistManager::Instance()->ConstructNewGasMaterial("SpaceAir", "G4_AIR", temp, pressurefactor*pressure);*/
	G4Material* bmat = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");
	G4double pres = STP_Pressure*density*temp/(bmat->GetDensity()*STP_Temperature);
	return new G4Material("SpaceAir", density, bmat, kStateGas, temp, pres);
}

G4Material * DetectorConstruction::getVacuumMaterial() {
	return G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");
}
