# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/interface/ui/SeedTreeNym.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/interface/ui/SeedTree.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/interface/ui/SeedTreeItem.hpp"
    "SeedTreeNym.cpp"
    "SeedTreeNym.hpp"
    "SeedTree.cpp"
    "SeedTree.hpp"
    "SeedTreeItem.cpp"
    "SeedTreeItem.hpp"
)
target_include_directories(
  opentxs-common SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/cs_libguarded/src"
)
