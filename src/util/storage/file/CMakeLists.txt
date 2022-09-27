# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(LMDB_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/util/storage/file/Index.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/util/storage/file/Mapped.hpp"
      "Index.cpp"
      "Mapped.cpp"
      "MappedPrivate.cpp"
      "MappedPrivate.hpp"
  )
  target_link_libraries(opentxs-common PRIVATE Boost::headers)
  target_link_libraries(opentxs PUBLIC Boost::iostreams)
  target_include_directories(
    opentxs-common SYSTEM
    PRIVATE "${opentxs_SOURCE_DIR}/deps/cs_libguarded/src"
  )
endif()
