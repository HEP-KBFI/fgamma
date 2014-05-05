fgamma
======

**Requires**

* [Geant4](http://geant4.cern.ch/)
* [yaml-cpp](https://code.google.com/p/yaml-cpp/) (version >0.5)
* [Boost](http://www.boost.org/) (headers required by yaml-cpp)
* [HDF5 C library](http://www.hdfgroup.org/HDF5/)


**Compiling**

The software uses [CMake](http://www.cmake.org/) to manage the building process.

For Geant4 you need to source the Geant4 environment beforehand. This is necessary
for running the simulation as well, because Geant4 uses the environment variables
to find the data files.

If the other necessary libraries are installed at custom locations, you can
specify the paths when configuring the build using `-D` flags (`YAMLCPP_DIR` for
yaml-cpp and `BOOST_ROOT` for Boost).

The `FindHDF5.cmake` script is a bit stupid and you have to specify the
`HDF5_ROOT` as an environment variable (e.g. `HDF5_ROOT=path cmake ...`).

In order to have a debug build you can use the standard `CMAKE_BUILD_TYPE`.


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
		  --cutoff=CUT           define an energy cutoff (in GeVs)
	  -m, --model=MODELFILE      set the YAML file used to model the geometry
								 (default: model.yml)

	 Other:
	  -?, --help                 Give this help list
		  --usage                Give a short usage message
	  -V, --version              Print program version

	Mandatory or optional arguments to long options are also mandatory or optional
	for any corresponding short options.

