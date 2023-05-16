# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

macro(libopentxs_print_build_details)
  if(OPENTXS_STANDALONE)
    message(STATUS "Configuring libopentxs as a regular library")
  else()
    message(STATUS "Configuring libopentxs as an object library")
  endif()

  include(otcommon-print-build-details)
  otcommon_print_build_details(OPENTXS_PEDANTIC_BUILD OPENTXS_BUILD_TESTS)

  message(STATUS "Storage backends-----------------------------")
  message(STATUS "filesystem:               ${OT_STORAGE_FS}")
  message(STATUS "sqlite                    ${OT_STORAGE_SQLITE}")
  message(STATUS "LMDB                      ${OT_STORAGE_LMDB}")

  message(STATUS "Key algorithms-------------------------------")
  message(STATUS "ed25519:                  ${OT_CRYPTO_SUPPORTED_KEY_ED25519}")
  message(STATUS "RSA:                      ${OT_CRYPTO_SUPPORTED_KEY_RSA}")
  message(
    STATUS "secp256k1                 ${OT_CRYPTO_SUPPORTED_KEY_SECP256K1}"
  )

  message(STATUS "Crypto library providers---------------------")
  message(STATUS "OpenSSL:                  ${OT_CRYPTO_USING_OPENSSL}")
  message(STATUS "libsecp256k1:             ${OT_CRYPTO_USING_LIBSECP256K1}")
  message(STATUS "packetcrypt:              ${OT_CRYPTO_USING_PACKETCRYPT}")

  message(STATUS "Blockchain-----------------------------------")
  message(STATUS "Blockchain client:        ${OT_WITH_BLOCKCHAIN}")

  message(STATUS "Cash library providers-----------------------")
  message(STATUS "Lucre:                    ${OT_CASH_USING_LUCRE}")
  message(STATUS "Lucre debug info:         ${OT_LUCRE_DEBUG}")

  message(STATUS "Script engines-------------------------------")
  message(STATUS "Chai:                     ${OT_SCRIPT_USING_CHAI}")

  message(STATUS "Interface------------------------------------")
  message(STATUS "Qt:                       ${OT_WITH_QT}")
  message(STATUS "QML:                      ${OT_WITH_QML}")
  message(STATUS "RPC:                      ${OT_ENABLE_RPC}")

  message(STATUS "Developer -----------------------------------")
  message(STATUS "Valgrind support:         ${OT_VALGRIND}")
  message(STATUS "precompiled headers:      ${OT_PCH}")
  message(STATUS "iwyu:                     ${OPENTXS_IWYU_ARGS}")
  message(STATUS "fix_includes:             ${OPENTXS_FIX_INCLUDES_ARGS}")
  message(STATUS "clang-tidy:               ${OT_CLANG_TIDY}")

  if(OT_IWYU)
    message(STATUS "")
    message(STATUS "iwyu instructions:")
    message(
      STATUS
        "  1. Save build output when compiling: cmake --build . -- -k 0 | tee iwyu.txt"
    )
    message(
      STATUS
        "  2. Run the fix_includes target to apply automatic fixes: cmake --build . --target fix_includes"
    )
    message(STATUS "")
    message(STATUS "fix_includes caveats (requires manual editing to fix):")
    message(
      STATUS
        "  1. iwyu always suggests pre-c++11 standard library paths. (<string.h> vs <cstring>)"
    )
    message(
      STATUS
        "  2. iwyu will sometimes suggest bracket includes when we want quote includes."
    )
    message(
      STATUS
        "  3. iwyu will sometimes suggest platform-specific headers instead of standard headers. (<bits/exception.h> vs <stdexcept>) Add new cases to iwyu.imp when they occur."
    )
    message(
      STATUS
        "  4. iwyu will suggest forward declarations simultaneously in source files and their associated headers. cpp files should not have forward declarations."
    )
    message(
      STATUS
        "  5. The way iwyu sorts forward declarations is cursed. Sort namespaces before types and add one blank line between two namespaces at the same level or between the last namespace and the types."
    )
    message(
      STATUS "  6. Run clang-format on any files that fix_includes edited."
    )
    message(STATUS "")
  endif()
endmacro()
