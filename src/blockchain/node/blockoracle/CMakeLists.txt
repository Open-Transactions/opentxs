# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/BlockOracle.hpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/BlockBatch.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/BlockOracle.hpp"
      "BlockBatch.cpp"
      "BlockBatch.hpp"
      "BlockDownloader.cpp"
      "BlockDownloader.hpp"
      "BlockOracle.cpp"
      "BlockOracle.hpp"
      "Cache.cpp"
      "Cache.hpp"
      "MemDB.cpp"
      "MemDB.hpp"
  )
  target_link_libraries(opentxs-common PRIVATE Boost::headers)
  target_link_libraries(opentxs PUBLIC Boost::system)
  target_include_directories(
    opentxs-common SYSTEM
    PRIVATE "${opentxs_SOURCE_DIR}/deps/cs_libguarded/src"
  )

  if(PACKETCRYPT_EXPORT)
    target_sources(opentxs-common PRIVATE "GetValidator_packetcrypt.cpp")
  else()
    target_sources(opentxs-common PRIVATE "GetValidator.cpp")
  endif()
endif()

target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/node
    COMPONENT dev
  )
endif()
