# Copyright (c) 2020-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

function(libopentxs_add_public_moc_header file_name)
  get_target_property(
    OPENTXS_QT_PUBLIC_HEADERS_EXISTING
    opentxs-common
    QT_MOC_PUBLIC
  )
  list(
    APPEND
    OPENTXS_QT_PUBLIC_HEADERS_EXISTING
    "${file_name}"
  )
  set_target_properties(
    opentxs-common PROPERTIES QT_MOC_PUBLIC
                              "${OPENTXS_QT_PUBLIC_HEADERS_EXISTING}"
  )
endfunction()
