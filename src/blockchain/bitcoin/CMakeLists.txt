# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_BLOCKCHAIN_EXPORT)
  add_subdirectory(bloom)
endif()

add_subdirectory(cfilter)

target_sources(opentxs-common PRIVATE "NumericHash.cpp" "NumericHash.hpp")
set(cxx-install-headers "")

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/bitcoin/Bitcoin.hpp"
      "Bitcoin.cpp"
      "Inventory.cpp"
      "Inventory.hpp"
      "Work.cpp"
      "Work.hpp"
  )
  target_link_libraries(opentxs-common PRIVATE Boost::headers)
  list(
    APPEND
    cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/bitcoin/NumericHash.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/bitcoin/Work.hpp"
  )
endif()

target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/bitcoin
    COMPONENT dev
  )
endif()
