// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "blockchain/node/wallet/subchain/statemachine/Elements.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/block/Types.internal.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class ElementCache final : public Allocated
{
public:
    auto GetElements() const noexcept -> const Elements&;
    auto get_allocator() const noexcept -> allocator_type final;

    auto Add(database::ElementMap&& data) noexcept -> void;
    auto Add(
        const database::ConsumedTXOs& consumed,
        database::TXOs&& created) noexcept -> void;
    auto Add(database::TXOs&& created) noexcept -> void;
    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    ElementCache(
        block::Patterns&& data,
        Vector<database::UTXO>&& txos,
        allocator_type alloc) noexcept;

    ~ElementCache() final;

private:
    const Log& log_;
    database::ElementMap data_;
    Elements elements_;

    static auto convert(
        block::Patterns&& in,
        allocator_type alloc = {}) noexcept -> database::ElementMap;

    auto index(const database::ElementMap::value_type& data) noexcept -> void;
    auto index(
        const crypto::Bip32Index index,
        const block::Element& element) noexcept -> void;
};
}  // namespace opentxs::blockchain::node::wallet
