# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-testlib
  PRIVATE
    "Base.cpp"
    "Base.hpp"
    "Client.cpp"
    "Client.hpp"
    "Counter.cpp"
    "Counter.hpp"
    "LowLevel.cpp"
    "LowLevel.hpp"
    "Notary.cpp"
    "Notary.hpp"
    "OneClientSession.cpp"
    "OneClientSession.hpp"
    "Options.cpp"
    "Options.hpp"
    "PasswordCallback.cpp"
    "PasswordCallback.hpp"
    "User.cpp"
    "User.hpp"
)
