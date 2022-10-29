# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/crypto/Crypto.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/blockchain/crypto/Factory.hpp"
    "Account.cpp"
    "Account.hpp"
    "Account.tpp"
    "AccountIndex.cpp"
    "AccountIndex.hpp"
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
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/crypto/Account.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/crypto/AddressStyle.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/crypto/Deterministic.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/crypto/Element.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/crypto/Ethereum.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/crypto/HD.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/crypto/HDProtocol.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/crypto/Imported.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/crypto/Notification.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/crypto/PaymentCode.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/crypto/Subaccount.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/crypto/SubaccountType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/crypto/Subchain.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/crypto/Types.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/blockchain/crypto/Wallet.hpp"
)

target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/blockchain/crypto
    COMPONENT dev
  )
endif()
