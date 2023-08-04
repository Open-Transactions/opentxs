# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/interface/ui/ContactActivity.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/interface/ui/ContactActivityItem.hpp"
    "ContactActivity.cpp"
    "ContactActivity.hpp"
    "ContactActivityItem.cpp"
    "ContactActivityItem.hpp"
    "MailItem.cpp"
    "MailItem.hpp"
    "PaymentItem.cpp"
    "PaymentItem.hpp"
    "PendingSend.cpp"
    "PendingSend.hpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common PRIVATE "BlockchainContactActivityItem.cpp"
                           "BlockchainContactActivityItem.hpp"
  )
else()
  target_sources(opentxs-common PRIVATE "no-blockchain.cpp")
endif()
