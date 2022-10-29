# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/network/otdht/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/otdht/Types.hpp"
    "Acknowledgement.cpp"
    "Base.cpp"
    "Base.hpp"
    "Block.cpp"
    "Data.cpp"
    "Factory.cpp"
    "PublishContract.cpp"
    "PublishContractReply.cpp"
    "PushTransaction.cpp"
    "PushTransactionReply.cpp"
    "Query.cpp"
    "QueryContract.cpp"
    "QueryContractReply.cpp"
    "Request.cpp"
    "State.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/network/otdht/Acknowledgement.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/otdht/Base.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/otdht/Block.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/otdht/Data.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/otdht/MessageType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/otdht/PublishContract.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/otdht/PublishContractReply.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/otdht/PushTransaction.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/otdht/PushTransactionReply.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/otdht/Query.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/otdht/QueryContract.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/otdht/QueryContractReply.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/otdht/Request.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/otdht/State.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/otdht/Types.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/network/otdht
    COMPONENT dev
  )
endif()
