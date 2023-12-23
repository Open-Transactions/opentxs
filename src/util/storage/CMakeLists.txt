# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(drivers)
add_subdirectory(file)
if(LMDB_EXPORT)
  add_subdirectory(lmdb)
endif()
add_subdirectory(tree)

libopentxs_add_public_header("util/storage" "Driver.hpp")

target_sources(opentxs-common PRIVATE "Config.cpp" "Config.hpp")

if(LMDB_EXPORT)
  target_sources(opentxs-common PRIVATE "config/DefaultLMDB.cpp")
elseif(SQLITE_EXPORT)
  target_sources(opentxs-common PRIVATE "config/DefaultSqlite.cpp")
elseif(FS_EXPORT)
  target_sources(opentxs-common PRIVATE "config/DefaultFS.cpp")
else()
  target_sources(opentxs-common PRIVATE "config/DefaultNone.cpp")
endif()
