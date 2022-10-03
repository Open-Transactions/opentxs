# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "ActivityThread.cpp"
    "ActivityThread.hpp"
    "ActivityThreadItem.cpp"
    "ActivityThreadItem.hpp"
    "MailItem.cpp"
    "MailItem.hpp"
    "PaymentItem.cpp"
    "PaymentItem.hpp"
    "PendingSend.cpp"
    "PendingSend.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/ui/ActivityThread.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/ui/ActivityThreadItem.hpp"
)
if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common PRIVATE "BlockchainActivityThreadItem.cpp"
                           "BlockchainActivityThreadItem.hpp"
  )
else()
  target_sources(opentxs-common PRIVATE "no-blockchain.cpp")
endif()

target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/interface/ui
    COMPONENT dev
  )
endif()
