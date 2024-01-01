# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_QML_EXPORT)
  target_sources(opentxs-common PRIVATE "Qml.cpp")
  libopentxs_link_internal(Qt::Qml)
  libopentxs_link_external(Qt::Qml)
  target_link_libraries(opentxs-moc PUBLIC Qt::Qml)
else()
  target_sources(opentxs-common PRIVATE "Null.cpp")
endif()

if(OT_QT_EXPORT)
  libopentxs_link_internal(Qt::Core)
  libopentxs_link_external(Qt::Core)
endif()
