# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

file_to_hex(
  "${CMAKE_CURRENT_SOURCE_DIR}/Seednodes.json"
  "OPENTXS_BLOCKCHAIN_SEEDNODES_HEX"
)

configure_file(
  "Seednodes.cpp.in"
  "${opentxs_BINARY_DIR}/src/network/blockchain/Seednodes.cpp"
  @ONLY
)
