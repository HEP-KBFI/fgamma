fgamma
======

**Requires**

* [Geant4](http://geant4.cern.ch/)
* [yaml-cpp](https://code.google.com/p/yaml-cpp/) (version >0.5)

**Usage**

	$ ./fgamma --help
	Usage: fgamma [OPTION...] ENERGY(GeV) INCIDENCE(pi/2)
	Simulation of gamma-rays produced in the atmosphere by cosmic rays.

	 General options:
	  -n, --runs=N               run the simulation N times
	  -o, --prefix=PREFIX        set the prefix of the output files
		  --seed=SEED            set the seed for the random generators; if this is
								 not specified, time(0) is used)
		  --tracks               store tracks in tracks.txt
	  -v, --verbosity=LEVEL      set the verbosity level (0 - minimal, 1 - a bit
								 (default), 2 - a lot)
		  --vis                  open the GUI instead of running the simulation

	 Options for tweaking the physics:
	  -m, --model=MODELFILE      set the YAML file used to model the geometry
								 (default: model.yml)

	 Other:
	  -?, --help                 Give this help list
		  --usage                Give a short usage message
	  -V, --version              Print program version

	Mandatory or optional arguments to long options are also mandatory or optional
	for any corresponding short options.

