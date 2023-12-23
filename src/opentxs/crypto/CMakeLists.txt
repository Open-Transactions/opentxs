# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(asymmetric)
add_subdirectory(symmetric)

libopentxs_add_public_header("crypto" "Bip32Child.hpp")
libopentxs_add_public_header("crypto" "Bip43Purpose.hpp")
libopentxs_add_public_header("crypto" "EcdsaCurve.hpp")
libopentxs_add_public_header("crypto" "HashType.hpp")
libopentxs_add_public_header("crypto" "Language.hpp")
libopentxs_add_public_header("crypto" "ParameterType.hpp")
libopentxs_add_public_header("crypto" "SecretStyle.hpp")
libopentxs_add_public_header("crypto" "SeedStrength.hpp")
libopentxs_add_public_header("crypto" "SeedStyle.hpp")
libopentxs_add_public_header("crypto" "SignatureRole.hpp")
libopentxs_add_public_header("crypto" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "Bip32Child.cpp"
    "Bip43Purpose.cpp"
    "EcdsaCurve.cpp"
    "HashType.cpp"
    "Language.cpp"
    "ParameterType.cpp"
    "SecretStyle.cpp"
    "SeedStrength.cpp"
    "SeedStyle.cpp"
    "SignatureRole.cpp"
    "Types.cpp"
    "Types.internal.cpp"
    "Types.internal.hpp"
)
