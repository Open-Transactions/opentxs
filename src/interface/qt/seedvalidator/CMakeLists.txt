# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

libopentxs_add_public_header_qt("SeedValidator.hpp")
target_sources(opentxs-common PRIVATE "SeedValidator.cpp")
target_link_libraries(opentxs-moc PRIVATE Qt::Gui)
libopentxs_link_internal(Qt::Gui)
libopentxs_link_external(Qt::Gui)

set(OPENTXS_QT_PUBLIC_HEADERS
    ${OPENTXS_QT_PUBLIC_HEADERS}
    PARENT_SCOPE
)
set(OPENTXS_QT_PRIVATE_HEADERS
    ${OPENTXS_QT_PRIVATE_HEADERS}
    PARENT_SCOPE
)
