# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(FS_EXPORT)
  target_sources(
    opentxs-common
    PRIVATE
      "Archiving.cpp"
      "Archiving.hpp"
      "Common.cpp"
      "Common.hpp"
      "GarbageCollected.cpp"
      "GarbageCollected.hpp"
  )
  libopentxs_add_platform_specific("Common")
  libopentxs_link_external(Boost::iostreams)
else()
  target_sources(opentxs-common PRIVATE "null.cpp")
endif()
