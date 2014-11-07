# Copyright (c) Monetas AG, 2014

add_subdirectory(otprotob)
add_subdirectory(trade)
add_subdirectory(cron)
add_subdirectory(script)
add_subdirectory(recurring)

set(cxx-sources
  util/Timer.cpp
  util/Assert.cpp
  util/StringUtils.cpp
  util/OTDataFolder.cpp
  util/OTFolders.cpp
  util/OTPaths.cpp
  transaction/Helpers.cpp
  mkcert.cpp
  Account.cpp
  AccountList.cpp
  crypto/OTASCIIArmor.cpp
  OTAssetContract.cpp
  crypto/BitcoinCrypto.cpp
  crypto/OTAsymmetricKey.cpp
  crypto/OTAsymmetricKeyOpenSSL.cpp
  crypto/OTAsymmetricKeyOpenSSLPrivdp.cpp
  crypto/OTCachedKey.cpp
  crypto/OTCallback.cpp
  crypto/OTCaller.cpp
  OTCheque.cpp
  OTContract.cpp
  crypto/OTCredential.cpp
  crypto/OTCrypto.cpp
  crypto/OTCryptoOpenSSL.cpp
  OTData.cpp
  crypto/OTEnvelope.cpp
  OTIdentifier.cpp
  OTInstrument.cpp
  OTItem.cpp
  crypto/OTKeyCredential.cpp
  crypto/OTKeypair.cpp
  crypto/OTKeyring.cpp
  OTLedger.cpp
  OTLog.cpp
  crypto/OTLowLevelKeyData.cpp
  crypto/OTMasterkey.cpp
  OTMessage.cpp
  OTNumList.cpp
  crypto/OTNymOrSymmetricKey.cpp
  crypto/OTPassword.cpp
  crypto/OTPasswordData.cpp
  OTPseudonym.cpp
  OTServerContract.cpp
  OTSettings.cpp
  crypto/OTSignatureMetadata.cpp
  crypto/OTSignedFile.cpp
  OTStorage.cpp
  String.cpp
  OTStringXML.cpp
  crypto/OTSubcredential.cpp
  crypto/OTSubkey.cpp
  crypto/OTSymmetricKey.cpp
  OTTrackable.cpp
  OTTransaction.cpp
  OTTransactionType.cpp
)

file(GLOB cxx-headers
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/*.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/crypto/*.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/transaction/*.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/util/*.hpp"
)

include_directories(
  ${CMAKE_BINARY_DIR}/src/core/otprotob
)

if(KEYRING_GNOME)
  include(GNUInstallDirs)
  include_directories(${CMAKE_INSTALL_FULL_OLDINCLUDEDIR}/glib-2.0/ /usr/${CMAKE_INSTALL_LIBDIR}/glib-2.0/include/ ${CMAKE_INSTALL_FULL_OLDINCLUDEDIR}/gnome-keyring-1/)
  set(keyring libgnome-keyring.so)
endif()

include_directories(SYSTEM
  ${OPENSSL_INCLUDE_DIR}
  ${PROTOBUF_INCLUDE_DIR}
  ${ZLIB_INCLUDE_DIRS}
)

set(MODULE_NAME opentxs-core)
if(WIN32)
  # suppress warnings about exported internal symbols (global log stream objects)
  add_definitions(-DOTLOG_IMPORT)

  # suppress warnings about strcpy being unsafe in anyoption.cpp
  add_definitions(/wd4996)

  # suppress linker warnings about locallly defined imported symbols
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4049 /ignore:4217")

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/windll.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
    @ONLY
  )

  # we have an extra Windows-specific file to compile
  add_library(${MODULE_NAME}
    ${cxx-sources}
    ${cxx-headers}
    util/OTWindowsRegistryTools.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
  )
else()
  add_library(${MODULE_NAME}
    ${cxx-sources}
    ${cxx-headers}
  )
endif()

target_link_libraries(opentxs-core LINK_PRIVATE opentxs-recurring opentxs-script opentxs-cron opentxs-trade otprotob irrxml bitcoin-base58)
target_link_libraries(opentxs-core LINK_PUBLIC bigint ${PROTOBUF_LIBRARY} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARY} ${OPENTXS_SYSTEM_LIBRARIES} ${keyring})

set_target_properties(opentxs-core PROPERTIES VERSION "${VERSION_STRING}" SOVERSION "${VERSION_MAJOR}")

if(WIN32)
  add_custom_command(TARGET opentxs-core POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/opentxs-core.dll"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/opentxs-core.dll")

  add_custom_command(TARGET opentxs-core POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/opentxs-core.pdb"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/opentxs-core.pdb")
endif()

if(NOT WIN32)
  include(GNUInstallDirs)
  install(TARGETS opentxs-core
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          EXPORT opentxsTargets
          COMPONENT main)
endif()
