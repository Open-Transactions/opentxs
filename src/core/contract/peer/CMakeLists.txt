# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("core/contract/peer" "ConnectionInfoType.hpp")
libopentxs_add_public_header("core/contract/peer" "PeerObjectType.hpp")
libopentxs_add_public_header("core/contract/peer" "PeerRequestType.hpp")
libopentxs_add_public_header("core/contract/peer" "SecretType.hpp")
libopentxs_add_public_header("core/contract/peer" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/BailmentNotice.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/BailmentReply.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/BailmentRequest.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/ConnectionReply.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/ConnectionRequest.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/FaucetRequest.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/NoticeAcknowledgement.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/OutBailmentReply.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/OutBailmentRequest.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/PeerObject.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/PeerReply.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/PeerRequest.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/StoreSecret.hpp"
    "BailmentNotice.cpp"
    "BailmentNotice.cpp"
    "BailmentReply.cpp"
    "BailmentReply.cpp"
    "BailmentRequest.cpp"
    "BailmentRequest.cpp"
    "Common.cpp"
    "ConnectionReply.cpp"
    "ConnectionReply.cpp"
    "ConnectionRequest.cpp"
    "ConnectionRequest.cpp"
    "FaucetReply.cpp"
    "FaucetReply.cpp"
    "FaucetRequest.cpp"
    "FaucetRequest.cpp"
    "NoticeAcknowledgement.cpp"
    "NoticeAcknowledgement.cpp"
    "OutBailmentReply.cpp"
    "OutBailmentReply.cpp"
    "OutBailmentRequest.cpp"
    "OutBailmentRequest.cpp"
    "PairEvent.cpp"
    "PeerObject.cpp"
    "PeerObject.hpp"
    "PeerReply.cpp"
    "PeerReply.cpp"
    "PeerRequest.cpp"
    "PeerRequest.cpp"
    "StoreSecret.cpp"
    "StoreSecret.cpp"
)
