#include "DetectorConstruction.hh"

#include <G4Orb.hh>
#include <G4Sphere.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Material.hh>
#include <G4NistManager.hh>

#include <yaml-cpp/yaml.h>
#include <sstream>
#include <utility>

using namespace CLHEP;

DetectorConstruction::DetectorConstruction(G4String modelfile, unsigned int verbosity)
: mFromCenter(true), mStartRadius(0.0), mTotalThickness(0.0) {
	if(verbosity>0){G4cout << "Loading model from: " << modelfile << G4endl;}
	YAML::Node mdl = YAML::LoadFile(modelfile);

	if(verbosity>0){G4cout << "Model name: " << mdl["name"] << G4endl;}

	if(mdl["startat"]) {
		mStartRadius = mdl["startat"].as<double>()*km;
		mFromCenter = false;
		if(verbosity>1){G4cout << "Starts at: " << mStartRadius/km << " [km]" << G4endl;}
	} else {
		if(verbosity>1){G4cout << "Starts from the center." << G4endl;}
	}

	int layerid=0;
	for(YAML::const_iterator it=mdl["layers"].begin();it!=mdl["layers"].end();++it) {
		typedef std::pair<G4Element*, double> component;
		YAML::Node ly = *it;
		double totalDensity = 0.0, totalPressure = 0.0;
		std::vector<component> cs;
		layer cly;

		std::ostringstream layername_stream;
		layername_stream << "layer_" << layerid;
		cly.name = layername_stream.str();

		double temperature = ly["temperature"].as<double>()*kelvin;
		cly.thickness = ly["thickness"].as<double>() * km;
		if(verbosity>1) {
			G4cout << "> Layer: " << cly.name << G4endl;
			G4cout << "  layerid = " << layerid << G4endl;
			G4cout << "  thickness = " << cly.thickness/km << " [km]" << G4endl;
			G4cout << "  temperature = " << temperature/kelvin << " [K]" << G4endl;
		}

		for(YAML::const_iterator itc=ly["components"].begin();itc!=ly["components"].end();++itc) {
			// Find the corresponding G4Element
			std::string element_symbol = (*itc)["element"].as<std::string>();
			G4Element * element = G4NistManager::Instance()->FindOrBuildElement(element_symbol);
			double density = NAN;

			if((*itc)["isotopes"]) {
				// if it has a list of isotopes, they define the element
				typedef std::pair<G4Isotope*, double> isotope;
				std::vector<isotope> isotopes;
				density = 0.0;
				// we'll use the original element as a library for isotopes
				G4IsotopeVector * isolib = element->GetIsotopeVector();

				for(YAML::const_iterator itiso=(*itc)["isotopes"].begin();itiso!=(*itc)["isotopes"].end();++itiso) {
					int A = (*itiso)["A"].as<int>();
					double n = (*itiso)["number_density"].as<double>()/cm3;
					G4Isotope * thisisotope;

					bool found = false;
					for(G4IsotopeVector::iterator itlibiso=isolib->begin();itlibiso!=isolib->end();++itlibiso) {
						if((*itlibiso)->GetN() == A) {
							thisisotope = *itlibiso;
							found = true;
						}
					}

					if(found) {
						isotopes.push_back(isotope(thisisotope, n));
						density = thisisotope->GetA()*n/Avogadro;
					} else {
						G4cout << "WARNING: Did not find isotope: " << *itiso << G4endl;
					}
				}

				std::ostringstream element_name;
				element_name << element_symbol << "_layer_" << layerid;
				element = new G4Element(element_name.str(), element_symbol, isotopes.size());
				for(std::vector<isotope>::iterator itiso=isotopes.begin();itiso!=isotopes.end();++itiso) {
					element->AddIsotope(itiso->first, itiso->second);
				}
				if(verbosity>1){G4cout << element << G4endl;}
			} else if((*itc)["number_density"]) {
				// if not, try to find a number density
				double n = (*itc)["number_density"].as<double>()/cm3;
				density = element->GetA()*n/Avogadro;
			} else {
				// otherwise, assume that density is specified and hope for the best
				density = (*itc)["density"].as<double>()*g/cm3;
			}

			component c(element, density);
			totalPressure += Avogadro*k_Boltzmann*temperature*c.second/c.first->GetA();
			totalDensity += c.second;
			cs.push_back(component(element, density));
		}

		if(verbosity>1) {
			G4cout << "  density = " << totalDensity/(g/cm3) << " [g/cm3]" << G4endl;
			G4cout << "  pressure = " << totalPressure/atmosphere << " [atm]" << G4endl;
		}

		int ncomponents = cs.size();
		cly.material = new G4Material(cly.name, totalDensity, ncomponents, kStateGas, temperature, totalPressure);
		for(std::vector<component>::iterator itc=cs.begin();itc!=cs.end();++itc) {
			double fraction = itc->second/totalDensity;
			if(verbosity>1){G4cout << "  - " << itc->first->GetName() << ": " << fraction << " (" << itc->second/(g/cm3) << " [g/cm3])" << G4endl;}
			cly.material->AddElement(itc->first, fraction);
		}

		mTotalThickness += cly.thickness;
		layers.push_back(cly);
		layerid++;
	}
}

G4VPhysicalVolume* DetectorConstruction::Construct() {
	// World
	G4CSGSolid* sWorld;
	if(mFromCenter) {
		sWorld = new G4Orb("World", mTotalThickness);
	} else {
		sWorld = new G4Sphere("World", mStartRadius, mStartRadius+mTotalThickness, 0, 2*pi, 0, pi);
	}

	// Logical World Volume. Arguments: // shape, material, name
	G4Material* galacticMaterial = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");
	fWorldVolume = new G4LogicalVolume(sWorld, galacticMaterial, "World");

	G4VPhysicalVolume* pWorld = new G4PVPlacement(
		0,                      // no rotation
		G4ThreeVector(),        // at (0,0,0)
		fWorldVolume,           // logical volume
		"World",                // name
		0,                      // no mother volume
		false,                  // no boolean operation
		0                       // copy number
	);

	// Layers
	bool firstOrb = mFromCenter;
	double nextStartRadius = mStartRadius;
	for(std::vector<layer>::iterator it=layers.begin();it!=layers.end();++it) {
		layer& ly = *it;

		// Calculate the geometry parameters
		ly.dStartRadius = nextStartRadius;
		ly.dEndRadius = ly.dStartRadius + ly.thickness;
		nextStartRadius = ly.dEndRadius;

		// Create the proper solid: usually a shell, but a sphere if
		// the geometry starts from the center
		if(firstOrb) {
			ly.dSolid = new G4Orb(ly.name+"_solid", ly.dEndRadius);
			firstOrb = false;
		} else {
			ly.dSolid = new G4Sphere(ly.name+"_solid", ly.dStartRadius, ly.dEndRadius, 0, 2*pi, 0, pi);
		}

		// Logical World Volume. Arguments: // shape, material, name
		ly.dLogicalVolume = new G4LogicalVolume(ly.dSolid, ly.material, ly.name+"_logvol");

		//G4VPhysicalVolume* pAtm = new G4PVPlacement(
		new G4PVPlacement(
			0,                      // no rotation
			G4ThreeVector(),        // at (0,0,0)
			ly.dLogicalVolume,      // logical volume
			ly.name+"_placement",   // name
			fWorldVolume,           // mother volume
			false,                  // no boolean operation
			0                       // copy number
		);
	}

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

double DetectorConstruction::getWorldRadius() {
	return mStartRadius+mTotalThickness;
}
