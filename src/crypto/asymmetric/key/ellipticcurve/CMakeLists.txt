# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("crypto/asymmetric/key" "EllipticCurve.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/crypto/asymmetric/key/EllipticCurve.hpp"
    "EllipticCurve.cpp"
    "EllipticCurvePrivate.cpp"
    "EllipticCurvePrivate.hpp"
    "Imp.cpp"
    "Imp.hpp"
)
