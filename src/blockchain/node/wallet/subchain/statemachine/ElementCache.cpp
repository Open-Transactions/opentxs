// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/subchain/statemachine/ElementCache.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstring>
#include <iterator>
#include <memory>

#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Outpoint.hpp"        // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"          // IWYU pragma: keep
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
    log_(OT_PRETTY_CLASS())("caching ")(data.size())(" patterns").Flush();
    Add(convert(std::move(data), alloc));
    std::transform(
        txos.begin(),
        txos.end(),
        std::inserter(elements_.txos_, elements_.txos_.end()),
        [](auto& utxo) {
            return std::make_pair(utxo.first, std::move(utxo.second));
        });
    log_(OT_PRETTY_CLASS())("cache contains:").Flush();
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
                const auto exists =
                    std::find(
                        existingValues.begin(), existingValues.end(), value) !=
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
    database::TXOs&& created,
    database::TXOs&& consumed) noexcept -> void
{
    for (auto& [outpoint, output] : created) {
        auto& map = elements_.txos_;

        if (auto i = map.find(outpoint); map.end() != i) {
            i->second = std::move(output);
        } else {
            map.emplace(outpoint, std::move(output));
        }
    }

    for (const auto& [outpoint, output] : consumed) {
        elements_.txos_.erase(outpoint);
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
            OT_FAIL;
        }
    }
}

ElementCache::~ElementCache() = default;
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
ElementCache::Elements::Elements(allocator_type alloc) noexcept
    : elements_20_(alloc)
    , elements_32_(alloc)
    , elements_33_(alloc)
    , elements_64_(alloc)
    , elements_65_(alloc)
    , txos_(alloc)

{
}

ElementCache::Elements::Elements(
    const Elements& rhs,
    allocator_type alloc) noexcept
    : Elements(alloc)
{
    operator=(rhs);
}

ElementCache::Elements::Elements(Elements&& rhs, allocator_type alloc) noexcept
    : Elements(alloc)
{
    operator=(std::move(rhs));
}

ElementCache::Elements::Elements(Elements&& rhs) noexcept
    : Elements(std::move(rhs), rhs.get_allocator())
{
}

auto ElementCache::Elements::get_allocator() const noexcept -> allocator_type
{
    return elements_20_.get_allocator();
}

auto ElementCache::Elements::operator=(const Elements& rhs) noexcept
    -> Elements&
{
    elements_20_ = rhs.elements_20_;
    elements_32_ = rhs.elements_32_;
    elements_33_ = rhs.elements_33_;
    elements_64_ = rhs.elements_64_;
    elements_65_ = rhs.elements_65_;
    txos_ = rhs.txos_;

    return *this;
}

auto ElementCache::Elements::operator=(Elements&& rhs) noexcept -> Elements&
{
    elements_20_ = std::move(rhs.elements_20_);
    elements_32_ = std::move(rhs.elements_32_);
    elements_33_ = std::move(rhs.elements_33_);
    elements_64_ = std::move(rhs.elements_64_);
    elements_65_ = std::move(rhs.elements_65_);
    txos_ = std::move(rhs.txos_);

    return *this;
}

