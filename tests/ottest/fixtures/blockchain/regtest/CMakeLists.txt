# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-testlib
  PRIVATE
    "Base.cpp"
    "Base.hpp"
    "HD.cpp"
    "HD.hpp"
    "Normal.cpp"
    "Normal.hpp"
    "PaymentCode.cpp"
    "PaymentCode.hpp"
    "Simple.cpp"
    "Simple.hpp"
    "Single.cpp"
    "Single.hpp"
    "SyncClient.cpp"
    "SyncClient.hpp"
    "SyncServer.cpp"
    "SyncServer.hpp"
    "TCP.cpp"
    "TCP.hpp"
)
