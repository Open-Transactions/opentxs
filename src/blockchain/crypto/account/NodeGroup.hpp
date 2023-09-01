// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_shared_guarded.h>
#include <shared_mutex>

#include "blockchain/crypto/account/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace crypto
{
class Account;
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::account
{
template <typename InterfaceType, typename PayloadType>
class NodeGroup final : virtual public InterfaceType
{
public:
    using const_iterator = typename InterfaceType::const_iterator;
    using value_type = typename InterfaceType::value_type;

    auto all() const noexcept -> UnallocatedSet<identifier::Account> final
    {
        auto out = UnallocatedSet<identifier::Account>{};
        auto handle = data_.lock_shared();

        for (const auto& [id, count] : handle->index_) { out.emplace(id); }

        return out;
    }
    auto at(const std::size_t position) const -> const value_type& final
    {
        return *data_.lock_shared()->nodes_.at(position);
    }
    auto at(const identifier::Account& id) const -> const PayloadType& final
    {
        auto handle = data_.lock_shared();

        return *handle->nodes_.at(handle->index_.at(id));
    }
    auto begin() const noexcept -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto cbegin() const noexcept -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto cend() const noexcept -> const_iterator final
    {
        return const_iterator(this, data_.lock_shared()->nodes_.size());
    }
    auto end() const noexcept -> const_iterator final
    {
        return const_iterator(this, data_.lock_shared()->nodes_.size());
    }
    template <typename Invokable>
    auto for_each(Invokable cb) const noexcept(false) -> void
    {
        auto handle = data_.lock_shared();
        const auto& items = handle->nodes_;
        std::for_each(items.begin(), items.end(), [&](const auto& i) {
            std::invoke(cb, *i);
        });
    }
    auto size() const noexcept -> std::size_t final
    {
        return data_.lock_shared()->nodes_.size();
    }
    auto Type() const noexcept -> SubaccountType final { return type_; }

    auto at(const std::size_t position) -> value_type&
    {
        return *data_.lock()->nodes_.at(position);
    }
    auto at(const identifier::Account& id) -> PayloadType&
    {
        auto handle = data_.lock();

        return *handle->nodes_.at(handle->index_.at(id));
    }
    template <typename... Args>
    auto Construct(identifier::Account& out, const Args&... args) noexcept
        -> bool
    {
        auto handle = data_.lock();

        return construct(*handle, out, args...);
    }

    NodeGroup(
        const api::Session& api,
        const SubaccountType type,
        Account& parent) noexcept
        : api_(api)
        , type_(type)
        , parent_(parent)
        , data_()
    {
    }

private:
    struct Data {
        Vector<std::unique_ptr<PayloadType>> nodes_{};
        Map<identifier::Account, std::size_t> index_{};
    };

    using Guarded = libguarded::shared_guarded<Data, std::shared_mutex>;

    const api::Session& api_;
    const SubaccountType type_;
    Account& parent_;
    mutable Guarded data_;

    auto add(
        Data& data,
        const identifier::Account& id,
        std::unique_ptr<PayloadType> node) noexcept -> bool
    {
        if (false == bool(node)) {
            LogError()(OT_PRETTY_CLASS())("Invalid node").Flush();

            return false;
        }

        if (data.index_.contains(id)) {
            LogError()(OT_PRETTY_CLASS())("Index already exists").Flush();

            return false;
        }

        data.nodes_.emplace_back(std::move(node));
        const auto position = data.nodes_.size() - 1_uz;
        data.index_.emplace(id, position);

        return true;
    }
    template <typename... Args>
    auto construct(
        Data& data,
        identifier::Account& id,
        const Args&... args) noexcept -> bool
    {
        auto node{
            Factory<PayloadType, Args...>::get(api_, parent_, id, args...)};

        if (false == bool(node)) { return false; }

        if (data.index_.contains(id)) {
            LogTrace()(OT_PRETTY_CLASS())("subaccount ")(id, api_.Crypto())(
                " already exists")
                .Flush();

            return false;
        } else {
            LogTrace()(OT_PRETTY_CLASS())("subaccount ")(id, api_.Crypto())(
                " created")
                .Flush();
        }

        return add(data, id, std::move(node));
    }
};
}  // namespace opentxs::blockchain::crypto::account
