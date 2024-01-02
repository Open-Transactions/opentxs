# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

file(
  READ
  "${opentxs_SOURCE_DIR}/src/api/crypto/base58/COPYING"
  BASE58_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/external/base64/COPYING"
  BASE64_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/external/LICENSE.bech32"
  BECH32_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/external/ChaiScript/LICENSE"
  CHAISCRIPT_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/src/crypto/library/dash/crypto/LICENSE"
  DASH_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/external/irrxml/COPYING"
  IRRXML_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/docs/BSD2-clause"
  LIBGUARDED_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/external/lucre/LICENCE"
  LUCRE_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/LICENSE"
  OPENTXS_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/docs/CREDITS"
  OPENTXS_CREDITS_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/docs/protobuf.license"
  PROTOBUF_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/docs/simpleini.license"
  SIMPLEINI_LICENSE_TEXT
)
file(
  READ
  "${opentxs_SOURCE_DIR}/src/matterfi/LICENSE.txt"
  MATTERFI_LICENSE_TEXT
)

include(otcommon-file-to-hex)
otcommon_file_to_hex(
  "${CMAKE_CURRENT_SOURCE_DIR}/apache-2.0.txt" "APACHE_2_0_HEX"
)
otcommon_file_to_hex("${CMAKE_CURRENT_SOURCE_DIR}/cc0.txt" "CC0_HEX")
otcommon_file_to_hex("${CMAKE_CURRENT_SOURCE_DIR}/lgpl-2.1.txt" "LGPL_2_1_HEX")
otcommon_file_to_hex("${opentxs_SOURCE_DIR}/docs/MPL-2.0" "MPL_2_0_HEX")
otcommon_file_to_hex(
  "${opentxs_SOURCE_DIR}/external/argon2/LICENSE" "ARGON_HEX"
)
otcommon_file_to_hex(
  "${opentxs_SOURCE_DIR}/external/frozen/LICENSE" "FROZEN_HEX"
)

configure_file(
  "argon.cpp.in"
  "${opentxs_BINARY_DIR}/src/util/license/argon.cpp"
  @ONLY
)

configure_file(
  "apache.cpp.in"
  "${opentxs_BINARY_DIR}/src/util/license/apache.cpp"
  @ONLY
)

configure_file(
  "base58.cpp.in"
  "${opentxs_BINARY_DIR}/src/util/license/base58.cpp"
  @ONLY
)

configure_file(
  "base64.cpp.in"
  "${opentxs_BINARY_DIR}/src/util/license/base64.cpp"
  @ONLY
)

configure_file(
  "bech32.cpp.in"
  "${opentxs_BINARY_DIR}/src/util/license/bech32.cpp"
  @ONLY
)

configure_file(
  "cc0.cpp.in"
  "${opentxs_BINARY_DIR}/src/util/license/cc0.cpp"
  @ONLY
)

if(SCRIPT_CHAI_EXPORT)
  configure_file(
    "chai.cpp.in"
    "${opentxs_BINARY_DIR}/src/util/license/chai.cpp"
    @ONLY
  )
else()
  configure_file(
    "no-chai.cpp.in"
    "${opentxs_BINARY_DIR}/src/util/license/chai.cpp"
    @ONLY
  )
endif()

if(OT_BLOCKCHAIN_EXPORT)
  configure_file(
    "dash.cpp.in"
    "${opentxs_BINARY_DIR}/src/util/license/dash.cpp"
    @ONLY
  )
else()
  configure_file(
    "no-dash.cpp.in"
    "${opentxs_BINARY_DIR}/src/util/license/dash.cpp"
    @ONLY
  )
endif()

configure_file(
  "frozen.cpp.in"
  "${opentxs_BINARY_DIR}/src/util/license/frozen.cpp"
  @ONLY
)

configure_file(
  "irrxml.cpp.in"
  "${opentxs_BINARY_DIR}/src/util/license/irrxml.cpp"
  @ONLY
)

configure_file(
  "libguarded.cpp.in"
  "${opentxs_BINARY_DIR}/src/util/license/libguarded.cpp"
  @ONLY
)

if(CASH_LUCRE_EXPORT)
  configure_file(
    "lucre.cpp.in"
    "${opentxs_BINARY_DIR}/src/util/license/lucre.cpp"
    @ONLY
  )
else()
  configure_file(
    "no-lucre.cpp.in"
    "${opentxs_BINARY_DIR}/src/util/license/lucre.cpp"
    @ONLY
  )
endif()

configure_file(
  "lgpl21.cpp.in"
  "${opentxs_BINARY_DIR}/src/util/license/lgpl21.cpp"
  @ONLY
)

if(OT_ENABLE_MATTERFI)
  configure_file(
    "matterfi.cpp.in"
    "${opentxs_BINARY_DIR}/src/util/license/matterfi.cpp"
    @ONLY
  )
else()
  configure_file(
    "no-matterfi.cpp.in"
    "${opentxs_BINARY_DIR}/src/util/license/matterfi.cpp"
    @ONLY
  )
endif()

configure_file(
  "mpl.cpp.in"
  "${opentxs_BINARY_DIR}/src/util/license/mpl.cpp"
  @ONLY
)

configure_file(
  "opentxs.cpp.in"
  "${opentxs_BINARY_DIR}/src/util/license/opentxs.cpp"
  @ONLY
)

if(PACKETCRYPT_EXPORT)
  configure_file(
    "packetcrypt.cpp.in"
    "${opentxs_BINARY_DIR}/src/util/license/packetcrypt.cpp"
    @ONLY
  )
else()
  configure_file(
    "no-packetcrypt.cpp.in"
    "${opentxs_BINARY_DIR}/src/util/license/packetcrypt.cpp"
    @ONLY
  )
endif()

configure_file(
  "protobuf.cpp.in"
  "${opentxs_BINARY_DIR}/src/util/license/protobuf.cpp"
  @ONLY
)

configure_file(
  "simpleini.cpp.in"
  "${opentxs_BINARY_DIR}/src/util/license/simpleini.cpp"
  @ONLY
)

configure_file(
  "tbb.cpp.in"
  "${opentxs_BINARY_DIR}/src/util/license/tbb.cpp"
  @ONLY
)
