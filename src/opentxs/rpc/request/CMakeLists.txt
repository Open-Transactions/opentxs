# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("rpc/request" "GetAccountActivity.hpp")
libopentxs_add_public_header("rpc/request" "GetAccountBalance.hpp")
libopentxs_add_public_header("rpc/request" "ListAccounts.hpp")
libopentxs_add_public_header("rpc/request" "ListNyms.hpp")
libopentxs_add_public_header("rpc/request" "Message.hpp")
libopentxs_add_public_header("rpc/request" "SendPayment.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "GetAccountActivity.cpp"
    "GetAccountBalance.cpp"
    "ListAccounts.cpp"
    "ListNyms.cpp"
    "Message.factory.cpp"
    "MessagePrivate.cpp"
    "MessagePrivate.hpp"
    "SendPayment.cpp"
)
