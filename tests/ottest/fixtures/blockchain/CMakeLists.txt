# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(regtest)

target_sources(
  opentxs-testlib
  PRIVATE
    "Activity.cpp"
    "Activity.hpp"
    "Basic.cpp"
    "Basic.hpp"
    "BlockHeader.cpp"
    "BlockHeader.hpp"
    "BlockListener.cpp"
    "BlockListener.hpp"
    "BlockchainStartup.cpp"
    "BlockchainStartup.hpp"
    "CfilterListener.cpp"
    "CfilterListener.hpp"
    "Common.hpp"
    "Download.hpp"
    "HeaderOracle.cpp"
    "HeaderOracle.hpp"
    "MinedBlocks.cpp"
    "MinedBlocks.hpp"
    "PeerListener.cpp"
    "PeerListener.hpp"
    "ScanListener.cpp"
    "ScanListener.hpp"
    "StartStop.hpp"
    "SyncListener.cpp"
    "SyncListener.hpp"
    "SyncRequestor.cpp"
    "SyncRequestor.hpp"
    "SyncSubscriber.cpp"
    "SyncSubscriber.hpp"
    "TXOState.cpp"
    "TXOState.hpp"
    "TXOs.cpp"
    "TXOs.hpp"
)
