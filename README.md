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
yaml-cpp and `BOOST_ROOT` for Boost). Since the FindBoost.cmake module seems to
be a bit crappy, a `Boost_NO_BOOST_CMAKE=ON` is also probably necessary if there
is an older Boost installed on the system as well (otherwise the module may find
a wrong version).

The `FindHDF5.cmake` script is a bit stupid and you have to specify the
`HDF5_ROOT` as an environment variable (e.g. `HDF5_ROOT=path cmake ...`).

In order to have a debug build you can use the standard `CMAKE_BUILD_TYPE`.


**Usage**

	$ ./fgamma --help
	Usage: fgamma [OPTION...] [EVENT...]
	Simulation of gamma-rays produced in the atmosphere by cosmic rays.

	 General options:
	  -f, --eventfile=FILE       file with event parameters (each line with
								 eventconf syntax)
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

The parameters of the inital particles in the simulation can be supplied on the
command line, in a file or both. All non-option arguments on the command line
are interpreted as event paramters. The `-f` option can be used to define a file
where additional events will be read from (one event per line).

Each event parameter (argument or line) is a comma-separated list of values
in the form `name=value`. The possible parameters are:

  * **E** or **e** -- energy of the incoming particle (in GeV)
  * **aoi** -- angle of incidence (real number between 0 and 1,
    where 0 corresponds to 0 degrees and 1 to 90 degrees; default: 0)
  * **pid** -- PDG ID of the incoming particle (default: 2212, proton)
  * **n** -- number of events with these parameters (default: 1)

All parameters except energy have default values and can therefore be omitted.

An example: `./fgamma E=100 E=10,n=25 E=10,pid=11,aoi=0.5`  --
1 event with 100 GeV proton, 25 events with a 10 GeV proton and an event with
a 10 GeV electron coming in at a 45 degree angle.
