# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/block/Transaction.hpp"
    "Transaction.cpp"
    "TransactionPrivate.cpp"
    "TransactionPrivate.hpp"
    "Internal.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/block/Transaction.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(opentxs-common PRIVATE "Imp.cpp" "Imp.hpp")
endif()

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/block
    COMPONENT dev
  )
endif()
