// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <atomic>
#include <functional>
#include <memory>
#include <string_view>

#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "util/LMDB.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace crypto
{
class Account;
}  // namespace crypto

namespace node
{
namespace internal
{
struct Mempool;
struct Network;
struct WalletDatabase;
}  // namespace internal
}  // namespace node
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Account
{
public:
    enum class State {
        normal,
        pre_reorg,
        reorg,
        post_reorg,
        pre_shutdown,
        shutdown,
    };

    auto VerifyState(const State state) const noexcept -> void;

    auto ProcessReorg(
        const Lock& headerOracleLock,
        storage::lmdb::LMDB::Transaction& tx,
        std::atomic_int& errors,
        const block::Position& parent) noexcept -> void;

    Account(
        const api::Session& api,
        const crypto::Account& account,
        const node::internal::Network& node,
        const node::internal::WalletDatabase& db,
        const node::internal::Mempool& mempool,
        const Type chain,
        const cfilter::Type filter,
        const std::string_view fromParent,
        const std::string_view toParent) noexcept;
    Account(Account&&) noexcept;

    ~Account();

private:
    class Imp;

    // TODO switch to std::shared_ptr once the android ndk ships a version of
    // libc++ with unfucked pmr / allocate_shared support
    boost::shared_ptr<Imp> imp_;

    Account(const Account&) = delete;
    auto operator=(const Account&) -> Account& = delete;
    auto operator=(Account&&) -> Account& = delete;
};
}  // namespace opentxs::blockchain::node::wallet
