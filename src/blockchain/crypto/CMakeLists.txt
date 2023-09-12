# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(account)

libopentxs_add_public_header("blockchain/crypto" "AddressStyle.hpp")
libopentxs_add_public_header("blockchain/crypto" "Deterministic.hpp")
libopentxs_add_public_header("blockchain/crypto" "Element.hpp")
libopentxs_add_public_header("blockchain/crypto" "Ethereum.hpp")
libopentxs_add_public_header("blockchain/crypto" "HD.hpp")
libopentxs_add_public_header("blockchain/crypto" "HDProtocol.hpp")
libopentxs_add_public_header("blockchain/crypto" "Imported.hpp")
libopentxs_add_public_header("blockchain/crypto" "Notification.hpp")
libopentxs_add_public_header("blockchain/crypto" "PaymentCode.hpp")
libopentxs_add_public_header("blockchain/crypto" "Subaccount.hpp")
libopentxs_add_public_header("blockchain/crypto" "SubaccountType.hpp")
libopentxs_add_public_header("blockchain/crypto" "Subchain.hpp")
libopentxs_add_public_header("blockchain/crypto" "Types.hpp")
libopentxs_add_public_header("blockchain/crypto" "Wallet.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/crypto/Deterministic.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/crypto/Element.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/crypto/Factory.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/crypto/HD.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/crypto/Imported.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/crypto/Notification.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/crypto/PaymentCode.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/crypto/Subaccount.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/crypto/Wallet.hpp"
    "Crypto.cpp"
    "Deterministic.cpp"
    "Deterministic.hpp"
    "Element.cpp"
    "Element.hpp"
    "HD.cpp"
    "HD.hpp"
    "Notification.cpp"
    "Notification.hpp"
    "PaymentCode.cpp"
    "PaymentCode.hpp"
    "Subaccount.cpp"
    "Subaccount.hpp"
    "Wallet.cpp"
    "Wallet.hpp"
)
