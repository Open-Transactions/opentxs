# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header(
  "blockchain/protocol/bitcoin/bitcoincash/token/cashtoken/" "Capability.hpp"
)
libopentxs_add_public_header(
  "blockchain/protocol/bitcoin/bitcoincash/token/cashtoken" "Types.hpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common PRIVATE "Types.internal.cpp" "Types.internal.hpp"
  )
endif()
