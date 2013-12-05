#include "DetectorConstruction.hh"

#include <G4Orb.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Material.hh>
#include <G4NistManager.hh>

DetectorConstruction::DetectorConstruction(G4double radius)
	: G4VUserDetectorConstruction(),fRadius(radius) {}

G4VPhysicalVolume* DetectorConstruction::Construct() {
	G4Material* material = getSpaceAir();
	G4cout << "==================  Material  ==================" << G4endl;
	G4cout << (*material) << G4endl;

	// World
	G4CSGSolid* sWorld = new G4Orb("World", fRadius);

	// Logical World Volume. Arguments: // shape, material, name
	fWorldVolume = new G4LogicalVolume(sWorld, material, "World");

	G4VPhysicalVolume* pWorld = new G4PVPlacement(
		0,                      // no rotation
		G4ThreeVector(),        // at (0,0,0)
		fWorldVolume,           // logical volume
		"World",                // name
		0,                      // no mother volume
		false,                  // no boolean operation
		0                       // copy number
	);

	return pWorld; // always return the root volume
}

G4Material * DetectorConstruction::getSpaceAir(G4double temp, G4double pressure) {
	return G4NistManager::Instance()->ConstructNewGasMaterial("SpaceAir", "G4_AIR", temp, pressure);
}

G4Material * DetectorConstruction::getVacuumMaterial() {
	G4double vac_density     = universe_mean_density; //from PhysicalConstants.h
	G4double vac_pressure    = 1.e-19*pascal;
	G4double vac_temperature = 0.1*kelvin;
	return new G4Material("Vacuum", 1., 1.01*g/mole,
	                       vac_density, kStateGas, vac_temperature,
	                       vac_pressure);
}