# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(blind)
add_subdirectory(client)

libopentxs_add_public_header("otx" "ConsensusType.hpp")
libopentxs_add_public_header("otx" "LastReplyStatus.hpp")
libopentxs_add_public_header("otx" "OperationType.hpp")
libopentxs_add_public_header("otx" "PushType.hpp")
libopentxs_add_public_header("otx" "ServerReplyType.hpp")
libopentxs_add_public_header("otx" "ServerRequestType.hpp")
libopentxs_add_public_header("otx" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "ConsensusType.cpp"
    "LastReplyStatus.cpp"
    "OperationType.cpp"
    "PushType.cpp"
    "ServerReplyType.cpp"
    "ServerRequestType.cpp"
    "Types.cpp"
    "Types.internal.cpp"
    "Types.internal.hpp"
)
