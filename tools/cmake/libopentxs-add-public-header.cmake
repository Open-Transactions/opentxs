# Copyright (c) 2020-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

function(
  libopentxs_add_public_header
  install_location
  header_name
)
  set(HEADER_FILE
      "${opentxs_SOURCE_DIR}/include/opentxs/${install_location}/${header_name}"
  )

  if(NOT
     EXISTS
     "${HEADER_FILE}"
  )
    set(HEADER_FILE
        "${opentxs_BINARY_DIR}/include/opentxs/${install_location}/${header_name}"
    )
  endif()

  target_sources(opentxs-common PRIVATE "${HEADER_FILE}")

  if(OT_INSTALL_HEADERS)
    target_sources(
      libopentxs
      PUBLIC
        FILE_SET
        HEADERS
        FILES
        "${HEADER_FILE}"
    )
  endif()
endfunction()
