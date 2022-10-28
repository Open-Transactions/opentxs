# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(SECP256K1_EXPORT AND BIP32_EXPORT)
  target_sources(opentxs-testlib PRIVATE "Helpers.hpp" "Helpers.cpp")
endif()
