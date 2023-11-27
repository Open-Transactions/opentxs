# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(zap)

libopentxs_add_public_header("api/network" "Types.hpp")
libopentxs_add_public_header("api/network" "ZAP.hpp")
libopentxs_add_public_header("api/network" "ZeroMQ.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "Types.cpp"
    "Types.internal.cpp"
    "Types.internal.hpp"
    "ZAP.cpp"
    "ZAP.factory.cpp"
    "ZAP.internal.cpp"
    "ZAP.internal.hpp"
    "ZAPPrivate.cpp"
    "ZAPPrivate.hpp"
    "ZeroMQ.cpp"
    "ZeroMQ.factory.cpp"
    "ZeroMQ.internal.cpp"
    "ZeroMQ.internal.hpp"
    "ZeroMQPrivate.cpp"
    "ZeroMQPrivate.hpp"
    "internal.factory.hpp"
)
