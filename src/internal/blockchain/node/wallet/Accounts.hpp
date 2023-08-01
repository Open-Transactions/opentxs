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

class Manager;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Accounts
{
public:
    class Imp;

    auto Init() noexcept -> void;

    Accounts(
        std::shared_ptr<const api::session::Client> api,
        std::shared_ptr<const node::Manager> node) noexcept;
    Accounts(const Accounts&) = delete;
    Accounts(Accounts&&) = delete;
    auto operator=(const Accounts&) -> Accounts& = delete;
    auto operator=(Accounts&&) -> Accounts& = delete;

    ~Accounts();

private:
    // TODO switch to std::shared_ptr once the android ndk ships a version of
    // libc++ with unfucked pmr / allocate_shared support
    boost::shared_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::node::wallet
