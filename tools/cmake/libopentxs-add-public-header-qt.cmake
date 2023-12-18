# Copyright (c) 2020-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

function(libopentxs_add_public_header_qt header_name)
  include(libopentxs-add-public-header)
  include(libopentxs-add-public-moc-header)
  libopentxs_add_public_header("interface/qt" "${header_name}")
  libopentxs_add_public_moc_header(
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/${header_name}"
  )
endfunction()
