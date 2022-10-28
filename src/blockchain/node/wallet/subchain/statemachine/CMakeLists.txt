# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/subchain/statemachine/Index.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/subchain/statemachine/Job.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/subchain/statemachine/Process.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/subchain/statemachine/Progress.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/subchain/statemachine/Rescan.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/subchain/statemachine/Scan.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
    "Common.cpp"
    "DeterministicIndex.cpp"
    "DeterministicIndex.hpp"
    "ElementCache.cpp"
    "ElementCache.cpp"
    "Index.cpp"
    "Index.cpp"
    "Job.cpp"
    "Job.hpp"
    "NotificationIndex.cpp"
    "NotificationIndex.hpp"
    "Process.cpp"
    "Process.hpp"
    "Progress.cpp"
    "Progress.hpp"
    "Rescan.cpp"
    "Rescan.hpp"
    "Scan.cpp"
    "Scan.hpp"
)
