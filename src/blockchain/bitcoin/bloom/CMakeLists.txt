# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/bitcoin/bloom/BloomFilter.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/bitcoin/bloom/BloomUpdateFlag.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/bitcoin/bloom/Types.hpp"
)

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(opentxs-common PRIVATE "BloomFilter.cpp" "BloomFilter.hpp")
  target_link_libraries(opentxs-common PRIVATE Boost::headers)
endif()

target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/bitcoin/bloom
    COMPONENT dev
  )
endif()
