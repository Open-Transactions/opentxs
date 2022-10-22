# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/LucreConfig.cpp"
    "Lucre.cpp"
    "Lucre.hpp"
    "Mint.cpp"
    "Mint.hpp"
    "Token.cpp"
    "Token.hpp"
)
target_include_directories(
  opentxs-common SYSTEM
  PRIVATE
    "${opentxs_SOURCE_DIR}/deps/"
    "${opentxs_SOURCE_DIR}/deps/robin-hood/src/include"
    "${opentxs_SOURCE_DIR}/deps/lucre/include"
    "${OPENSSL_INCLUDE_DIR}"
)