auto ElementCache::Elements::size() const noexcept -> std::size_t
{
    return elements_20_.size() + elements_32_.size() + elements_33_.size() +
           elements_64_.size() + elements_65_.size() + txos_.size();
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
MatchCache::MatchCache(allocator_type alloc) noexcept
    : results_(alloc)
{
}

auto MatchCache::Add(Results&& results) noexcept -> void
{
    for (auto& [block, index] : results) {
        results_[block].Merge(std::move(index));
    }
}

auto MatchCache::Forget(const block::Position& last) noexcept -> void
{
    auto& map = results_;
    map.erase(map.begin(), map.upper_bound(last));
}

auto MatchCache::get_allocator() const noexcept -> allocator_type
{
    return results_.get_allocator();
}

auto MatchCache::GetMatches(const block::Position& block) const noexcept
    -> std::optional<Index>
{
    if (auto i = results_.find(block); results_.end() == i) {

        return std::nullopt;
    } else {

        return i->second;
    }
}

auto MatchCache::Reset() noexcept -> void { results_.clear(); }
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
MatchCache::Index::Index(allocator_type alloc) noexcept
    : confirmed_no_match_(alloc)
    , confirmed_match_(alloc)
{
}

MatchCache::Index::Index(const Index& rhs, allocator_type alloc) noexcept
    : Index(alloc)
{
    operator=(rhs);
}

MatchCache::Index::Index(Index&& rhs, allocator_type alloc) noexcept
    : Index(alloc)
{
    operator=(std::move(rhs));
}

MatchCache::Index::Index(Index&& rhs) noexcept
    : Index(std::move(rhs), rhs.get_allocator())
{
}

auto MatchCache::Index::get_allocator() const noexcept -> allocator_type
{
    return confirmed_no_match_.get_allocator();
}

auto MatchCache::Index::Merge(Index&& rhs) noexcept -> void
{
    confirmed_no_match_.Merge(std::move(rhs.confirmed_no_match_));
    confirmed_match_.Merge(std::move(rhs.confirmed_match_));
}

auto MatchCache::Index::operator=(const Index& rhs) noexcept -> Index&
{
    confirmed_no_match_ = rhs.confirmed_no_match_;
    confirmed_match_ = rhs.confirmed_match_;

    return *this;
}

auto MatchCache::Index::operator=(Index&& rhs) noexcept -> Index&
{
    confirmed_no_match_ = std::move(rhs.confirmed_no_match_);
    confirmed_match_ = std::move(rhs.confirmed_match_);

    return *this;
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
MatchCache::Matches::Matches(allocator_type alloc) noexcept
    : match_20_(alloc)
    , match_32_(alloc)
    , match_33_(alloc)
    , match_64_(alloc)
    , match_65_(alloc)
    , match_txo_(alloc)
{
}

MatchCache::Matches::Matches(const Matches& rhs, allocator_type alloc) noexcept
    : Matches(alloc)
{
    operator=(rhs);
}

MatchCache::Matches::Matches(Matches&& rhs, allocator_type alloc) noexcept
    : Matches(alloc)
{
    operator=(std::move(rhs));
}

MatchCache::Matches::Matches(Matches&& rhs) noexcept
    : Matches(std::move(rhs), rhs.get_allocator())
{
}

auto MatchCache::Matches::get_allocator() const noexcept -> allocator_type
{
    return match_20_.get_allocator();
}

auto MatchCache::Matches::Merge(Matches&& rhs) noexcept -> void
{
    std::move(
        rhs.match_20_.begin(),
        rhs.match_20_.end(),
        std::inserter(match_20_, match_20_.end()));
    std::move(
        rhs.match_32_.begin(),
        rhs.match_32_.end(),
        std::inserter(match_32_, match_32_.end()));
    std::move(
        rhs.match_33_.begin(),
        rhs.match_33_.end(),
        std::inserter(match_33_, match_33_.end()));
    std::move(
        rhs.match_64_.begin(),
        rhs.match_64_.end(),
        std::inserter(match_64_, match_64_.end()));
    std::move(
        rhs.match_65_.begin(),
        rhs.match_65_.end(),
        std::inserter(match_65_, match_65_.end()));
    std::move(
        rhs.match_txo_.begin(),
        rhs.match_txo_.end(),
        std::inserter(match_txo_, match_txo_.end()));
}

auto MatchCache::Matches::operator=(const Matches& rhs) noexcept -> Matches&
{
    match_20_ = rhs.match_20_;
    match_32_ = rhs.match_32_;
    match_33_ = rhs.match_33_;
    match_64_ = rhs.match_64_;
    match_65_ = rhs.match_65_;
    match_txo_ = rhs.match_txo_;

    return *this;
}

auto MatchCache::Matches::operator=(Matches&& rhs) noexcept -> Matches&
{
    match_20_ = std::move(rhs.match_20_);
    match_32_ = std::move(rhs.match_32_);
    match_33_ = std::move(rhs.match_33_);
    match_64_ = std::move(rhs.match_64_);
    match_65_ = std::move(rhs.match_65_);
    match_txo_ = std::move(rhs.match_txo_);

    return *this;
}
}  // namespace opentxs::blockchain::node::wallet
