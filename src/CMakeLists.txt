# Copyright (c) Monetas AG, 2014

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PEDANTIC_CXX_FLAGS}")

set(dependency_include_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/../deps/
)

include_directories(SYSTEM
  ${dependency_include_dir}
  ${dependency_include_dir}/ChaiScript/include
  ${dependency_include_dir}/lucre/include
)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}/../include
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

if(KEYRING_GNOME)
  set(keyring libgnome-keyring.so)
endif()

add_subdirectory(storage)
add_subdirectory(core)
add_subdirectory(cash)
add_subdirectory(ext)
add_subdirectory(network)
add_subdirectory(client)
add_subdirectory(server)

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
  $<TARGET_OBJECTS:opentxs-client>
  $<TARGET_OBJECTS:opentxs-core>
  $<TARGET_OBJECTS:opentxs-cron>
  $<TARGET_OBJECTS:opentxs-ext>
  $<TARGET_OBJECTS:opentxs-network>
  $<TARGET_OBJECTS:opentxs-recurring>
  $<TARGET_OBJECTS:opentxs-script>
  $<TARGET_OBJECTS:opentxs-server>
  $<TARGET_OBJECTS:opentxs-storage>
  $<TARGET_OBJECTS:opentxs-trade>
)

set(MODULE_NAME opentxs)

add_library(${MODULE_NAME} SHARED
  ${object-deps}
)

add_library(${MODULE_NAME}_static STATIC
  ${object-deps}
)

if(NOT WIN32)
  set_target_properties(${MODULE_NAME}_static PROPERTIES OUTPUT_NAME ${MODULE_NAME})
endif()

target_link_libraries(${MODULE_NAME}
  PRIVATE
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
  target_link_libraries(${MODULE_NAME} PUBLIC static_czmq)
else()
  target_link_libraries(${MODULE_NAME} PUBLIC ${CZMQ_LIBRARIES})
endif()

if (LIBSECP256K1_EXPORT)
  if (OT_INTERNAL_LIBSECP)
    add_library(staticlibsecp256k1 STATIC IMPORTED)
    set_property(
        TARGET staticlibsecp256k1
        PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/deps/lib/libsecp256k1.a)
    target_link_libraries(${MODULE_NAME} PRIVATE staticlibsecp256k1)
    target_link_libraries(${MODULE_NAME} PUBLIC ${GMP_LIBRARIES})
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

if(WIN32)
  add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/${MODULE_NAME}.dll"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/${MODULE_NAME}.dll")

  add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/${MODULE_NAME}.pdb"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/${MODULE_NAME}.pdb")
endif()

if(NOT WIN32)
  include(GNUInstallDirs)
  install(TARGETS ${MODULE_NAME} ${MODULE_NAME}_static
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          EXPORT opentxsTargets
          COMPONENT main)
endif()
