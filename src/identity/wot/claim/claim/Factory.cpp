// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::proto::ContactItemAttribute

#include "internal/identity/wot/claim/Factory.hpp"  // IWYU pragma: associated

#include <Claim.pb.h>
#include <ContactEnums.pb.h>
#include <ContactItem.pb.h>
#include <algorithm>
#include <iterator>
#include <optional>
#include <stdexcept>

#include "identity/wot/claim/claim/ClaimPrivate.hpp"
#include "identity/wot/claim/claim/Implementation.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto Claim(
    const api::Session& api,
    const identifier::Nym& claimant,
    identity::wot::claim::SectionType section,
    identity::wot::claim::ClaimType type,
    ReadView value,
    std::span<const identity::wot::claim::Attribute> attributes,
    Time start,
    Time stop,
    alloc::Strategy alloc) noexcept -> identity::wot::ClaimPrivate*
{
    using ReturnType = identity::wot::claim::implementation::Claim;
    using BlankType = identity::wot::ClaimPrivate;
    auto pmr = alloc::PMR<ReturnType>{alloc.result_};
    ReturnType* out = {nullptr};

    try {
        out = pmr.allocate(1_uz);

        if (nullptr == out) {

            throw std::runtime_error{"failed to allocate claim"};
        }

        pmr.construct(
            out,
            api,
            claimant,
            section,
            type,
            value,
            ReadView{},
            start,
            stop,
            [&] {
                auto a = Set<identity::wot::claim::Attribute>{alloc.result_};
                a.clear();
                std::copy(
                    attributes.begin(),
                    attributes.end(),
                    std::inserter(a, a.end()));

                return a;
            }(),
            std::nullopt);

        return out;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        auto fallback = alloc::PMR<BlankType>{alloc.result_};
        auto* blank = fallback.allocate(1_uz);

        OT_ASSERT(nullptr != blank);

        fallback.construct(blank);

        return blank;
    }
}

auto Claim(
    const api::Session& api,
    const proto::Claim& proto,
    alloc::Strategy alloc) noexcept -> identity::wot::ClaimPrivate*
{
    using ReturnType = identity::wot::claim::implementation::Claim;
    using BlankType = identity::wot::ClaimPrivate;
    auto pmr = alloc::PMR<ReturnType>{alloc.result_};
    ReturnType* out = {nullptr};

    try {
        out = pmr.allocate(1_uz);

        if (nullptr == out) {

            throw std::runtime_error{"failed to allocate claim"};
        }

        pmr.construct(
            out,
            api,
            api.Factory().NymIDFromBase58(proto.nymid()),
            translate(static_cast<proto::ContactSectionName>(proto.section())),
            translate(static_cast<proto::ContactItemType>(proto.type())),
            proto.value(),
            proto.subtype(),
            convert_stime(proto.start()),
            convert_stime(proto.end()),
            Set<identity::wot::claim::Attribute>{pmr},
            proto);

        return out;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        auto fallback = alloc::PMR<BlankType>{alloc.result_};
        auto* blank = fallback.allocate(1_uz);

        OT_ASSERT(nullptr != blank);

        fallback.construct(blank);

        return blank;
    }
}

auto Claim(
    const api::Session& api,
    const identifier::Nym& claimant,
    const identity::wot::claim::SectionType section,
    const proto::ContactItem& proto,
    alloc::Strategy alloc) noexcept -> identity::wot::ClaimPrivate*
{
    using ReturnType = identity::wot::claim::implementation::Claim;
    using BlankType = identity::wot::ClaimPrivate;
    auto pmr = alloc::PMR<ReturnType>{alloc.result_};
    ReturnType* out = {nullptr};

    try {
        out = pmr.allocate(1_uz);

        if (nullptr == out) {

            throw std::runtime_error{"failed to allocate claim"};
        }

        pmr.construct(
            out,
            api,
            proto.version(),
            claimant,
            section,
            translate(proto.type()),
            proto.value(),
            proto.subtype(),
            convert_stime(proto.start()),
            convert_stime(proto.end()),
            [&] {
                auto a = Set<identity::wot::claim::Attribute>{alloc.result_};
                a.clear();
                static const auto translate = [](const auto& in) {
                    return proto::translate(
                        static_cast<proto::ContactItemAttribute>(in));
                };
                std::transform(
                    proto.attribute().begin(),
                    proto.attribute().end(),
                    std::inserter(a, a.end()),
                    translate);

                return a;
            }(),
            std::nullopt);

        return out;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        auto fallback = alloc::PMR<BlankType>{alloc.result_};
        auto* blank = fallback.allocate(1_uz);

        OT_ASSERT(nullptr != blank);

        fallback.construct(blank);

        return blank;
    }
}
}  // namespace opentxs::factory
