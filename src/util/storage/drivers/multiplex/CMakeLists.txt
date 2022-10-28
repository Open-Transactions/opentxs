# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(opentxs-common PRIVATE "Multiplex.cpp" "Multiplex.hpp")

if(FS_EXPORT)
  target_sources(opentxs-common PRIVATE "fs.cpp")
else()
  target_sources(opentxs-common PRIVATE "nofs.cpp")
endif()

if(SQLITE_EXPORT)
  target_sources(opentxs-common PRIVATE "sqlite.cpp")
  opentxs_link_internal(SQLite::SQLite3)
  opentxs_link_external(SQLite::SQLite3)
else()
  target_sources(opentxs-common PRIVATE "nosqlite.cpp")
endif()

if(LMDB_EXPORT)
  target_sources(opentxs-common PRIVATE "lmdb.cpp")
  opentxs_link_internal(lmdb)
  opentxs_link_external(lmdb)
else()
  target_sources(opentxs-common PRIVATE "nolmdb.cpp")
endif()
