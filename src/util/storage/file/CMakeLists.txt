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
      "${opentxs_SOURCE_DIR}/src/internal/util/storage/file/Reader.hpp"
      "${opentxs_SOURCE_DIR}/src/internal/util/storage/file/Types.hpp"
      "Common.cpp"
      "Index.cpp"
      "Mapped.cpp"
      "MappedPrivate.cpp"
      "MappedPrivate.hpp"
      "Reader.cpp"
      "ReaderPrivate.cpp"
      "ReaderPrivate.hpp"
  )
  opentxs_link_external(Boost::iostreams)
endif()
