#include "DetectorConstruction.hh"

#include "G4Orb.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"

struct ElementFraction {
	G4String name;
	G4double fraction;
};

G4double atm_density     = 1.225*kg/m3;
G4double atm_pressure    = 1*atmosphere;
G4double atm_temperature = 300*kelvin;
ElementFraction atm_fractions[] = {
	{"G4_N", 70.00},
	{"G4_O", 20.00},
	{"G4_C", 10.00}
};

DetectorConstruction::DetectorConstruction(G4double radius)
	: G4VUserDetectorConstruction(),fRadius(radius) {}

G4VPhysicalVolume* DetectorConstruction::Construct() {
	int fractions = 3;
	G4Material* material = fractions==0 ? getVacuumMaterial() : getAtmosphereMaterial();
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

G4Material * DetectorConstruction::getAtmosphereMaterial(unsigned int Nfractions) {
	// Define materials via NIST manager
	G4NistManager* nm = G4NistManager::Instance();
	
	// Calculate the total fraction. Used for normalization.
	double totalfraction = 0.0;
	for(unsigned int i=0; i < Nfractions; i++) {
		totalfraction += atm_fractions[i].fraction;
	}
	
	G4Material* solarmaterial = new G4Material(
		"Air", atm_density, // name, density
		Nfractions, kStateGas, // ncomponents, state
		atm_temperature, atm_pressure // temperature, pressure
	);
	for(size_t i=0; i < Nfractions; i++) {
		solarmaterial->AddMaterial(
			nm->FindOrBuildMaterial(atm_fractions[i].name),
			atm_fractions[i].fraction/totalfraction
		);
	}
	
	G4cout << "Atmosphere material - total fraction: " << totalfraction << G4endl;
	
	return solarmaterial;
}

G4Material * DetectorConstruction::getVacuumMaterial() {
	G4double vac_density     = universe_mean_density; //from PhysicalConstants.h
	G4double vac_pressure    = 1.e-19*pascal;
	G4double vac_temperature = 0.1*kelvin;
	return new G4Material("Vacuum", 1., 1.01*g/mole,
	                       vac_density, kStateGas, vac_temperature,
	                       vac_pressure);
}
