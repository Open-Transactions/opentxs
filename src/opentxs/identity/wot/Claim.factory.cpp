// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::proto::ContactItemAttribute

#include "opentxs/identity/wot/internal.factory.hpp"  // IWYU pragma: associated

#include <Claim.pb.h>
#include <ContactItem.pb.h>
#include <ContactItemAttribute.pb.h>
#include <algorithm>
#include <functional>
#include <iterator>
#include <optional>
#include <stdexcept>

#include "internal/util/PMR.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Type.hpp"  // IWYU pragma: keep
#include "opentxs/identifier/Types.hpp"
#include "opentxs/identity/wot/Claim.internal.hpp"
#include "opentxs/identity/wot/ClaimPrivate.hpp"
#include "opentxs/identity/wot/claim/Types.internal.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
using namespace std::literals;

auto Claim(
    const api::Session& api,
    const identity::wot::Claimant& claimant,
    identity::wot::claim::SectionType section,
    identity::wot::claim::ClaimType type,
    ReadView value,
    ReadView subtype,
    std::span<const identity::wot::claim::Attribute> attributes,
    Time start,
    Time stop,
    VersionNumber version,
    alloc::Strategy alloc) noexcept -> identity::wot::internal::Claim*
{
    using ReturnType = identity::wot::ClaimPrivate;
    using BlankType = identity::wot::internal::Claim;

    try {
        return pmr::construct<ReturnType>(
            alloc.result_,
            api,
            claimant,
            version,
            section,
            type,
            value,
            subtype,
            start,
            stop,
            attributes,
            std::nullopt);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}

auto Claim(
    const api::Session& api,
    const proto::Claim& proto,
    alloc::Strategy alloc) noexcept -> identity::wot::internal::Claim*
{
    using ReturnType = identity::wot::ClaimPrivate;
    using BlankType = identity::wot::internal::Claim;

    try {
        const auto& item = proto.item();
        const auto claimant = [&]() -> identity::wot::Claimant {
            const auto id = api.Factory().Internal().Identifier(proto.nym());
            const auto type = id.Type();

            switch (type) {
                using enum identifier::Type;
                case generic: {

                    return id;
                }
                case nym: {

                    return api.Factory().Internal().NymID(proto.nym());
                }
                case invalid:
                case notary:
                case unitdefinition:
                case account:
                case hdseed:
                default: {
                    throw std::runtime_error{
                        "invalid identifier type: "s.append(print(type))};
                }
            }
        }();

        return pmr::construct<ReturnType>(
            alloc.result_,
            api,
            claimant,
            proto.version(),
            translate(proto.section()),
            translate(item.type()),
            item.value(),
            item.subtype(),
            seconds_since_epoch_unsigned(item.start()).value(),
            seconds_since_epoch_unsigned(item.end()).value(),
            Vector<identity::wot::claim::Attribute>{alloc.work_},
            proto);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}

auto Claim(
    const api::Session& api,
    const identity::wot::Claimant& claimant,
    const identity::wot::claim::SectionType section,
    const proto::ContactItem& proto,
    alloc::Strategy alloc) noexcept -> identity::wot::internal::Claim*
{
    using ReturnType = identity::wot::ClaimPrivate;
    using BlankType = identity::wot::internal::Claim;

    try {
        return pmr::construct<ReturnType>(
            alloc.result_,
            api,
            claimant,
            proto.version(),
            section,
            translate(proto.type()),
            proto.value(),
            proto.subtype(),
            seconds_since_epoch_unsigned(proto.start()).value(),
            seconds_since_epoch_unsigned(proto.end()).value(),
            [&] {
                const auto& in = proto.attribute();
                auto a = Vector<identity::wot::claim::Attribute>{alloc.result_};
                a.reserve(in.size());
                a.clear();
                static const auto translate = [](const auto& v) {
                    return proto::translate(
                        static_cast<proto::ContactItemAttribute>(v));
                };
                std::ranges::transform(
                    in, std::inserter(a, a.end()), translate);

                return a;
            }(),
            std::nullopt);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}
}  // namespace opentxs::factory
