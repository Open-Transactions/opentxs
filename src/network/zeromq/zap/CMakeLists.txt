# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/zap/Callback.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/zap/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/zap/Handler.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/zap/Reply.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/zap/Request.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/zap/ZAP.hpp"
    "Callback.cpp"
    "Callback.hpp"
    "Handler.cpp"
    "Handler.hpp"
    "Reply.cpp"
    "Reply.hpp"
    "Request.cpp"
    "Request.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/zap/Types.hpp"
)
target_link_libraries(opentxs-common PRIVATE "${OT_ZMQ_TARGET}")
target_link_libraries(opentxs PUBLIC "${OT_ZMQ_TARGET}")
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/network/zeromq/zap
    COMPONENT dev
  )
endif()
