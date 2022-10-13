// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"       // IWYU pragma: associated
#include "crypto/HDNode.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "util/Sodium.hpp"

namespace opentxs::crypto::implementation
{
HDNode::HDNode(const api::Crypto& crypto) noexcept
    : data_space_(Context().Factory().Secret(0))
    , hash_space_(Context().Factory().Secret(0))
    , data_(data_space_.WriteInto()(33 + 4))
    , hash_(hash_space_.WriteInto()(64))
    , crypto_(crypto)
    , switch_(0)
    , a_(Context().Factory().Secret(0))
    , b_(Context().Factory().Secret(0))
{
    check();

    {
        static const auto size = 32_uz + 32_uz + 33_uz;
        a_.WriteInto()(size);
        b_.WriteInto()(size);

        OT_ASSERT(size == a_.size());
        OT_ASSERT(size == b_.size());
    }
}

auto HDNode::Assign(const EcdsaCurve& curve, Bip32::Key& output) const
    noexcept(false) -> void
{
    auto& [privateKey, chainCode, publicKey, pathOut, parent] = output;
    const auto privateOut = ParentPrivate();
    const auto chainOut = ParentCode();
    const auto publicOut = ParentPublic();

    if (EcdsaCurve::secp256k1 == curve) {
        privateKey.Assign(privateOut);
        publicKey.Assign(publicOut);
    } else {
        const auto expanded = sodium::ExpandSeed(
            {reinterpret_cast<const char*>(privateOut.data()),
             privateOut.size()},
            privateKey.WriteInto(Secret::Mode::Mem),
            publicKey.WriteInto());

        if (false == expanded) {
            throw std::runtime_error("Failed to expand seed");
        }
    }

    chainCode.Assign(chainOut);
}

auto HDNode::check() const noexcept(false) -> void
{
    if (false == data_.valid(33 + 4)) {
        throw std::runtime_error("Failed to allocate temporary data space");
    }

    if (false == hash_.valid(64)) {
        throw std::runtime_error("Failed to allocate temporary hash space");
    }
}

auto HDNode::child() noexcept -> Secret&
{
    return (1 == (switch_ % 2)) ? a_ : b_;
}

auto HDNode::ChildCode() noexcept -> WritableView
{
    auto* start = static_cast<std::byte*>(child().data());
    std::advance(start, 32);

    return WritableView{start, 32};
}

auto HDNode::ChildPrivate() noexcept -> AllocateOutput
{
    auto* start = static_cast<std::byte*>(child().data());

    return [start](const auto) { return WritableView{start, 32}; };
}

auto HDNode::ChildPublic() noexcept -> AllocateOutput
{
    auto* start = static_cast<std::byte*>(child().data());
    std::advance(start, 32 + 32);

    return [start](const auto) { return WritableView{start, 33}; };
}

auto HDNode::Fingerprint() const noexcept -> Bip32Fingerprint
{
    return key::HD::CalculateFingerprint(crypto_.Hash(), ParentPublic());
}

auto HDNode::InitCode() noexcept -> AllocateOutput
{
    auto* start = static_cast<std::byte*>(parent().data());
    std::advance(start, 32);

    return [start](const auto) { return WritableView{start, 32}; };
}

auto HDNode::InitPrivate() noexcept -> AllocateOutput
{
    auto* start = static_cast<std::byte*>(parent().data());

    return [start](const auto) { return WritableView{start, 32}; };
}

auto HDNode::InitPublic() noexcept -> AllocateOutput
{
    auto* start = static_cast<std::byte*>(parent().data());
    std::advance(start, 32 + 32);

    return [start](const auto) { return WritableView{start, 33}; };
}

auto HDNode::Next() noexcept -> void { ++switch_; }

auto HDNode::parent() const noexcept -> const Secret&
{
    return (0 == (switch_ % 2)) ? a_ : b_;
}

auto HDNode::parent() noexcept -> Secret&
{
    return (0 == (switch_ % 2)) ? a_ : b_;
}

auto HDNode::ParentCode() const noexcept -> ReadView
{
    const auto* start{parent().Bytes().data()};
    std::advance(start, 32);

    return ReadView{start, 32};
}

auto HDNode::ParentPrivate() const noexcept -> ReadView
{
    const auto* start{parent().Bytes().data()};

    return ReadView{start, 32};
}

auto HDNode::ParentPublic() const noexcept -> ReadView
{
    const auto* start{parent().Bytes().data()};
    std::advance(start, 32 + 32);

    return ReadView{start, 33};
}
}  // namespace opentxs::crypto::implementation
