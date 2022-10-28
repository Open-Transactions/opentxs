# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/BailmentNotice.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/BailmentReply.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/BailmentRequest.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/ConnectionReply.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/ConnectionRequest.hpp"
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
    "ConnectionReply.cpp"
    "ConnectionReply.cpp"
    "ConnectionRequest.cpp"
    "ConnectionRequest.cpp"
    "NoticeAcknowledgement.cpp"
    "NoticeAcknowledgement.cpp"
    "OutBailmentReply.cpp"
    "OutBailmentReply.cpp"
    "OutBailmentRequest.cpp"
    "OutBailmentRequest.cpp"
    "Peer.cpp"
    "PeerObject.cpp"
    "PeerObject.hpp"
    "PeerReply.cpp"
    "PeerReply.cpp"
    "PeerRequest.cpp"
    "PeerRequest.cpp"
    "StoreSecret.cpp"
    "StoreSecret.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/peer/ConnectionInfoType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/peer/PeerObjectType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/peer/PeerRequestType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/peer/SecretType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/peer/Types.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core/contract/peer
    COMPONENT dev
  )
endif()
