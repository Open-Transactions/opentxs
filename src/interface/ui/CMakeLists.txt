# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(accountactivity)
add_subdirectory(accountlist)
add_subdirectory(accountsummary)
add_subdirectory(accounttree)
add_subdirectory(activitysummary)
add_subdirectory(activitythread)
add_subdirectory(base)
add_subdirectory(blockchainaccountstatus)
add_subdirectory(blockchainselection)
add_subdirectory(blockchainstatistics)
add_subdirectory(contact)
add_subdirectory(contactlist)
add_subdirectory(messagablelist)
add_subdirectory(nymlist)
add_subdirectory(payablelist)
add_subdirectory(profile)
add_subdirectory(seedlist)
add_subdirectory(seedtree)
add_subdirectory(unitlist)

libopentxs_add_public_header("interface/ui" "Blockchains.hpp")
libopentxs_add_public_header("interface/ui" "Types.hpp")

target_sources(
  opentxs-common
  PRIVATE "${opentxs_SOURCE_DIR}/src/internal/interface/ui/UI.hpp" "UI.cpp"
)
