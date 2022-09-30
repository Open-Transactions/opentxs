# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/Work.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OT_BLOCKCHAIN_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "WorkPrivate.cpp"
      "WorkPrivate.hpp"
      "Work.cpp"
  )
  target_link_libraries(opentxs-common PRIVATE Boost::headers)
else()
  target_sources(opentxs-common PRIVATE "Disabled.cpp")
endif()

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain
    COMPONENT dev
  )
endif()
