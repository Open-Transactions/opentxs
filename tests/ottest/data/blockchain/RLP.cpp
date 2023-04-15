// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/data/blockchain/RLP.hpp"  // IWYU pragma: associated

#include <boost/json.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/utility/string_view.hpp>
#include <opentxs/opentxs.hpp>
#include <cctype>
#include <iterator>
#include <sstream>
#include <utility>

#include "internal/util/LogMacros.hpp"

namespace ottest
{
auto GetRLPVectors(const ot::api::Session& api) noexcept
    -> const ot::Vector<RLPVector>&
{
    namespace json = boost::json;
    static const auto data = [&] {
        auto out = ot::Vector<RLPVector>{};
        const auto root = [&] {
            const auto raw = get_rlp_raw();
            const auto view = json::string_view{raw.data(), raw.size()};
            auto ec = json::error_code{};
            auto parsed = json::parse(view, ec);

            OT_ASSERT(false == ec.operator bool());

            return parsed;
        }();

        OT_ASSERT(root.is_object());

        for (const auto& [key, value] : root.as_object()) {
            OT_ASSERT(value.is_object());

            auto& vector = out.emplace_back();
            vector.name_ = {key.data(), key.size()};
            const auto& hex = value.as_object().at("out");

            OT_ASSERT(hex.is_string());

            const auto str = hex.as_string();
            auto rc = vector.encoded_.DecodeHex({str.data(), str.size()});

            OT_ASSERT(rc);

            const auto& in = value.as_object().at("in");
            parse(api, in, vector.node_);
        }

        return out;
    }();

    return data;
}

auto json_is_bigint(const boost::json::string& in) noexcept -> bool
{
    if (2u > in.size()) { return false; }

    if ('#' != in.at(0)) { return false; }

    for (const auto* i = std::next(in.cbegin()); i != in.cend(); ++i) {
        if (0 == std::isdigit(static_cast<unsigned char>(*i))) { return false; }
    }

    return true;
}

auto json_is_escaped_unicode(const boost::json::string& in) noexcept -> bool
{
    if (3u > in.size()) { return false; }

    if ('\\' != in.at(0)) { return false; }

    if ('u' != in.at(0)) { return false; }

    return true;
}

auto parse(
    const opentxs::api::Session& api,
    const boost::json::string& in,
    opentxs::blockchain::ethereum::rlp::Node& out) noexcept -> void
{
    if (0u == in.size()) {
        out.data_ = ot::blockchain::ethereum::rlp::Null{};
    } else {
        const auto view = boost::json::string_view{in};
        out.data_ = api.Factory().DataFromBytes({view.data(), view.size()});
    }
}

auto parse(
    const opentxs::api::Session& api,
    const boost::json::value& in,
    opentxs::blockchain::ethereum::rlp::Node& out) noexcept -> void
{
    switch (in.kind()) {
        using Type = boost::json::kind;

        case Type::string: {
            const auto& string = in.as_string();

            if (json_is_bigint(string)) {
                parse_as_bigint(api, string, out);
            } else if (json_is_escaped_unicode(string)) {
                parse_as_escaped_unicode(api, string, out);
            } else {
                parse(api, string, out);
            }
        } break;
        case Type::int64: {
            parse(api, in.as_int64(), out);
        } break;
        case Type::array: {
            const auto& array = in.as_array();
            auto items = ot::blockchain::ethereum::rlp::Sequence{};

            for (const auto& item : array) {
                auto& subnode = items.emplace_back();
                parse(api, item, subnode);
            }

            out.data_ = std::move(items);
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto parse(
    const opentxs::api::Session& api,
    std::int64_t in,
    opentxs::blockchain::ethereum::rlp::Node& out) noexcept -> void
{
    if (0 == in) {
        out.data_ = ot::blockchain::ethereum::rlp::Null{};
    } else {
        namespace mp = boost::multiprecision;
        const auto number = mp::checked_cpp_int{in};
        const auto hex = [&] {
            auto ss = std::stringstream{};
            ss << std::hex << std::showbase << number;

            return ss.str();
        }();
        out.data_ = api.Factory().DataFromHex(hex);
    }
}

auto parse_as_bigint(
    const opentxs::api::Session& api,
    const boost::json::string& in,
    opentxs::blockchain::ethereum::rlp::Node& out) noexcept -> void
{
    const auto string = std::string{in.c_str()}.substr(1);
    namespace mp = boost::multiprecision;
    const auto number = mp::checked_cpp_int{string};
    const auto hex = [&] {
        auto ss = std::stringstream{};
        ss << std::hex << std::showbase << number;

        return ss.str();
    }();
    out.data_ = api.Factory().DataFromHex(hex);
}

auto parse_as_escaped_unicode(
    const opentxs::api::Session& api,
    const boost::json::string& in,
    opentxs::blockchain::ethereum::rlp::Node& out) noexcept -> void
{
    // TODO find a better way than hard coding known values from the test
    // vectors
    using namespace std::literals;
    static constexpr auto bytestring00 = "\u0000"sv;
    static constexpr auto bytestring01 = "\u0001"sv;
    static constexpr auto bytestring7F = "\u007F"sv;

    if (bytestring00 == in.c_str()) {
        out.data_ = api.Factory().DataFromBytes(bytestring00);
    } else if (bytestring01 == in.c_str()) {
        out.data_ = api.Factory().DataFromBytes(bytestring01);
    } else if (bytestring7F == in.c_str()) {
        out.data_ = api.Factory().DataFromBytes(bytestring7F);
    } else {

        OT_FAIL;
    }
}
}  // namespace ottest
