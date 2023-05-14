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

libopentxs_add_public_header("otx" "ConsensusType.hpp")
libopentxs_add_public_header("otx" "LastReplyStatus.hpp")
libopentxs_add_public_header("otx" "OTXPushType.hpp")
libopentxs_add_public_header("otx" "OperationType.hpp")
libopentxs_add_public_header("otx" "Reply.hpp")
libopentxs_add_public_header("otx" "Request.hpp")
libopentxs_add_public_header("otx" "ServerReplyType.hpp")
libopentxs_add_public_header("otx" "ServerRequestType.hpp")
libopentxs_add_public_header("otx" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/otx/AccountList.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/OTX.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/Types.hpp"
    "AccountList.cpp"
    "OTX.cpp"
    "Reply.cpp"
    "Reply.hpp"
    "Request.cpp"
    "Request.hpp"
)
