fgamma
======

**Requires**

* [Geant4](http://geant4.cern.ch/)
* [GNU Scientific Library (GSL)](http://www.gnu.org/software/gsl/)

**Usage**

	$ ./fgamma --help
	Usage: fgamma [OPTION...] <ENERGY in GeV>
	Simulation of gamma-rays produced in the atmosphere by cosmic rays.

	 General options:
	  -n, --runs=N               run the simulation N times
		  --seed=SEED            set the seed for the random generators; if this is
								 not specified, time(0) is used)

	 Options for tweaking the physics:
	  -r, --radius=R             set the radius of the sphere in km (default is 10
								 km)

	 Other:
	  -?, --help                 Give this help list
		  --usage                Give a short usage message
	  -V, --version              Print program version

	Mandatory or optional arguments to long options are also mandatory or optional
	for any corresponding short options.
