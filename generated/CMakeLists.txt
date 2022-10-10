# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(blockchain)
if(CASH_LUCRE_EXPORT)
  add_subdirectory(lucre)
endif()
add_subdirectory(license)
add_subdirectory(otprotob)
add_subdirectory(protobuf)
add_subdirectory(size)
add_subdirectory(tests)

if(BIP32_EXPORT)
  set(OPENTXS_HAVE_HD_KEYS "true")
  set(OPENTXS_DEFAULT_CRED_TYPE "identity::CredentialType::HD")

  if(SECP256K1_EXPORT)
    set(OPENTXS_DEFAULT_SOURCE_TYPE "identity::SourceType::Bip47")
  else()
    set(OPENTXS_DEFAULT_SOURCE_TYPE "identity::SourceType::PubKey")
  endif()
else()
  set(OPENTXS_HAVE_HD_KEYS "false")
  set(OPENTXS_DEFAULT_CRED_TYPE "identity::CredentialType::Legacy")
  set(OPENTXS_DEFAULT_SOURCE_TYPE "identity::SourceType::PubKey")
endif()

if(SECP256K1_EXPORT)
  set(OPENTXS_DEFAULT_KEY_TYPE "ParameterType::secp256k1")
elseif(ED25519_EXPORT)
  set(OPENTXS_DEFAULT_KEY_TYPE "ParameterType::ed25519")
elseif(RSA_EXPORT)
  set(OPENTXS_DEFAULT_KEY_TYPE "ParameterType::rsa")
else()
  set(OPENTXS_DEFAULT_KEY_TYPE "ParameterType::error")
endif()

if(SECP256K1_EXPORT)
  set(OPENTXS_CRYPTO_HAVE_SECP256K1 "true")
else()
  set(OPENTXS_CRYPTO_HAVE_SECP256K1 "false")
endif()

if(ED25519_EXPORT)
  set(OPENTXS_CRYPTO_HAVE_ED25519 "true")
else()
  set(OPENTXS_CRYPTO_HAVE_ED25519 "false")
endif()

if(RSA_EXPORT)
  set(OPENTXS_CRYPTO_HAVE_RSA "true")
else()
  set(OPENTXS_CRYPTO_HAVE_RSA "false")
endif()

if(MSVC)
  set(OPENTXS_EXPORT_TEMPLATE_PARAM "")
else()
  set(OPENTXS_EXPORT_TEMPLATE_PARAM "OPENTXS_EXPORT")
endif()

configure_file(
  "Platform.hpp.in"
  "${opentxs_BINARY_DIR}/include/opentxs/util/Platform.hpp"
  @ONLY
)

configure_file(
  "Version.cpp.in"
  "${opentxs_BINARY_DIR}/src/Version.cpp"
  @ONLY
)

configure_file(
  "StorageLevel.cpp.in"
  "${opentxs_BINARY_DIR}/src/blockchain/database/common/StorageLevel.cpp"
  @ONLY
)

configure_file(
  "NymParameters.cpp.in"
  "${opentxs_BINARY_DIR}/src/crypto/Parameters_generated.cpp"
  @ONLY
)

configure_file(
  "CryptoConfig.cpp.in"
  "${opentxs_BINARY_DIR}/src/api/crypto/Config_generated.cpp"
  @ONLY
)

include(GenerateExportHeader)
generate_export_header(
  opentxs
  EXPORT_FILE_NAME
  "${opentxs_BINARY_DIR}/include/opentxs/Export.hpp"
  EXPORT_MACRO_NAME
  OPENTXS_EXPORT
)
