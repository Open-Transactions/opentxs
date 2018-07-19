
set(name unittests-opentxs-api)

set(cxx-sources
  main.cpp
  Test_Bip47.cpp
  ${PROJECT_SOURCE_DIR}/tests/OTTestEnvironment.cpp
)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/tests
        ${GTEST_INCLUDE_DIRS}
)

#add_subdirectory(crypto)

add_executable(${name} ${cxx-sources})
target_link_libraries(${name} opentxs opentxs-proto ${GTEST_BOTH_LIBRARIES})
set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests)
add_test(${name} ${PROJECT_BINARY_DIR}/tests/${name} --gtest_output=xml:gtestresults.xml)
