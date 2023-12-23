// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/HDNode.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <functional>
#include <iterator>
#include <span>
#include <stdexcept>

#include "internal/crypto/asymmetric/key/HD.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/crypto/EcdsaCurve.hpp"  // IWYU pragma: keep
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Sodium.hpp"

namespace opentxs::crypto::implementation
{
HDNode::HDNode(const api::Factory& factory, const api::Crypto& crypto) noexcept
    : data_space_(factory.Secret(0))
    , hash_space_(factory.Secret(0))
    , data_(data_space_.WriteInto().Reserve(33 + 4))
    , hash_(hash_space_.WriteInto().Reserve(64))
    , crypto_(crypto)
    , switch_(0)
    , a_(factory.Secret(0))
    , b_(factory.Secret(0))
{
    check();

    {
        static const auto size = 32_uz + 32_uz + 33_uz;
        a_.resize(size);
        b_.resize(size);

        assert_true(size == a_.size());
        assert_true(size == b_.size());
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
    if (false == data_.IsValid(33 + 4)) {
        throw std::runtime_error("Failed to allocate temporary data space");
    }

    if (false == hash_.IsValid(64)) {
        throw std::runtime_error("Failed to allocate temporary hash space");
    }
}

auto HDNode::child() noexcept -> Secret&
{
    return (1 == (switch_ % 2)) ? a_ : b_;
}

auto HDNode::ChildCode() noexcept -> WriteBuffer
{
    auto* start = static_cast<std::byte*>(child().data());
    std::advance(start, 32);

    return std::span<std::byte>{start, 32};
}

auto HDNode::ChildPrivate() noexcept -> Writer
{
    auto* start = static_cast<std::byte*>(child().data());

    return {[start](const auto) -> WriteBuffer {
        return std::span<std::byte>{start, 32};
    }};
}

auto HDNode::ChildPublic() noexcept -> Writer
{
    auto* start = static_cast<std::byte*>(child().data());
    std::advance(start, 32 + 32);

    return {[start](const auto) -> WriteBuffer {
        return std::span<std::byte>{start, 33};
    }};
}

auto HDNode::Fingerprint() const noexcept -> Bip32Fingerprint
{
    return asymmetric::internal::key::HD::CalculateFingerprint(
        crypto_.Hash(), ParentPublic());
}

auto HDNode::InitCode() noexcept -> Writer
{
    auto* start = static_cast<std::byte*>(parent().data());
    std::advance(start, 32);

    return {[start](const auto) -> WriteBuffer {
        return std::span<std::byte>{start, 32};
    }};
}

auto HDNode::InitPrivate() noexcept -> Writer
{
    auto* start = static_cast<std::byte*>(parent().data());

    return {[start](const auto) -> WriteBuffer {
        return std::span<std::byte>{start, 32};
    }};
}

auto HDNode::InitPublic() noexcept -> Writer
{
    auto* start = static_cast<std::byte*>(parent().data());
    std::advance(start, 32 + 32);

    return {[start](const auto) -> WriteBuffer {
        return std::span<std::byte>{start, 33};
    }};
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
