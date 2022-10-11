# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(contract)
add_subdirectory(display)
add_subdirectory(identifier)
add_subdirectory(paymentcode)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/core/Amount.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/Armored.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/Core.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/Null.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/String.hpp"
    "Amount.cpp"
    "Armored.cpp"
    "Armored.hpp"
    "ByteArray.cpp"
    "ByteArrayPrivate.cpp"
    "ByteArrayPrivate.hpp"
    "Contact.cpp"
    "Core.cpp"
    "Data.cpp"
    "FixedByteArray.cpp"
    "FixedByteArray.tpp"
    "Secret.cpp"
    "SecretPrivate.cpp"
    "SecretPrivate.hpp"
    "Shutdown.cpp"
    "Shutdown.hpp"
    "StateMachine.cpp"
    "StateMachine.hpp"
    "String.cpp"
    "String.hpp"
    "Worker.hpp"
)
target_include_directories(
  opentxs-common SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
)

set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/core/AccountType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/AddressType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Amount.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/ByteArray.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Contact.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Data.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/FixedByteArray.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Secret.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Types.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/UnitType.hpp"
)
target_link_libraries(
  opentxs-common
  PRIVATE
    Boost::headers
    ZLIB::ZLIB
    opentxs-core-otprotob
    unofficial-sodium::sodium
)
target_link_libraries(
  opentxs
  PUBLIC
    Boost::system
    ZLIB::ZLIB
    unofficial-sodium::sodium
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core
    COMPONENT dev
  )
endif()
