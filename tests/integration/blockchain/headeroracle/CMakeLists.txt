# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_opentx_test(ottest-integration-blockchain-headeroracle-basic Basic.cpp)
add_opentx_test(ottest-integration-blockchain-headeroracle-bch BCH.cpp)
add_opentx_test(ottest-integration-blockchain-headeroracle-btc BTC.cpp)
add_opentx_test(
  ottest-integration-blockchain-headeroracle-checkpoint-already-best-chain
  AddCheckpointAlreadyInBestChain.cpp
)
add_opentx_test(
  ottest-integration-blockchain-headeroracle-checkpoint-delete
  DeleteCheckpoint.cpp
)
add_opentx_test(
  ottest-integration-blockchain-headeroracle-checkpoint-disconnected
  AddCheckpointDisconnected.cpp
)
add_opentx_test(
  ottest-integration-blockchain-headeroracle-checkpoint-prevents-reorg
  CheckpointPreventsReorg.cpp
)
add_opentx_test(
  ottest-integration-blockchain-headeroracle-checkpoint-prevents-update
  CheckpointPreventsUpdate.cpp
)
add_opentx_test(
  ottest-integration-blockchain-headeroracle-out-of-order OutOfOrder.cpp
)
add_opentx_test(ottest-integration-blockchain-headeroracle-random Random.cpp)
add_opentx_test(ottest-integration-blockchain-headeroracle-reorg Reorg.cpp)
add_opentx_test(
  ottest-integration-blockchain-headeroracle-reorg-to-checkpoint
  ReorgToCheckpoint.cpp
)
add_opentx_test(
  ottest-integration-blockchain-headeroracle-reorg-to-checkpoint-descendant
  ReorgToCheckpointDescendant.cpp
)
add_opentx_test(
  ottest-integration-blockchain-headeroracle-serialization Serialization.cpp
)
