# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_QML_EXPORT)
  target_sources(opentxs-common PRIVATE "Qml.cpp")
  opentxs_link_internal(Qt::Qml)
  opentxs_link_external(Qt::Qml)
else()
  target_sources(opentxs-common PRIVATE "Null.cpp")
endif()

if(OT_QT_EXPORT)
  opentxs_link_internal(Qt::Core)
  opentxs_link_external(Qt::Core)
endif()
