// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <utility>

#include "internal/network/zeromq/zap/Request.hpp"
#include "internal/network/zeromq/zap/ZAP.hpp"
#include "internal/util/P0330.hpp"
#include "network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::network::zeromq::zap
{
class Request::Imp final : public zeromq::Message::Imp
{
public:
    using MechanismMap = UnallocatedMap<zap::Mechanism, UnallocatedCString>;
    using MechanismReverseMap =
        UnallocatedMap<UnallocatedCString, zap::Mechanism>;

    static constexpr auto default_version_{"1.0"};
    static constexpr auto version_position_ = 0_uz;
    static constexpr auto request_id_position_ = 1_uz;
    static constexpr auto domain_position_ = 2_uz;
    static constexpr auto address_position_ = 3_uz;
    static constexpr auto identity_position_ = 4_uz;
    static constexpr auto mechanism_position_ = 5_uz;
    static constexpr auto credentials_start_position_ = 6_uz;
    static constexpr auto max_string_field_size_ = 255_uz;
    static constexpr auto pubkey_size_ = 32_uz;

    static const UnallocatedSet<UnallocatedCString> accept_versions_;
    static const MechanismMap mechanism_map_;
    static const MechanismReverseMap mechanism_reverse_map_;

    static auto string_to_mechanism(const ReadView in) -> zap::Mechanism;

    Imp() noexcept;
    Imp(const ReadView address,
        const ReadView domain,
        const network::zeromq::zap::Mechanism mechanism,
        const ReadView requestID,
        const ReadView identity,
        const ReadView version) noexcept;
    Imp(const Imp& rhs) noexcept;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp() final = default;

private:
    static auto mechanism_to_string(const zap::Mechanism in)
        -> UnallocatedCString;
};
}  // namespace opentxs::network::zeromq::zap
