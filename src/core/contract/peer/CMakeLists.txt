# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(reply)
add_subdirectory(request)

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/Object.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/PairEvent.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/PairEventType.hpp"
    "Factory.cpp"
    "Object.cpp"
    "Object.hpp"
    "PairEvent.cpp"
)
