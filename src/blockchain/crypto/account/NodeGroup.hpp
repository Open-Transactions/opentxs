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
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
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
using namespace std::literals;

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
    auto at(const std::size_t index) const noexcept(false)
        -> const value_type& final
    {
        return at(*data_.lock_shared(), index);
    }
    auto at(const identifier::Account& id) const noexcept(false)
        -> const PayloadType& final
    {
        auto handle = data_.lock_shared();
        const auto index = [&] {
            const auto& map = handle->index_;

            if (auto i = map.find(id); map.end() != i) {

                return i->second;
            } else {

                throw std::runtime_error{
                    "account "s.append(id.asBase58(api_.Crypto()))
                        .append(" does not exist in index")};
            }
        }();

        return at(*handle, index);
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

    auto at(const std::size_t index) -> value_type&
    {
        return at(*data_.lock, index);
    }
    auto at(const identifier::Account& id) -> PayloadType&
    {
        auto handle = data_.lock();
        const auto index = [&] {
            const auto& map = handle->index_;

            if (auto i = map.find(id); map.end() != i) {

                return i->second;
            } else {

                throw std::runtime_error{
                    "account "s.append(id.asBase58(api_.Crypto()))
                        .append(" does not exist in index")};
            }
        }();

        return at(*handle, index);
    }
    template <typename... Args>
    auto Construct(identifier::Account& out, const Args&... args) noexcept
        -> bool
    {
        auto handle = data_.lock();

        if (handle->index_.contains(out)) { return true; }

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

    auto at(const Data& data, std::size_t index) const noexcept(false)
        -> const value_type&
    {
        const auto& vec = data.nodes_;

        if (index < vec.size()) {

            return *vec[index];
        } else {

            throw std::runtime_error{
                "index "s.append(std::to_string(index)).append(" is invalid")};
        }
    }

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

        OT_ASSERT(data.index_.contains(id));

        return true;
    }
    auto at(Data& data, std::size_t index) noexcept(false) -> value_type&
    {
        auto& vec = data.nodes_;

        if (index < vec.size()) {

            return *vec[index];
        } else {

            throw std::runtime_error{
                "index "s.append(std::to_string(index)).append(" is invalid")};
        }
    }
    template <typename... Args>
    auto construct(
        Data& data,
        identifier::Account& id,
        const Args&... args) noexcept -> bool
    {
        const auto& crypto = api_.Crypto();
        auto node{
            Factory<PayloadType, Args...>::get(api_, parent_, id, args...)};

        if (nullptr == node) {
            if (data.index_.contains(id)) {
                LogTrace()(OT_PRETTY_CLASS())("subaccount ")(id, crypto)(
                    " already exists")
                    .Flush();

                return true;
            } else {
                LogError()(OT_PRETTY_CLASS())("failed to construct subaccount")
                    .Flush();

                return false;
            }
        }

        if (data.index_.contains(id)) {
            LogTrace()(OT_PRETTY_CLASS())("subaccount ")(id, crypto)(
                " already exists")
                .Flush();

            return true;
        } else {
            LogTrace()(OT_PRETTY_CLASS())("subaccount ")(id, crypto)(" for ")(
                parent_.NymID(),
                crypto)(" on ")(print(parent_.Chain()))(" created or loaded")
                .Flush();
        }

        return add(data, id, std::move(node));
    }
};
}  // namespace opentxs::blockchain::crypto::account
