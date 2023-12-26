# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(blind)
add_subdirectory(client)
add_subdirectory(common)
add_subdirectory(consensus)
add_subdirectory(server)
add_subdirectory(smartcontract)

libopentxs_add_public_header("otx" "Reply.hpp")
libopentxs_add_public_header("otx" "Request.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/otx/AccountList.hpp"
    "AccountList.cpp"
    "Reply.cpp"
    "Reply.hpp"
    "Request.cpp"
    "Request.hpp"
)
