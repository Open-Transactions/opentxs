// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/subchain/statemachine/ElementCache.hpp"  // IWYU pragma: associated

#include <boost/container/vector.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iterator>
#include <ranges>
#include <utility>

#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node::wallet
{
ElementCache::ElementCache(
    block::Patterns&& data,
    Vector<database::UTXO>&& txos,
    allocator_type alloc) noexcept
    : log_(LogTrace())
    , data_(alloc)
    , elements_(alloc)
{
    log_()("caching ")(data.size())(" patterns").Flush();
    Add(convert(std::move(data), alloc));
    std::ranges::transform(
        txos,
        std::inserter(elements_.txos_, elements_.txos_.end()),
        [](auto& utxo) {
            return std::make_pair(utxo.first, std::move(utxo.second));
        });
    log_()("cache contains:").Flush();
    log_("  * ")(elements_.elements_20_.size())(" 20 byte elements").Flush();
    log_("  * ")(elements_.elements_32_.size())(" 32 byte elements").Flush();
    log_("  * ")(elements_.elements_33_.size())(" 33 byte elements").Flush();
    log_("  * ")(elements_.elements_64_.size())(" 64 byte elements").Flush();
    log_("  * ")(elements_.elements_65_.size())(" 65 byte elements").Flush();
    log_("  * ")(elements_.txos_.size())(" txo elements").Flush();
}

auto ElementCache::Add(database::ElementMap&& data) noexcept -> void
{
    for (auto& i : data) {
        auto& [incomingKey, incomingValues] = i;

        if (auto j = data_.find(incomingKey); data_.end() == j) {
            index(i);
            data_.try_emplace(incomingKey, std::move(incomingValues));
        } else {
            auto& [existingKey, existingValues] = *j;

            for (auto& value : incomingValues) {
                const auto exists = std::ranges::find(existingValues, value) !=
                                    existingValues.end();

                if (exists) {

                    continue;
                } else {
                    index(incomingKey, value);
                    existingValues.emplace_back(std::move(value));
                }
            }
        }
    }
}

auto ElementCache::Add(
    const database::ConsumedTXOs& consumed,
    database::TXOs&& created) noexcept -> void
{
    Add(std::move(created));
    auto& map = elements_.txos_;

    for (const auto& outpoint : consumed) { map.erase(outpoint); }
}

auto ElementCache::Add(database::TXOs&& created) noexcept -> void
{
    auto& map = elements_.txos_;

    for (auto& [outpoint, output] : created) {
        if (auto i = map.find(outpoint); map.end() != i) {
            i->second = std::move(output);
        } else {
            map.emplace(outpoint, std::move(output));
        }
    }
}

auto ElementCache::convert(block::Patterns&& in, allocator_type alloc) noexcept
    -> database::ElementMap
{
    auto out = database::ElementMap{alloc};

    for (auto& [id, item] : in) {
        const auto& [index, subchain] = id;
        out[index].emplace_back(std::move(item));
    }

    return out;
}

auto ElementCache::GetElements() const noexcept -> const Elements&
{
    return elements_;
}

auto ElementCache::get_allocator() const noexcept -> allocator_type
{
    return data_.get_allocator();
}

auto ElementCache::index(const database::ElementMap::value_type& data) noexcept
    -> void
{
    const auto& [key, values] = data;

    for (const auto& value : values) { index(key, value); }
}

auto ElementCache::index(
    const Bip32Index index,
    const block::Element& element) noexcept -> void
{
    switch (element.size()) {
        case 20: {
            auto& data = elements_.elements_20_.emplace_back(
                index, std::array<std::byte, 20>{});
            std::memcpy(data.second.data(), element.data(), 20);
        } break;
        case 33: {
            auto& data = elements_.elements_33_.emplace_back(
                index, std::array<std::byte, 33>{});
            std::memcpy(data.second.data(), element.data(), 33);
        } break;
        case 32: {
            auto& data = elements_.elements_32_.emplace_back(
                index, std::array<std::byte, 32>{});
            std::memcpy(data.second.data(), element.data(), 32);
        } break;
        case 65: {
            auto& data = elements_.elements_65_.emplace_back(
                index, std::array<std::byte, 65>{});
            std::memcpy(data.second.data(), element.data(), 65);
        } break;
        case 64: {
            auto& data = elements_.elements_64_.emplace_back(
                index, std::array<std::byte, 64>{});
            std::memcpy(data.second.data(), element.data(), 64);
        } break;
        default: {
            LogAbort()().Abort();
        }
    }
}

ElementCache::~ElementCache() = default;
}  // namespace opentxs::blockchain::node::wallet
