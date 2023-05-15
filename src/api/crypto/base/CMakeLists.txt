# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header("api/crypto" "Crypto.hpp")

target_sources(opentxs-common PRIVATE "Crypto.cpp" "Crypto.hpp")

if(LIBSECP256K1_EXPORT)
  target_sources(opentxs-common PRIVATE "Libsecp256k1.cpp")
else()
  target_sources(opentxs-common PRIVATE "noLibsecp256k1.cpp")
endif()

if(OPENSSL_EXPORT)
  target_sources(opentxs-common PRIVATE "OpenSSL.cpp")
else()
  target_sources(opentxs-common PRIVATE "noOpenSSL.cpp")
endif()
