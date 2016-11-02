# Copyright (c) Monetas AG, 2014

add_subdirectory(otprotob)
add_subdirectory(trade)
add_subdirectory(cron)
add_subdirectory(script)
add_subdirectory(recurring)

set(cxx-sources
  app/App.cpp
  app/Dht.cpp
  app/Identity.cpp
  app/Settings.cpp
  app/Wallet.cpp
  contract/basket/Basket.cpp
  contract/basket/BasketContract.cpp
  contract/basket/BasketItem.cpp
  contract/peer/BailmentNotice.cpp
  contract/peer/BailmentReply.cpp
  contract/peer/BailmentRequest.cpp
  contract/peer/NoticeAcknowledgement.cpp
  contract/peer/OutBailmentReply.cpp
  contract/peer/OutBailmentRequest.cpp
  contract/peer/PeerObject.cpp
  contract/peer/PeerReply.cpp
  contract/peer/PeerRequest.cpp
  contract/CurrencyContract.cpp
  contract/SecurityContract.cpp
  contract/ServerContract.cpp
  contract/Signable.cpp
  contract/UnitDefinition.cpp
  crypto/AsymmetricKeyEC.cpp
  crypto/AsymmetricKeyEd25519.cpp
  crypto/AsymmetricKeySecp256k1.cpp
  crypto/Bip32.cpp
  crypto/Bip39.cpp
  crypto/ChildKeyCredential.cpp
  crypto/ContactCredential.cpp
  crypto/Credential.cpp
  crypto/CredentialSet.cpp
  crypto/Crypto.cpp
  crypto/CryptoAsymmetric.cpp
  crypto/CryptoEncodingEngine.cpp
  crypto/CryptoEngine.cpp
  crypto/CryptoHash.cpp
  crypto/CryptoHashEngine.cpp
  crypto/CryptoSymmetric.cpp
  crypto/CryptoSymmetricEngine.cpp
  crypto/CryptoUtil.cpp
  crypto/Ecdsa.cpp
  crypto/KeyCredential.cpp
  crypto/Letter.cpp
  crypto/Libsecp256k1.cpp
  crypto/Libsodium.cpp
  crypto/LowLevelKeyGenerator.cpp
  crypto/MasterCredential.cpp
  crypto/NymParameters.cpp
  crypto/OTASCIIArmor.cpp
  crypto/OTAsymmetricKey.cpp
  crypto/OTAsymmetricKeyOpenSSL.cpp
  crypto/OTAsymmetricKeyOpenSSLPrivdp.cpp
  crypto/OTCachedKey.cpp
  crypto/OTCallback.cpp
  crypto/OTCaller.cpp
  crypto/OTEnvelope.cpp
  crypto/OTKeypair.cpp
  crypto/OTKeyring.cpp
  crypto/OTNymOrSymmetricKey.cpp
  crypto/OTPassword.cpp
  crypto/OTPasswordData.cpp
  crypto/OTSignatureMetadata.cpp
  crypto/OTSignedFile.cpp
  crypto/OTSymmetricKey.cpp
  crypto/OpenSSL.cpp
  crypto/PaymentCode.cpp
  crypto/SymmetricKey.cpp
  crypto/TrezorCrypto.cpp
  crypto/VerificationCredential.cpp
  crypto/mkcert.cpp
  transaction/Helpers.cpp
  util/Assert.cpp
  util/OTDataFolder.cpp
  util/OTFolders.cpp
  util/OTPaths.cpp
  util/StringUtils.cpp
  util/Tag.cpp
  util/Timer.cpp
  Account.cpp
  AccountList.cpp
  Cheque.cpp
  Contract.cpp
  Identifier.cpp
  Instrument.cpp
  Item.cpp
  Ledger.cpp
  Log.cpp
  Message.cpp
  NumList.cpp
  Nym.cpp
  NymIDSource.cpp
  OTData.cpp
  OTStorage.cpp
  OTStringXML.cpp
  OTTrackable.cpp
  OTTransaction.cpp
  OTTransactionType.cpp
  String.cpp
)

if (TREZORCRYPTO_EXPORT)
  set(trezor $<TARGET_OBJECTS:TrezorCrypto>)
else()
  set(trezor "")
endif()

set(object-deps
  $<TARGET_OBJECTS:base64>
  $<TARGET_OBJECTS:irrxml>
  $<TARGET_OBJECTS:lucre>
  ${trezor}
  $<TARGET_OBJECTS:otprotob>
  $<TARGET_OBJECTS:opentxs-cash>
  $<TARGET_OBJECTS:opentxs-cron>
  $<TARGET_OBJECTS:opentxs-ext>
  $<TARGET_OBJECTS:opentxs-network>
  $<TARGET_OBJECTS:opentxs-recurring>
  $<TARGET_OBJECTS:opentxs-script>
  $<TARGET_OBJECTS:opentxs-storage>
  $<TARGET_OBJECTS:opentxs-trade>
)

