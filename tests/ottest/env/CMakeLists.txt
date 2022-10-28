# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(basic)
add_subdirectory(lowlevel)

target_sources(opentxs-testlib PRIVATE "OTTestEnvironment.hpp" "main.cpp")
