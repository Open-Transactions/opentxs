# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/util/storage/lmdb/Database.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/storage/lmdb/Transaction.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/storage/lmdb/Types.hpp"
    "Database.cpp"
    "DatabasePrivate.cpp"
    "DatabasePrivate.hpp"
    "Transaction.cpp"
    "TransactionPrivate.cpp"
    "TransactionPrivate.hpp"
)
libopentxs_link_internal("${OT_LMDB_TARGET}")
libopentxs_link_external("${OT_LMDB_TARGET}")
