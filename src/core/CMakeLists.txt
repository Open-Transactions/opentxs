# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(contract)
add_subdirectory(paymentcode)

libopentxs_add_public_header("core" "Amount.hpp")
libopentxs_add_public_header("core" "ByteArray.hpp")
libopentxs_add_public_header("core" "Contact.hpp")
libopentxs_add_public_header("core" "Data.hpp")
libopentxs_add_public_header("core" "FixedByteArray.hpp")
libopentxs_add_public_header("core" "Secret.hpp")

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
    "Secret.cpp"
    "SecretPrivate.cpp"
    "SecretPrivate.hpp"
    "StateMachine.cpp"
    "StateMachine.hpp"
    "String.cpp"
    "String.hpp"
    "Types.cpp"
    "UnitType.cpp"
    "Worker.hpp"
)

libopentxs_add_platform_specific("FixedByteArray")
libopentxs_add_platform_specific("String")
libopentxs_link_internal(unofficial-sodium::sodium)
libopentxs_link_external(Boost::system)
libopentxs_link_external(unofficial-sodium::sodium)
