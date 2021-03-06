# ======================================================================
#  artdaq-demo main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/artdaq-demo/ups/setup_for_development
#  buildtool
# ======================================================================


# use cmake 2.8 or later
cmake_minimum_required (VERSION 2.8)


project(artdaq-demo)
# cetbuildtools contains our cmake modules
SET ( CETBUILDTOOLS_VERSION $ENV{CETBUILDTOOLS_VERSION} )
IF (NOT CETBUILDTOOLS_VERSION)
    MESSAGE (FATAL_ERROR "ERROR: setup cetbuildtools to get the cmake modules")
ENDIF()

if( DEFINED ENV{CANVAS_DIR} )
set(CMAKE_MODULE_PATH
  $ENV{ART_DIR}/Modules
  $ENV{CANVAS_DIR}/Modules
	$ENV{CETBUILDTOOLS_DIR}/Modules
	${CMAKE_MODULE_PATH})
else()
set(CMAKE_MODULE_PATH
  $ENV{ART_DIR}/Modules
	$ENV{CETBUILDTOOLS_DIR}/Modules
	${CMAKE_MODULE_PATH})
endif()


include(CetCMakeEnv)
cet_cmake_env()

cet_set_compiler_flags(DIAGS VIGILANT
 WERROR
 NO_UNDEFINED
 EXTRA_FLAGS -pedantic
 )

string(TOUPPER ${CMAKE_BUILD_TYPE} BTYPE_UC )
#if( ${BTYPE_UC} MATCHES "DEBUG" )
#  cet_add_compiler_flags(-fsanitize=address)
#endif()

cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
find_ups_product( cetbuildtools v3_03_00 )
find_ups_product( TRACE v3_03_00 )
find_ups_product( artdaq v0_04_02 )
find_ups_product( artdaq_utilities v1_00_00)
find_ups_product( artdaq_core v1_00_00 )
find_ups_product( artdaq_core_demo v0_00_01 )
find_ups_product( art v1_06_00 )
find_ups_product( fhiclcpp v2_16_00 )
find_ups_product( messagefacility v1_10_00 )
#find_ups_product( cpp0x v1_03_00 )
find_ups_product( cetlib v1_03_00 )
find_ups_root( v5_34_00 )
find_ups_boost( v1_50_0 filesystem
                        program_options
                        unit_test_framework
 )

INCLUDE_DIRECTORIES (/home/echurch/artdaq2/artdaq-demo/artdaq-demo/Generators/flx)
## below directories provide, in order: libFlxCard.so, libregmap2.so (mine, not dot-a version), libcmem_rcc.so, libio_rcc.so, respectively, ...., then the artdaq stuff. 		    
LINK_DIRECTORIES(/home/echurch/felix/software.r3222/flxcard/build /home/echurch/felix/software.r3222/flxcard/build/regmap /home/echurch/felix/software.r3222/drivers_rcc/lib64 /home/echurch/artdaq2/artdaq-demo/artdaq-demo/Generators/flx)

# Needed if using old art with new cetbuildtools / CMake.
check_ups_version(art "$ENV{ART_VERSION}" v1_11_00 PRODUCT_OLDER_VAR have_old_art)
if (have_old_art)
  cmake_policy(SET CMP0046 OLD)
endif()
unset(have_old_art)

check_ups_version(art "$ENV{ART_VERSION}" v2_00_00 PRODUCT_OLDER_VAR no_canvas)
if (no_canvas)
  set(CANVAS "art")
else (no_canvas)
  find_ups_product( canvas v1_00_00 )
  set(CANVAS "canvas")
  add_definitions(-DCANVAS)
endif()

# macros for art_dictionary and simple_plugin
include(ArtDictionary)
include(ArtMake)
include(BuildPlugins)
include(CetTest)

# source
add_subdirectory(artdaq-demo)

# testing
add_subdirectory(test)

# tools
add_subdirectory(tools)

# examples
FILE(COPY examples DESTINATION .)
INSTALL(DIRECTORY examples DESTINATION ${product}/${version})

# ups - table and config files
add_subdirectory(ups)

# packaging utility
include(UseCPack)
