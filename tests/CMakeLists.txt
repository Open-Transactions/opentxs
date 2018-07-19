# Copyright (c) 2018 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../deps/)


add_subdirectory(api)
if(OT_CRYPTO_SUPPORTED_KEY_HD)
  add_subdirectory(blockchain)
endif()
add_subdirectory(client)
add_subdirectory(core)
add_subdirectory(contact)
add_subdirectory(crypto)
add_subdirectory(network/zeromq)
add_subdirectory(ui)
