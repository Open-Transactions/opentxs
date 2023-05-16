# Copyright (c) 2020-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

macro(libopentxs_add_public_header_qt header_name)
  include(libopentxs-add-public-header)
  libopentxs_add_public_header("interface/qt" "${header_name}")
  list(
    APPEND
    OPENTXS_QT_PUBLIC_HEADERS
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/${header_name}"
  )
endmacro()
