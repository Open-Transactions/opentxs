# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/interface/ui/Profile.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/interface/ui/ProfileItem.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/interface/ui/ProfileSection.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/interface/ui/ProfileSubsection.hpp"
    "Profile.cpp"
    "Profile.hpp"
    "ProfileItem.cpp"
    "ProfileItem.hpp"
    "ProfileSection.cpp"
    "ProfileSection.hpp"
    "ProfileSubsection.cpp"
    "ProfileSubsection.hpp"
)
