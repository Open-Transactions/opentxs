# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/api/session/UI.hpp"
    "Imp-base.cpp"
    "Imp-base.hpp"
    "UI.cpp"
    "UI.hpp"
    "UpdateManager.cpp"
    "UpdateManager.hpp"
)

if(OT_QT_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "Imp-qt.cpp"
      "Imp-qt.hpp"
      "Qt.cpp"
  )
  opentxs_link_internal(Qt::Core)
  opentxs_link_internal(Qt::Gui)
  opentxs_link_external(Qt::Core)
  opentxs_link_external(Qt::Gui)
else()
  target_sources(opentxs-common PRIVATE "No_Qt.cpp")
endif()
