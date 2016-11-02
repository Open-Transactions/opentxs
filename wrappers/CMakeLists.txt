#
# Define everything here that all wrappers will use,
# then process each language in its own subdirectory
#


# Locate include files for our wrappers
set(SWIG_INCLUDE_DIRS
  SYSTEM
  ${CMAKE_CURRENT_SOURCE_DIR}/../deps/
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/core
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/core/crypto
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/ext
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/opentxs/client
)


# The opentxs libraries will we link to
set(OTX_LINK_LIBS opentxs)


# Build wrappers for each lang enabled
if(PYTHON)
  subdirs(python)
elseif(PYTHON2)
  subdirs(python)
elseif(PYTHON3)
  subdirs(python)
endif(PYTHON)

if(CSHARP)
  subdirs(csharp)
endif(CSHARP)

if(JAVA)
  subdirs(java)
endif(JAVA)

if(PHP)
  subdirs(php)
endif(PHP)

if(PERL)
  #subdirs(perl)
  message("Perl module currently breaks the build so is disabled.")
  message("See ./wrappers/CMakeLists.txt:28 to manually enable.")
endif(PERL)
