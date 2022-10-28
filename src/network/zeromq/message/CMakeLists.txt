# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/message/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/message/Frame.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/message/FrameIterator.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/message/FrameSection.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/message/Message.hpp"
    "Frame.cpp"
    "Frame.hpp"
    "FrameIterator.cpp"
    "FrameIterator.hpp"
    "FrameSection.cpp"
    "FrameSection.hpp"
    "Message.cpp"
    "Message.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/message/Frame.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/message/FrameIterator.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/message/FrameSection.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/message/Message.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/network/zeromq/message/Message.tpp"
)
opentxs_link_internal("${OT_ZMQ_TARGET}")
opentxs_link_external("${OT_ZMQ_TARGET}")
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/network/zeromq/message
    COMPONENT dev
  )
endif()
