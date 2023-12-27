# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(request)
add_subdirectory(response)

libopentxs_add_public_header("rpc" "AccountData.hpp")
libopentxs_add_public_header("rpc" "AccountEvent.hpp")
libopentxs_add_public_header("rpc" "AccountEventType.hpp")
libopentxs_add_public_header("rpc" "AccountType.hpp")
libopentxs_add_public_header("rpc" "CommandType.hpp")
libopentxs_add_public_header("rpc" "ContactEventType.hpp")
libopentxs_add_public_header("rpc" "PaymentType.hpp")
libopentxs_add_public_header("rpc" "PushType.hpp")
libopentxs_add_public_header("rpc" "ResponseCode.hpp")
libopentxs_add_public_header("rpc" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "AccountData.cpp"
    "AccountEvent.cpp"
    "AccountEventType.cpp"
    "AccountType.cpp"
    "CommandType.cpp"
    "ContactEventType.cpp"
    "PaymentType.cpp"
    "Processor.internal.cpp"
    "Processor.internal.hpp"
    "PushType.cpp"
    "ResponseCode.cpp"
    "Types.cpp"
    "Types.internal.cpp"
    "Types.internal.hpp"
    "internal.factory.hpp"
)

if(OT_ENABLE_RPC)
  target_sources(
    opentxs-common
    PRIVATE
      "Processor.factory.cpp"
      "ProcessorPrivate.cpp"
      "ProcessorPrivate.getaccountactivity.cpp"
      "ProcessorPrivate.getaccountbalance.cpp"
      "ProcessorPrivate.hpp"
      "ProcessorPrivate.listaccounts.cpp"
      "ProcessorPrivate.listnyms.cpp"
      "ProcessorPrivate.sendpayment.cpp"
      "ProcessorPrivate.tpp"
      "ProcessorPrivate.tpp.cpp"
  )
else()
  target_sources(opentxs-common PRIVATE "Processor.disabled.cpp")
endif()
