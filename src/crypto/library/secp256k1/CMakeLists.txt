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

  if(OT_BUNDLED_SECP256K1)
    libopentxs_link_internal(secp256k1-obj)
    libopentxs_add_sources($<TARGET_OBJECTS:secp256k1-obj>)
  else()
    libopentxs_link_internal(unofficial::secp256k1)
    libopentxs_link_external(unofficial::secp256k1)
  endif()
else()
  target_sources(opentxs-common PRIVATE "Null.cpp")
endif()
