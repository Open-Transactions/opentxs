# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OPENSSL_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/crypto/library/OpenSSL.hpp"
      "BIO.cpp"
      "BIO.hpp"
      "OpenSSL.cpp"
      "OpenSSL.hpp"
  )
  opentxs_link_internal(OpenSSL::Crypto)
  opentxs_link_external(OpenSSL::Crypto)
  opentxs_include_directories("${OPENSSL_INCLUDE_DIR}")

  if(RSA_EXPORT)
    target_sources(opentxs-common PRIVATE "RSA.cpp")
  else()
    target_sources(opentxs-common PRIVATE "noRSA.cpp")
  endif()

  if(WIN32 AND OT_STATIC_DEPENDENCIES)
    opentxs_link_internal(CRYPT32.LIB)
    opentxs_link_external(CRYPT32.LIB)
  endif()
else()
  target_sources(opentxs-common PRIVATE "Null.cpp")
endif()
