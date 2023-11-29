# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(context)
add_subdirectory(curve)
add_subdirectory(message)
add_subdirectory(socket)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/ListenCallback.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/PairEventCallback.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/Pipeline.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/Proxy.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/ReplyCallback.hpp"
    "Actor.cpp"
    "Actor.hpp"
    "ListenCallback.cpp"
    "ListenCallback.hpp"
    "PairEventCallback.cpp"
    "PairEventCallback.hpp"
    "PairEventListener.cpp"
    "PairEventListener.hpp"
    "Proxy.cpp"
    "Proxy.hpp"
    "ReplyCallback.cpp"
    "ReplyCallback.hpp"
)
libopentxs_link_internal("${OT_ZMQ_TARGET}")
libopentxs_link_external("${OT_ZMQ_TARGET}")
