# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

file(
  READ
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/DifficultyTest.c"
  DIFFICULTY_TEST_ORIGINAL
)
string(
  REPLACE
    "DifficultyTest_isMinAnnDiffOk"
    "pc_is_min_ann_diff_ok"
    DIFFICULTY_TEST_1
    "${DIFFICULTY_TEST_ORIGINAL}"
)
string(
  REPLACE
    "DifficultyTest_degradeAnnouncementTarget"
    "pc_degrade_announcement_target"
    DIFFICULTY_TEST_2
    "${DIFFICULTY_TEST_1}"
)
string(
  REPLACE
    "assert(BN_zero(out));"
    "BN_zero(out);"
    DIFFICULTY_TEST_3
    "${DIFFICULTY_TEST_2}"
)
string(
  REPLACE
    "DifficultyTest_getEffectiveTarget"
    "pc_get_effective_target"
    DIFFICULTY_TEST_FIXED
    "${DIFFICULTY_TEST_3}"
)
file(
  WRITE "${CMAKE_CURRENT_BINARY_DIR}/DifficultyTest.c"
  "${DIFFICULTY_TEST_FIXED}"
)

add_library(
  packetcrypt OBJECT
  "${CMAKE_CURRENT_BINARY_DIR}/DifficultyTest.c"
  "Validate-fixed.c"
  "Validate-fixed.h"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/include/packetcrypt/PacketCrypt.h"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/include/packetcrypt/Validate.h"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/AnnMerkle.c"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/AnnMerkle.h"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/Announce.c"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/Announce.h"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/Buf.h"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/Conf.h"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/CryptoCycle.c"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/CryptoCycle.h"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/DecodeInsn.h"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/Difficulty.h"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/Hash.c"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/Hash.h"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/OpTemplate2.h"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/PacketCryptProof.c"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/PacketCryptProof.h"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/PcCompress.c"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/PcCompress.h"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/RandGen.c"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/RandGen.h"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/RandHash.h"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/RandHash_interpreted.c"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/Util.h"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/Validate.c"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/ValidateCtx.h"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/Vec.h"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/Work.c"
  "packetcrypt_rs/packetcrypt-sys/packetcrypt/src/Work.h"
)
target_link_libraries(
  packetcrypt PUBLIC OpenSSL::Crypto unofficial-sodium::sodium
)
target_include_directories(
  packetcrypt SYSTEM
  INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
  PUBLIC "packetcrypt_rs/packetcrypt-sys/packetcrypt/include"
  PRIVATE "packetcrypt_rs/packetcrypt-sys/packetcrypt/src"
)
set_target_properties(
  packetcrypt
  PROPERTIES
    C_INCLUDE_WHAT_YOU_USE ""
    CXX_INCLUDE_WHAT_YOU_USE ""
    C_STANDARD 11
    UNITY_BUILD OFF
)

# defined signed overflow
if(MSVC)
  # TODO target_compile_options(packetcrypt PRIVATE "/d2UndefIntOverflow-")
else()
  target_compile_options(packetcrypt PRIVATE "-fwrapv")
endif()

opentxs_link_internal(packetcrypt)
opentxs_link_external(OpenSSL::Crypto unofficial-sodium::sodium)
opentxs_add_sources($<TARGET_OBJECTS:packetcrypt>)
