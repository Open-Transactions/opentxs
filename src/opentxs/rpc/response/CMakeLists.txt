# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("rpc/response" "Factory.hpp")
libopentxs_add_public_header("rpc/response" "GetAccountActivity.hpp")
libopentxs_add_public_header("rpc/response" "GetAccountBalance.hpp")
libopentxs_add_public_header("rpc/response" "ListAccounts.hpp")
libopentxs_add_public_header("rpc/response" "ListNyms.hpp")
libopentxs_add_public_header("rpc/response" "Message.hpp")
libopentxs_add_public_header("rpc/response" "SendPayment.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "Factory.cpp"
    "GetAccountActivity.cpp"
    "GetAccountBalance.cpp"
    "Invalid.cpp"
    "Invalid.hpp"
    "ListAccounts.cpp"
    "ListNyms.cpp"
    "MessagePrivate.cpp"
    "MessagePrivate.hpp"
    "SendPayment.cpp"
)
