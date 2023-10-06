// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/request/storesecret/Implementation.hpp"  // IWYU pragma: associated

#include <PeerRequest.pb.h>
#include <StoreSecret.pb.h>
#include <algorithm>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "internal/core/contract/peer/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/contract/peer/Types.hpp"

namespace opentxs::contract::peer::request::storesecret
{
Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    identifier::Nym initiator,
    identifier::Nym responder,
    const contract::peer::SecretType kind,
    std::span<const std::string_view> data,
    allocator_type alloc) noexcept(false)
    : RequestPrivate(alloc)
    , StoreSecretPrivate(alloc)
    , base::Implementation(
          api,
          std::move(signer),
          default_version_,
          std::move(initiator),
          std::move(responder),
          alloc)
    , kind_(std::move(kind))
    , values_([&] {
        auto out = decltype(values_){alloc};
        out.reserve(max_values_);
        out.clear();

        if (data.size() > max_values_) {

            throw std::runtime_error{"too many elements"};
        }

        std::ranges::copy(data, std::back_inserter(out));

        return out;
    }())
    , views_(make_views(values_))
    , self_(this)
{
    OT_ASSERT(values_.size() == views_.size());
}

Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    const serialized_type& proto,
    allocator_type alloc) noexcept(false)
    : RequestPrivate(alloc)
    , StoreSecretPrivate(alloc)
    , base::Implementation(api, std::move(signer), proto, alloc)
    , kind_(translate(proto.storesecret().type()))
    , values_([&] {
        auto out = decltype(values_){alloc};
        out.reserve(max_values_);
        out.clear();
        const auto& data = proto.storesecret();

        if (const auto& first = data.primary(); false == first.empty()) {
            out.emplace_back(first);

            if (const auto& second = data.secondary();
                false == second.empty()) {
                out.emplace_back(second);
            }
        }

        OT_ASSERT(out.size() <= max_values_);

        return out;
    }())
    , views_(make_views(values_))
    , self_(this)
{
    OT_ASSERT(values_.size() == views_.size());
}

Implementation::Implementation(
    const Implementation& rhs,
    allocator_type alloc) noexcept
    : RequestPrivate(alloc)
    , StoreSecretPrivate(alloc)
    , base::Implementation(rhs, alloc)
    , kind_(rhs.kind_)
    , values_(rhs.values_, alloc)
    , views_(make_views(values_))
    , self_(this)
{
    OT_ASSERT(values_.size() == views_.size());
}

auto Implementation::id_form() const noexcept -> serialized_type
{
    auto out = base::Implementation::id_form();
    auto& storesecret = *out.mutable_storesecret();
    storesecret.set_version(Version());
    storesecret.set_type(translate(kind_));

    if (0_uz < views_.size()) {
        storesecret.set_primary(views_[0].data(), views_[0].size());
    }

    if (1_uz < views_.size()) {
        storesecret.set_secondary(views_[1].data(), views_[1].size());
    }

    return out;
}

auto Implementation::make_views(const Vector<ByteArray>& in) noexcept
    -> Vector<std::string_view>
{
    auto out = Vector<std::string_view>{in.get_allocator()};
    out.reserve(in.size());
    out.clear();
    static constexpr auto as_read_view = [](const auto& i) {
        return i.Bytes();
    };
    std::ranges::transform(in, std::back_inserter(out), as_read_view);

    return out;
}

Implementation::~Implementation() { Reset(self_); }
}  // namespace opentxs::contract::peer::request::storesecret
