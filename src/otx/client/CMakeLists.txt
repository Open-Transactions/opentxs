# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(obsolete)

libopentxs_add_public_header("otx/client" "PaymentWorkflowState.hpp")
libopentxs_add_public_header("otx/client" "PaymentWorkflowType.hpp")
libopentxs_add_public_header("otx/client" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/otx/client/Client.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/client/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/client/Helpers.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/client/Issuer.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/client/OTPayment.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/client/Pair.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/client/ServerAction.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/client/Types.hpp"
    "Common.cpp"
    "DepositPayment.cpp"
    "DepositPayment.hpp"
    "Issuer.cpp"
    "Issuer.hpp"
    "OTPayment.cpp"
    "Operation.cpp"
    "Operation.hpp"
    "Pair.cpp"
    "Pair.hpp"
    "PaymentTasks.cpp"
    "PaymentTasks.hpp"
    "ServerAction.cpp"
    "ServerAction.hpp"
    "StateMachine.cpp"
    "StateMachine.hpp"
    "StateMachine.tpp"
)
