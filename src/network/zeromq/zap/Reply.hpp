// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::network::zeromq::zap::Status

#pragma once

#include <cstddef>

#include "internal/network/zeromq/message/Factory.hpp"
#include "internal/network/zeromq/zap/Reply.hpp"
#include "internal/network/zeromq/zap/ZAP.hpp"
#include "internal/util/P0330.hpp"
#include "network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace zap
{
class Request;
}  // namespace zap
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::zap
{
class Reply::Imp final : public zeromq::Message::Imp
{
public:
    static constexpr auto default_version_{"1.0"};
    static constexpr auto version_position_ = 0_uz;
    static constexpr auto request_id_position_ = 1_uz;
    static constexpr auto status_code_position_ = 2_uz;
    static constexpr auto status_text_position_ = 3_uz;
    static constexpr auto user_id_position_ = 4_uz;
    static constexpr auto metadata_position_ = 5_uz;

    static auto string_to_code(const ReadView string) noexcept -> zap::Status;

    Imp() noexcept;
    Imp(const Request&,
        const zap::Status& code,
        const ReadView status,
        const ReadView userID,
        const ReadView metadata,
        const ReadView version) noexcept;
    Imp(const Imp& rhs) noexcept;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp() final = default;

private:
    using CodeMap = UnallocatedMap<zap::Status, UnallocatedCString>;
    using CodeReverseMap = UnallocatedMap<UnallocatedCString, zap::Status>;

    static const CodeMap code_map_;
    static const CodeReverseMap code_reverse_map_;

    static auto code_to_string(const zap::Status& code) noexcept
        -> UnallocatedCString;

    Imp(const SimpleCallback header,
        const ReadView requestID,
        const zap::Status& code,
        const ReadView status,
        const ReadView userID,
        const ReadView metadata,
        const ReadView version) noexcept;
};
}  // namespace opentxs::network::zeromq::zap
