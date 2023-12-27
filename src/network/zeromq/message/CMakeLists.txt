# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("network/zeromq/message" "Envelope.hpp")
libopentxs_add_public_header("network/zeromq/message" "Frame.hpp")
libopentxs_add_public_header("network/zeromq/message" "Message.hpp")
libopentxs_add_public_header("network/zeromq/message" "Message.tpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/message/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/message/Frame.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/network/zeromq/message/Message.hpp"
    "Envelope.cpp"
    "EnvelopePrivate.cpp"
    "EnvelopePrivate.hpp"
    "Frame.cpp"
    "Frame.hpp"
    "Message.cpp"
    "Message.cpp"
    "Message.tpp.cpp"
)
libopentxs_link_internal("${OT_ZMQ_TARGET}")
libopentxs_link_external("${OT_ZMQ_TARGET}")
