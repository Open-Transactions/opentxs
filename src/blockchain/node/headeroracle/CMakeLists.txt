# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/HeaderOracle.hpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/headeroracle/HeaderJob.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/headeroracle/HeaderOracle.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/headeroracle/Types.hpp"
      "Actor.cpp"
      "Actor.hpp"
      "HeaderJob.cpp"
      "HeaderOracle.cpp"
      "HeaderOraclePrivate.cpp"
      "HeaderOraclePrivate.hpp"
      "Shared.cpp"
      "Shared.hpp"
  )
  target_link_libraries(opentxs-common PUBLIC Boost::headers)
  target_link_libraries(opentxs PUBLIC Boost::system)
  target_include_directories(
    opentxs-common SYSTEM
    PRIVATE "${opentxs_SOURCE_DIR}/deps/cs_libguarded/src"
  )
endif()

target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/node
    COMPONENT dev
  )
endif()
