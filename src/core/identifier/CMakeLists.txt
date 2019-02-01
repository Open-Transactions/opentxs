#[[
// clang-format off
]]#
# Copyright (c) 2018 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(MODULE_NAME opentxs-core-identifier)

set(cxx-install-headers
<<<<<<< HEAD
     "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/core/identifier/Nym.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/core/identifier/Server.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/core/identifier/UnitDefinition.hpp"
=======
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/core/identifier/Nym.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/core/identifier/Server.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/core/identifier/UnitDefinition.hpp"

>>>>>>> e8812e4f0... Use opentxs::identifier::Nym class for nym identifiers
)

install(FILES ${cxx-install-headers}
        DESTINATION include/opentxs/core/identifier
        COMPONENT dev)
#[[
// clang-format on
]]#
