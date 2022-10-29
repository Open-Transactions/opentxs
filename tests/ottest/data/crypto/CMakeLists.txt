# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-testlib
  PRIVATE
    "Bip32.cpp"
    "Bip32.hpp"
    "Hashes.cpp"
    "Hashes.hpp"
    "PaymentCodeV1.cpp"
    "PaymentCodeV1.hpp"
    "PaymentCodeV3.cpp"
    "PaymentCodeV3.hpp"
)
