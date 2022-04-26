# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  ottest
  PRIVATE
    "Activity.cpp"
    "Activity.hpp"
    "Basic.cpp"
    "Basic.hpp"
    "Download.hpp"
    "HeaderOracle.cpp"
    "HeaderOracle.hpp"
    "Regtest.cpp"
    "Regtest.hpp"
    "RegtestSimple.cpp"
    "RegtestSimple.hpp"
    "StartStop.hpp"
)
