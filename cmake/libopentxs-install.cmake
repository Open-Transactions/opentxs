# Copyright (c) 2020-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

macro(libopentxs_install)
  if(OPENTXS_STANDALONE OR OT_INSTALL_LIBRARY_DEPENDENCIES)
    install(
      TARGETS opentxs
      DESTINATION "${CMAKE_INSTALL_LIBDIR}"
      EXPORT opentxs-targets
      COMPONENT opentxs_library
      FILE_SET HEADERS
      DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
      INCLUDES
      DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )
  endif()

  if(OT_INSTALL_LIBRARY_DEPENDENCIES)
    string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_SUFFIX)
    get_target_property(
      Boost_system_LIBRARY
      Boost::system
      IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
    )
    get_target_property(
      Boost_thread_LIBRARY
      Boost::thread
      IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
    )
    get_target_property(
      Protobuf_LITE_LIBRARY
      protobuf::libprotobuf-lite
      IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
    )
    get_target_property(
      ZLIB_LIBRARY
      ZLIB::ZLIB
      IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
    )

    if(OT_USE_VCPKG_TARGETS)
      get_target_property(
        ZMQ_LIBRARY_PATH
        "${OT_ZMQ_TARGET}"
        IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
      )
    else()
      set(ZMQ_LIBRARY_PATH "${ZMQ_LIBRARY}")
    endif()

    install(
      FILES
        "${Boost_system_LIBRARY}"
        "${Boost_thread_LIBRARY}"
        "${Protobuf_LITE_LIBRARY}"
        "${SODIUM_LIBRARY}"
        "${ZLIB_LIBRARY}"
        "${ZMQ_LIBRARY_PATH}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT opentxs_library
    )

    if(Boost_stacktrace_basic_FOUND)
      get_target_property(
        Boost_stacktrace_basic_LIBRARY
        Boost::stacktrace_basic
        IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
      )
      install(
        FILES "${Boost_stacktrace_basic_LIBRARY}"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT opentxs_library
      )
    endif()

    if(Boost_iostreams_FOUND)
      get_target_property(
        Boost_iostreams_LIBRARY
        Boost::iostreams
        IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
      )
      install(
        FILES "${Boost_iostreams_LIBRARY}"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT opentxs_library
      )
    endif()

    if(Boost_json_FOUND AND NOT OT_BOOST_JSON_HEADER_ONLY)
      get_target_property(
        Boost_json_LIBRARY
        Boost::json
        IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
      )
      install(
        FILES "${Boost_json_LIBRARY}"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT opentxs_library
      )
    endif()

    if(SQLITE_EXPORT)
      get_target_property(
        SQLITE3_LIBRARY
        SQLite::SQLite3
        IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
      )

      install(
        FILES "${SQLITE3_LIBRARY}"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT opentxs_library
      )
    endif(SQLITE_EXPORT)

    if(LMDB_EXPORT)
      install(
        FILES "${LMDB_LIBRARY}"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT opentxs_library
      )
    endif()

    if(OPENSSL_EXPORT)
      install(
        FILES "${OPENSSL_CRYPTO_LIBRARY}" "${OPENSSL_SSL_LIBRARY}"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT opentxs_library
      )
    endif()

    if(LIBSECP256K1_EXPORT AND NOT OT_BUNDLED_SECP256K1)
      install(
        FILES "${SECP256K1_LIBRARY}"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT opentxs_library
      )
    endif()
  endif()
endmacro()
