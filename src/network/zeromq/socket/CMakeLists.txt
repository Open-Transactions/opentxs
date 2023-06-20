# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("network/zeromq/socket" "Direction.hpp")
libopentxs_add_public_header("network/zeromq/socket" "Policy.hpp")
libopentxs_add_public_header("network/zeromq/socket" "SocketType.hpp")
libopentxs_add_public_header("network/zeromq/socket" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/socket/Dealer.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/socket/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/socket/Pair.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/socket/Pipeline.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/socket/Publish.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/socket/Pull.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/socket/Push.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/socket/Raw.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/socket/Reply.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/socket/Request.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/socket/Router.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/socket/Sender.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/socket/Socket.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/socket/SocketInternal.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/socket/Subscribe.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/socket/Types.hpp"
    "Bidirectional.hpp"
    "Bidirectional.tpp"
    "Common.cpp"
    "Dealer.cpp"
    "Dealer.hpp"
    "Pair.cpp"
    "Pair.hpp"
    "Pipeline.cpp"
    "Pipeline.hpp"
    "Publish.cpp"
    "Publish.hpp"
    "Pull.cpp"
    "Pull.hpp"
    "Push.cpp"
    "Push.hpp"
    "Raw.cpp"
    "Raw.hpp"
    "Receiver.hpp"
    "Receiver.tpp"
    "Reply.cpp"
    "Reply.hpp"
    "Request.cpp"
    "Request.hpp"
    "Router.cpp"
    "Router.hpp"
    "Sender.hpp"
    "Sender.tpp"
    "Socket.cpp"
    "Socket.hpp"
    "Subscribe.cpp"
    "Subscribe.hpp"
)
libopentxs_link_internal("${OT_ZMQ_TARGET}")
libopentxs_link_external("${OT_ZMQ_TARGET}")
