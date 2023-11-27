# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("network/zeromq" "Context.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/Batch.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/Context.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/Handle.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/Pool.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/Thread.hpp"
    "Batch.cpp"
    "Context.cpp"
    "Context.hpp"
    "Handle.cpp"
    "Pool.cpp"
    "Pool.hpp"
    "Thread.cpp"
    "Thread.hpp"
)
libopentxs_link_internal("${OT_ZMQ_TARGET}")
libopentxs_link_external(Boost::system "${OT_ZMQ_TARGET}")
