# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(LIBSECP256K1_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/crypto/library/Secp256k1.hpp"
      "Secp256k1.cpp"
      "Secp256k1.hpp"
  )

  libopentxs_link_internal(unofficial::secp256k1)
  libopentxs_link_external(unofficial::secp256k1)

  if(OT_USE_VCPKG_TARGETS)
    libopentxs_link_internal(unofficial::secp256k1_precomputed)
    libopentxs_link_external(unofficial::secp256k1_precomputed)
  endif()
else()
  target_sources(opentxs-common PRIVATE "Null.cpp")
endif()
