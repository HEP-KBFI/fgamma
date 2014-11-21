#----------------------------------------------------------------------------
# Setup the project
# ---
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(FGAMMA)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")

#----------------------------------------------------------------------------
# Use c++11 for all C++ files.
# ---
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
# We do not use Geant4_USE_FILE for settign up directories, compile flags etc
# because it overwrites the variables, instead of appending to them. We set up
# necessary stuff manually, based on the aforementioned file.
#
# Also, the Geant4_CXX_FLAGS gets prepended to CMAKE_CXX_FLAGS because Geant4
# sets the -std=c++98 flag, but be want to use -std=c++11, which gets added to
# CMAKE_CXX_FLAGS above.
# ---
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
	find_package(Geant4 REQUIRED ui_all vis_all)
else()
	find_package(Geant4 REQUIRED)
endif()

add_definitions(${Geant4_DEFINITIONS})
include_directories(${Geant4_INCLUDE_DIRS})
set(CMAKE_CXX_FLAGS                "${Geant4_CXX_FLAGS} ${CMAKE_CXX_FLAGS}") # prepending for a reason, see comment above
set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} ${Geant4_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL} ${Geant4_CXX_FLAGS_MINSIZEREL}")
set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} ${Geant4_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${Geant4_CXX_FLAGS_RELWITHDEBINFO}")
set(CMAKE_EXE_LINKER_FLAGS         "${CMAKE_EXE_LINKER_FLAGS} ${Geant4_EXE_LINKER_FLAGS}")

#----------------------------------------------------------------------------
# Load HDF5 library
# ---
find_package(HDF5 REQUIRED C HL)
include_directories(${HDF5_INCLUDE_DIR})

#----------------------------------------------------------------------------
# Load the Boost library (headers required by yaml-cpp)
# ---
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

#----------------------------------------------------------------------------
# Load the yaml-cpp library
# ---
find_package(YamlCpp REQUIRED)
include_directories(${YAMLCPP_INCLUDE_DIR})

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
# ---
set(source_files
	configuration.cc
	DetectorConstruction.cc
	PrimaryGeneratorAction.cc
	UserEventInformation.cc
	UserActionManager.cc
	Timer.cc
	HDFTable.cc
	TrackingLog.cc
)

message(" > Sources...")
set(sources)
foreach(source ${source_files})
	set(source_full "${PROJECT_SOURCE_DIR}/src/${source}")
	list(APPEND sources ${source_full})
	message(" > - ${source_full}")
endforeach(source)

configure_file(${PROJECT_SOURCE_DIR}/vis.mac ${PROJECT_BINARY_DIR}/vis.mac COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/models/example.yml ${PROJECT_BINARY_DIR}/model.yml COPYONLY)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
# ---
set(LIBRARIES ${Geant4_LIBRARIES} ${YAMLCPP_LIBRARY} ${HDF5_LIBRARIES})

add_executable(fgamma src/main.cc ${sources})
target_link_libraries(fgamma ${LIBRARIES})

# mergeruns tool
add_executable(mergeruns tools/mergeruns.cc)
target_link_libraries(mergeruns ${HDF5_LIBRARIES})

# analyzer tool
add_executable(analyzer tools/analyzer.cc)
target_link_libraries(analyzer ${HDF5_LIBRARIES})

# loadmodel test
add_executable(test-eventconf tests/eventconf.cc src/configuration.cc)
target_link_libraries(test-eventconf)

add_executable(test-loadmodel tests/loadmodel.cc src/DetectorConstruction.cc)
target_link_libraries(test-loadmodel ${Geant4_LIBRARIES} ${YAMLCPP_LIBRARY})

add_executable(test-hdftable tests/hdftable.cc src/HDFTable.cc)
target_link_libraries(test-hdftable ${HDF5_LIBRARIES})
