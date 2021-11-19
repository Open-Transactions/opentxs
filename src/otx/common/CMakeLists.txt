# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/otx/common/Contract.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/otx/common/XML.hpp"
    "Contract.cpp"
    "Contract.hpp"
    "XML.cpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/otx/common/Contract.hpp"
)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/api
    COMPONENT dev
  )
endif()