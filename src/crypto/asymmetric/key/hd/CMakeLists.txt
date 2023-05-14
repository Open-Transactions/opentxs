# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("crypto/asymmetric/key" "HD.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/crypto/asymmetric/key/HD.hpp"
    "HD.cpp"
    "HDPrivate.cpp"
    "HDPrivate.hpp"
    "Imp.cpp"
    "Imp.hpp"
)

if(BIP32_EXPORT)
  target_sources(opentxs-common PRIVATE "Enabled.cpp")

  if(ED25519_EXPORT)
    target_sources(opentxs-common PRIVATE "EnabledEd25519.cpp")
  else()
    target_sources(opentxs-common PRIVATE "DisabledEd25519.cpp")
  endif()

  if(SECP256K1_EXPORT)
    target_sources(opentxs-common PRIVATE "EnabledSecp256k1.cpp")
  else()
    target_sources(opentxs-common PRIVATE "DisabledSecp256k1.cpp")
  endif()
else()
  target_sources(opentxs-common PRIVATE "Disabled.cpp")
  target_sources(opentxs-common PRIVATE "DisabledEd25519.cpp")
  target_sources(opentxs-common PRIVATE "DisabledSecp256k1.cpp")
endif()
