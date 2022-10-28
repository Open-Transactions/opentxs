// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <string_view>

#include "internal/blockchain/ethereum/RLP.hpp"

namespace ot = opentxs;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace boost
{
namespace json
{
class string;
class value;
}  // namespace json
}  // namespace boost

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace ethereum
{
namespace rlp
{
class Node;
}  // namespace rlp
}  // namespace ethereum
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ottest
{
struct OPENTXS_EXPORT RLPVector {
    ot::CString name_{};
    ot::blockchain::ethereum::rlp::Node node_{};
    ot::ByteArray encoded_{};
};

OPENTXS_EXPORT auto GetRLPVectors(const ot::api::Session& api) noexcept
    -> const ot::Vector<RLPVector>&;
}  // namespace ottest

namespace ottest
{
OPENTXS_EXPORT auto get_rlp_raw() noexcept -> std::string_view;
OPENTXS_EXPORT auto json_is_bigint(const boost::json::string& in) noexcept
    -> bool;
OPENTXS_EXPORT auto json_is_escaped_unicode(
    const boost::json::string& in) noexcept -> bool;
OPENTXS_EXPORT auto parse(
    const opentxs::api::Session& api,
    const boost::json::string& in,
    opentxs::blockchain::ethereum::rlp::Node& out) noexcept -> void;
OPENTXS_EXPORT auto parse(
    const opentxs::api::Session& api,
    const boost::json::value& in,
    opentxs::blockchain::ethereum::rlp::Node& out) noexcept -> void;
OPENTXS_EXPORT auto parse(
    const opentxs::api::Session& api,
    std::int64_t in,
    opentxs::blockchain::ethereum::rlp::Node& out) noexcept -> void;
OPENTXS_EXPORT auto parse_as_bigint(
    const opentxs::api::Session& api,
    const boost::json::string& in,
    opentxs::blockchain::ethereum::rlp::Node& out) noexcept -> void;
OPENTXS_EXPORT auto parse_as_escaped_unicode(
    const opentxs::api::Session& api,
    const boost::json::string& in,
    opentxs::blockchain::ethereum::rlp::Node& out) noexcept -> void;
}  // namespace ottest
