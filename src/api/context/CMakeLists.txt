# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("api" "Context.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/api/Context.hpp"
    "Context.cpp"
    "Context.hpp"
)

if(OT_ENABLE_RPC)
  target_sources(opentxs-common PRIVATE "rpc.cpp")
else()
  target_sources(opentxs-common PRIVATE "no-rpc.cpp")
endif()

if(OT_QT_EXPORT)
  target_sources(opentxs-common PRIVATE "Qt.cpp")
  libopentxs_link_internal(Qt::Core)
  libopentxs_link_external(Qt::Core)
else()
  target_sources(opentxs-common PRIVATE "no-Qt.cpp")
endif()
