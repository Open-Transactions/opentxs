# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(CASH_LUCRE_EXPORT)
  add_subdirectory(lucre)
endif()
add_subdirectory(mint)
add_subdirectory(purse)
add_subdirectory(token)

target_sources(
  opentxs-common
  PRIVATE "${opentxs_SOURCE_DIR}/src/internal/otx/blind/Factory.hpp"
)
