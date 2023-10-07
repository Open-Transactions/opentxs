// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <memory>
#include <string_view>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace blockchain
{
namespace crypto
{
class Account;
}  // namespace crypto

namespace database
{
class Wallet;
}  // namespace database

namespace node
{
namespace internal
{
class Mempool;
}  // namespace internal

namespace wallet
{
class Reorg;
}  // namespace wallet

class Manager;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Account
{
public:
    auto Init() noexcept -> void;

    Account(
        Reorg& reorg,
        const crypto::Account& account,
        std::shared_ptr<const api::session::Client> api,
        std::shared_ptr<const node::Manager> node,
        std::string_view fromParent) noexcept;
    Account(const Account&) = delete;
    Account(Account&&) = delete;
    auto operator=(const Account&) -> Account& = delete;
    auto operator=(Account&&) -> Account& = delete;

    ~Account();

private:
    class Imp;

    std::shared_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::node::wallet
