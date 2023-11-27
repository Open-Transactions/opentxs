# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(base)
add_subdirectory(client)
add_subdirectory(notary)

libopentxs_add_public_header("api/session" "Client.hpp")
libopentxs_add_public_header("api/session" "Factory.hpp")
libopentxs_add_public_header("api/session" "Notary.hpp")
libopentxs_add_public_header("api/session" "Wallet.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "Client.cpp"
    "Client.factory.cpp"
    "Client.internal.hpp"
    "ClientPrivate.cpp"
    "ClientPrivate.hpp"
    "Factory.cpp"
    "Factory.internal.cpp"
    "Factory.internal.hpp"
    "FactoryPrivate.cpp"
    "FactoryPrivate.hpp"
    "Notary.cpp"
    "Notary.factory.cpp"
    "Notary.internal.hpp"
    "NotaryPrivate.cpp"
    "NotaryPrivate.hpp"
    "Wallet.cpp"
    "Wallet.internal.cpp"
    "Wallet.internal.hpp"
    "WalletPrivate.cpp"
    "WalletPrivate.hpp"
    "ZeroMQ.cpp"
    "ZeroMQ.factory.cpp"
    "ZeroMQ.internal.cpp"
    "ZeroMQ.internal.hpp"
    "ZeroMQPrivate.cpp"
    "ZeroMQPrivate.hpp"
    "internal.factory.hpp"
)
