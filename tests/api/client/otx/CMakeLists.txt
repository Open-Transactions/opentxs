# Copyright (c) 2018 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(name unittests-opentxs-api-client-otx)

set(cxx-sources
        ${PROJECT_SOURCE_DIR}/tests/main.cpp
        Test_DepositCheques.cpp
        ${PROJECT_SOURCE_DIR}/tests/OTTestEnvironment.cpp
        )

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/tests
        ${GTEST_INCLUDE_DIRS}
)

add_executable(${name} ${cxx-sources})
target_link_libraries(${name} opentxs ${GTEST_LIBRARY})

if(NOT OT_BUNDLED_PROTOBUF)
  target_link_libraries(${name} ${PROTOBUF_LITE_LIBRARIES})
endif()

if(NOT OT_BUNDLED_OPENTXS_PROTO)
  target_link_libraries(${name} opentxs-proto)
endif()

if(OT_WITH_QT)
  target_link_libraries(${name} ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Qml_LIBRARIES} ${Qt5Widgets_LIBRARIES})
endif()

set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests)
add_test(${name} ${PROJECT_BINARY_DIR}/tests/${name} --gtest_output=xml:gtestresults.xml)
