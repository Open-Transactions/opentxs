# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/interface/ui/AccountSummary.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/interface/ui/AccountSummaryItem.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/interface/ui/IssuerItem.hpp"
    "AccountSummary.cpp"
    "AccountSummary.hpp"
    "AccountSummaryItem.cpp"
    "AccountSummaryItem.hpp"
    "IssuerItem.cpp"
    "IssuerItem.hpp"
)
