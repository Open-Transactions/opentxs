# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("crypto" "ParameterType.hpp")
libopentxs_add_public_header("crypto" "Parameters.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_BINARY_DIR}/src/crypto/Parameters_generated.cpp"
    "${opentxs_SOURCE_DIR}/src/internal/crypto/Parameters.hpp"
    "Imp.cpp"
    "Imp.hpp"
    "Parameters.cpp"
)
