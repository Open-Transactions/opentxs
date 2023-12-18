# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header_qt("IdentityManager.hpp")
target_sources(
  opentxs-common PRIVATE "IdentityManager.cpp" "IdentityManager.hpp"
)
target_link_libraries(opentxs-moc PRIVATE Qt::Core)
libopentxs_link_internal(Qt::Core)
libopentxs_link_external(Qt::Core)
