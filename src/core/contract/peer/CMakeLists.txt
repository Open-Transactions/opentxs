# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(reply)
add_subdirectory(request)

libopentxs_add_public_header("core/contract/peer" "ConnectionInfoType.hpp")
libopentxs_add_public_header("core/contract/peer" "ObjectType.hpp")
libopentxs_add_public_header("core/contract/peer" "RequestType.hpp")
libopentxs_add_public_header("core/contract/peer" "SecretType.hpp")
libopentxs_add_public_header("core/contract/peer" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/Object.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/PairEvent.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/PairEventType.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/contract/peer/Types.hpp"
    "Common.cpp"
    "Factory.cpp"
    "Object.cpp"
    "Object.hpp"
    "PairEvent.cpp"
)
