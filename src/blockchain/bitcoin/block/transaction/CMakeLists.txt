# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/bitcoin/block/Transaction.hpp"
    "Transaction.cpp"
    "TransactionPrivate.cpp"
    "TransactionPrivate.hpp"
    "Internal.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/bitcoin/block/Transaction.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "Data.cpp"
      "Data.hpp"
      "Factory.cpp"
      "Imp.cpp"
      "Imp.hpp"
  )
else()
  target_sources(opentxs-common PRIVATE "Null.cpp")
endif()

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/bitcoin/block
    COMPONENT dev
  )
endif()