file(GLOB cxx-headers
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/app/*.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/contract/basket/*.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/contract/peer/*.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/contract/*.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/crypto/*.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/transaction/*.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/util/*.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/core/*.hpp"
)

include_directories(${ProtobufIncludePath})

if(KEYRING_GNOME)
  include(GNUInstallDirs)
  include_directories(${CMAKE_INSTALL_FULL_OLDINCLUDEDIR}/glib-2.0/ /usr/${CMAKE_INSTALL_LIBDIR}/glib-2.0/include/ ${CMAKE_INSTALL_FULL_OLDINCLUDEDIR}/gnome-keyring-1/)
endif()

include_directories(SYSTEM
  ${OPENSSL_INCLUDE_DIR}
  ${PROTOBUF_INCLUDE_DIR}
  ${ZLIB_INCLUDE_DIRS}
)

if (LIBSECP256K1_EXPORT AND NOT OT_INTERNAL_LIBSECP)
  include_directories(SYSTEM ${LIBSECP256K1_INCLUDE_DIR})
endif()

if (NOT OT_INTERNAL_ZMQ)
  include_directories(SYSTEM ${CZMQ_INCLUDE_DIRS})
endif()

set(MODULE_NAME opentxs-core)
if(WIN32)
  # suppress warnings about exported internal symbols (global log stream objects)
  add_definitions(-DOTLOG_IMPORT)

  # suppress linker warnings about locallly defined imported symbols
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4049 /ignore:4217")

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/windll.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
    @ONLY
  )

  # we have an extra Windows-specific file to compile
  add_library(${MODULE_NAME} STATIC
    ${cxx-sources}
    ${cxx-headers}
    ${object-deps}
    util/OTWindowsRegistryTools.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
  )
else()
  add_library(${MODULE_NAME} STATIC
    ${cxx-sources}
    ${cxx-headers}
    ${object-deps}
  )
endif()

target_link_libraries(${MODULE_NAME}
  PUBLIC
  ${OPENSSL_LIBRARIES}
  ${OPENTXS_PROTO_LIBRARIES}
  ${PROTOBUF_LITE_LIBRARIES}
  ${ZLIB_LIBRARY}
  ${SODIUM_LIBRARY}
  ${OPENTXS_SYSTEM_LIBRARIES}
  ${keyring}
)

if (OT_INTERNAL_ZMQ)
  add_library(static_zeromq SHARED IMPORTED)
  add_library(static_czmq SHARED IMPORTED)
  set_property(
    TARGET static_zeromq
    PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/deps/lib/libzmq${CMAKE_SHARED_LIBRARY_SUFFIX})
  set_property(
    TARGET static_czmq
    PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/deps/lib/libczmq${CMAKE_SHARED_LIBRARY_SUFFIX})
  target_link_libraries(${MODULE_NAME} PRIVATE static_zeromq)
  target_link_libraries(${MODULE_NAME} PRIVATE static_czmq)
else()
  target_link_libraries(${MODULE_NAME} PRIVATE ${CZMQ_LIBRARIES})
endif()

if (LIBSECP256K1_EXPORT)
  if (OT_INTERNAL_LIBSECP)
    add_library(staticlibsecp256k1 STATIC IMPORTED)
    set_property(
        TARGET staticlibsecp256k1
        PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/deps/lib/libsecp256k1.a)
    target_link_libraries(${MODULE_NAME} PRIVATE staticlibsecp256k1)
    target_link_libraries(${MODULE_NAME} PRIVATE ${GMP_LIBRARIES})
  else()
    target_link_libraries(${MODULE_NAME} PRIVATE ${LIBSECP256K1_LIBRARY})
  endif()
endif()

if(OT_DHT)
  target_link_libraries(${MODULE_NAME} PRIVATE ${OPENDHT_LIBRARIES} ${GNUTLS_LIBRARIES})
endif()

if (OT_STORAGE_SQLITE)
    target_link_libraries(${MODULE_NAME} PRIVATE ${SQLITE3_LIBRARIES})
endif()

if (OT_STORAGE_FS)
    target_link_libraries(${MODULE_NAME} PRIVATE ${Boost_SYSTEM_LIBRARIES} ${Boost_FILESYSTEM_LIBRARIES})
endif()

if(NOT WIN32)
  include(GNUInstallDirs)
  install(TARGETS ${MODULE_NAME}
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          EXPORT opentxsTargets
          COMPONENT main)
endif()

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})
