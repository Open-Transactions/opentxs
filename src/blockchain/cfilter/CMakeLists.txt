# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("blockchain/cfilter" "FilterType.hpp")
libopentxs_add_public_header("blockchain/cfilter" "GCS.hpp")
libopentxs_add_public_header("blockchain/cfilter" "Hash.hpp")
libopentxs_add_public_header("blockchain/cfilter" "Header.hpp")
libopentxs_add_public_header("blockchain/cfilter" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/cfilter/GCS.hpp"
    "Common.cpp"
    "GCS.cpp"
    "GCSPrivate.cpp"
    "GCSPrivate.hpp"
    "Hash.cpp"
    "Header.cpp"
)

libopentxs_add_platform_specific("Hash")
libopentxs_add_platform_specific("Header")

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "Factory.cpp"
      "GCSImp.cpp"
      "GCSImp.hpp"
  )
else()
  target_sources(opentxs-common PRIVATE "Disabled.cpp")
endif()
