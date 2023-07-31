# Copyright (c) 2010-2023 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(opentxs-testlib PRIVATE "Helpers.hpp" "Helpers.cpp")

if(OT_ENABLE_RPC)
  target_sources(opentxs-testlib PRIVATE "Helpers-rpc.cpp")
  target_sources(opentxs-testlib PRIVATE "SendPayment_blockchain.cpp")
  target_sources(opentxs-testlib PRIVATE "Rpc.cpp")
  target_sources(opentxs-testlib PRIVATE "RpcAsync.cpp")
else()
  target_sources(opentxs-testlib PRIVATE "Null.cpp")
endif()
