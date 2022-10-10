# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

file_to_hex("${CMAKE_CURRENT_SOURCE_DIR}/BCH.json" "OPENTXS_BCH_HEX")
file_to_hex("${CMAKE_CURRENT_SOURCE_DIR}/BSV.json" "OPENTXS_BSV_HEX")
file_to_hex("${CMAKE_CURRENT_SOURCE_DIR}/BTC.json" "OPENTXS_BTC_HEX")
file_to_hex("${CMAKE_CURRENT_SOURCE_DIR}/ETH.json" "OPENTXS_ETH_HEX")
file_to_hex(
  "${CMAKE_CURRENT_SOURCE_DIR}/ETHropsten.json" "OPENTXS_ETHROPSTEN_HEX"
)
file_to_hex("${CMAKE_CURRENT_SOURCE_DIR}/LTC.json" "OPENTXS_LTC_HEX")
file_to_hex("${CMAKE_CURRENT_SOURCE_DIR}/PKT.json" "OPENTXS_PKT_HEX")
file_to_hex("${CMAKE_CURRENT_SOURCE_DIR}/UNITTEST.json" "OPENTXS_UNITTEST_HEX")
file_to_hex("${CMAKE_CURRENT_SOURCE_DIR}/XEC.json" "OPENTXS_XEC_HEX")
file_to_hex("${CMAKE_CURRENT_SOURCE_DIR}/tnBCH.json" "OPENTXS_TNBCH_HEX")
file_to_hex("${CMAKE_CURRENT_SOURCE_DIR}/tnBSV.json" "OPENTXS_TNBSV_HEX")
file_to_hex("${CMAKE_CURRENT_SOURCE_DIR}/tnBTC.json" "OPENTXS_TNBTC_HEX")
file_to_hex("${CMAKE_CURRENT_SOURCE_DIR}/tnLTC.json" "OPENTXS_TNLTC_HEX")
file_to_hex("${CMAKE_CURRENT_SOURCE_DIR}/tnPKT.json" "OPENTXS_TNPKT_HEX")
file_to_hex("${CMAKE_CURRENT_SOURCE_DIR}/tnXEC.json" "OPENTXS_TNXEC_HEX")

configure_file(
  "Json.cpp.in"
  "${opentxs_BINARY_DIR}/src/blockchain/Json.cpp"
  @ONLY
)
