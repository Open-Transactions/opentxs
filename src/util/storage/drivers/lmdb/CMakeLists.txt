# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(LMDB_EXPORT)
  target_sources(opentxs-common PRIVATE "LMDB.cpp" "LMDB.hpp")
  libopentxs_link_internal("${OT_LMDB_TARGET}")
  libopentxs_link_external("${OT_LMDB_TARGET}")
else()
  target_sources(opentxs-common PRIVATE "null.cpp")
endif()
