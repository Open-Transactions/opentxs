# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("interface/rpc/response" "Base.hpp")
libopentxs_add_public_header("interface/rpc/response" "GetAccountActivity.hpp")
libopentxs_add_public_header("interface/rpc/response" "GetAccountBalance.hpp")
libopentxs_add_public_header("interface/rpc/response" "ListAccounts.hpp")
libopentxs_add_public_header("interface/rpc/response" "ListNyms.hpp")
libopentxs_add_public_header("interface/rpc/response" "SendPayment.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "Base.cpp"
    "Base.hpp"
    "Factory.cpp"
    "GetAccountActivity.cpp"
    "GetAccountBalance.cpp"
    "Invalid.cpp"
    "Invalid.hpp"
    "ListAccounts.cpp"
    "ListNyms.cpp"
    "SendPayment.cpp"
)
