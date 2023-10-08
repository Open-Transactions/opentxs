# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("network/blockchain" "Address.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/network/blockchain/Address.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/blockchain/Factory.hpp"
    "Address.cpp"
    "Internal.cpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "AddressPrivate.cpp"
      "AddressPrivate.hpp"
      "Factory.cpp"
  )
else()
  target_sources(opentxs-common PRIVATE "Disabled.cpp")
endif()
