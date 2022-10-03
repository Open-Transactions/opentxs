# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/node/Manager.hpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/blockchain/node/Manager.hpp"
      "Manager.cpp"
      "Manager.hpp"
  )
  target_include_directories(
    opentxs-common SYSTEM
    PRIVATE "${opentxs_SOURCE_DIR}/deps/cs_libguarded/src"
  )
  target_link_libraries(opentxs-common PRIVATE "${OT_ZMQ_TARGET}")
  target_link_libraries(opentxs PUBLIC "${OT_ZMQ_TARGET}")
endif()

target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/node
    COMPONENT dev
  )
endif()
