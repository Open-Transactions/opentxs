# Copyright (c) Monetas AG, 2014

#add_subdirectory(ChaiScript)
add_subdirectory(gtest)
add_subdirectory(irrxml)

### Build lucre as library
set(lucre-sources
  ${CMAKE_CURRENT_SOURCE_DIR}/lucre/src/bankimp.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lucre/include/lucre/)


if(WIN32)
  # suppress sscanf/sprintf being unsafe warnings 
  add_definitions("/wd4996")

  add_library(lucre
    STATIC
    ${lucre-sources}
  )

  target_link_libraries(lucre ${LIB_EAY_RELEASE})
else()
  add_library(lucre
    ${lucre-sources}
  )

  target_link_libraries(lucre ssl crypto)
endif()


### Build bigint as library
set(bigint-sources
  ${CMAKE_CURRENT_SOURCE_DIR}/bigint/BigInteger.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/bigint/BigIntegerAlgorithms.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/bigint/BigIntegerUtils.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/bigint/BigUnsigned.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/bigint/BigUnsignedInABase.cc
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/bigint/)

if(WIN32)
  include_directories(${OPENSSL_INCLUDE_DIR})
  add_library(bigint
    STATIC
    ${bigint-sources}
  )
else()
  add_library(bigint
    ${bigint-sources}
  )
endif()



### Build libzmq as static library
if(NOT WIN32)
  include(GNUInstallDirs)
  execute_process(COMMAND sed "-i.bak" "s|add_library(libzmq SHARED|add_library(libzmq STATIC|" "${CMAKE_CURRENT_SOURCE_DIR}/libzmq_4/CMakeLists.txt")
  execute_process(COMMAND sed "-i.bak" "s|set(LIBRARY_OUTPUT_PATH  \${CMAKE_CURRENT_BINARY_DIR}/lib)| |" "${CMAKE_CURRENT_SOURCE_DIR}/libzmq_4/CMakeLists.txt")
  execute_process(COMMAND sed "-i.bak" "s|LIBRARY DESTINATION lib$|LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}|" "${CMAKE_CURRENT_SOURCE_DIR}/libzmq_4/CMakeLists.txt")
  execute_process(COMMAND sed "-i.bak" "s|ARCHIVE DESTINATION lib$|ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}|" "${CMAKE_CURRENT_SOURCE_DIR}/libzmq_4/CMakeLists.txt")
endif()

set(ZMQ_BUILD_TESTS OFF CACHE BOOL "")

if (NOT ZMQ_LIB_FOUND)
  add_subdirectory(libzmq_4)
endif()

if (NOT WIN32)
  include(GNUInstallDirs)
  install(TARGETS bigint lucre
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          EXPORT opentxsTargets
          COMPONENT main)
endif()
