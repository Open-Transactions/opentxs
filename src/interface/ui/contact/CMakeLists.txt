# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/interface/ui/Contact.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/interface/ui/ContactItem.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/interface/ui/ContactSection.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/interface/ui/ContactSubsection.hpp"
    "Contact.cpp"
    "Contact.hpp"
    "ContactItem.cpp"
    "ContactItem.hpp"
    "ContactSection.cpp"
    "ContactSection.hpp"
    "ContactSubsection.cpp"
    "ContactSubsection.hpp"
)
