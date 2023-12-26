# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_ENABLE_RPC)
  add_subdirectory(processor)
endif()

add_subdirectory(request)
add_subdirectory(response)

libopentxs_add_public_header("interface/rpc" "AccountData.hpp")
libopentxs_add_public_header("interface/rpc" "AccountEvent.hpp")
libopentxs_add_public_header("interface/rpc" "AccountEventType.hpp")
libopentxs_add_public_header("interface/rpc" "AccountType.hpp")
libopentxs_add_public_header("interface/rpc" "CommandType.hpp")
libopentxs_add_public_header("interface/rpc" "ContactEventType.hpp")
libopentxs_add_public_header("interface/rpc" "PaymentType.hpp")
libopentxs_add_public_header("interface/rpc" "PushType.hpp")
libopentxs_add_public_header("interface/rpc" "ResponseCode.hpp")
libopentxs_add_public_header("interface/rpc" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/interface/rpc/RPC.hpp"
    "AccountData.cpp"
    "AccountEvent.cpp"
    "AccountEventType.cpp"
    "AccountType.cpp"
    "CommandType.cpp"
    "ContactEventType.cpp"
    "PaymentType.cpp"
    "PushType.cpp"
    "ResponseCode.cpp"
    "Types.cpp"
    "Types.internal.cpp"
    "Types.internal.hpp"
)

if(OT_ENABLE_RPC)
  target_sources(
    opentxs-common
    PRIVATE
      "RPC.cpp"
      "RPC.hpp"
      "RPC.tpp"
  )
else()
  target_sources(opentxs-common PRIVATE "Null.cpp")
endif()
