#----------------------------------------------------------------------------
# Setup the project
# ---
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(FGAMMA)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
# ---
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
	find_package(Geant4 REQUIRED ui_all vis_all)
else()
	find_package(Geant4 REQUIRED)
endif()

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
# Setup include directory for this project
# ---
include(${Geant4_USE_FILE})

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
# Use c++11 for all C++ files.
# ---
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
# ---
set(LIBRARIES ${Geant4_LIBRARIES} ${YAMLCPP_LIBRARY} ${HDF5_LIBRARIES})

add_executable(fgamma src/main.cc ${sources})
target_link_libraries(fgamma ${LIBRARIES})

# loadmodel test
add_executable(test-eventconf tests/eventconf.cc src/configuration.cc)
target_link_libraries(test-eventconf)

add_executable(test-loadmodel tests/loadmodel.cc src/DetectorConstruction.cc)
target_link_libraries(test-loadmodel ${Geant4_LIBRARIES} ${YAMLCPP_LIBRARY})

add_executable(test-hdftable tests/hdftable.cc src/HDFTable.cc)
target_link_libraries(test-hdftable ${HDF5_LIBRARIES})
