# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("network/otdht" "Acknowledgement.hpp")
libopentxs_add_public_header("network/otdht" "Base.hpp")
libopentxs_add_public_header("network/otdht" "Block.hpp")
libopentxs_add_public_header("network/otdht" "Data.hpp")
libopentxs_add_public_header("network/otdht" "MessageType.hpp")
libopentxs_add_public_header("network/otdht" "PublishContract.hpp")
libopentxs_add_public_header("network/otdht" "PublishContractReply.hpp")
libopentxs_add_public_header("network/otdht" "PushTransaction.hpp")
libopentxs_add_public_header("network/otdht" "PushTransactionReply.hpp")
libopentxs_add_public_header("network/otdht" "Query.hpp")
libopentxs_add_public_header("network/otdht" "QueryContract.hpp")
libopentxs_add_public_header("network/otdht" "QueryContractReply.hpp")
libopentxs_add_public_header("network/otdht" "Request.hpp")
libopentxs_add_public_header("network/otdht" "State.hpp")
libopentxs_add_public_header("network/otdht" "Types.hpp")

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
