# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(activity)
add_subdirectory(ui)

libopentxs_add_public_header("api/session" "Contacts.hpp")
libopentxs_add_public_header("api/session" "Crypto.hpp")
libopentxs_add_public_header("api/session" "Endpoints.hpp")
libopentxs_add_public_header("api/session" "OTX.hpp")
libopentxs_add_public_header("api/session" "Storage.hpp")
libopentxs_add_public_header("api/session" "UI.hpp")
libopentxs_add_public_header("api/session" "Workflow.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/api/session/Crypto.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/session/Endpoints.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/session/OTX.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/session/Storage.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/session/Types.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/api/session/Workflow.hpp"
    "Common.cpp"
    "Contacts.cpp"
    "Contacts.hpp"
    "Crypto.cpp"
    "Crypto.hpp"
    "Endpoints.cpp"
    "Endpoints.hpp"
    "OTX.cpp"
    "OTX.hpp"
    "Storage.cpp"
    "Storage.hpp"
    "Workflow.cpp"
    "Workflow.hpp"
)
