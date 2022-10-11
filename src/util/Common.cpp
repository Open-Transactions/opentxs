// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "opentxs/util/Types.hpp"  // IWYU pragma: associated

#include <boost/json.hpp>
#include <exception>
#include <iostream>
#include <string>

#include "internal/util/LogMacros.hpp"
#include "opentxs/util/BlockchainProfile.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
using namespace std::literals;

auto print(BlockchainProfile in) noexcept -> std::string_view
{
    try {
        using namespace std::literals;
        static const auto map = Map<BlockchainProfile, std::string_view>{
            {BlockchainProfile::mobile, "mobile"sv},
            {BlockchainProfile::desktop, "desktop"sv},
            {BlockchainProfile::desktop_native, "desktop_native"sv},
            {BlockchainProfile::server, "server"sv},
        };

        return map.at(in);
    } catch (...) {
        LogError()(__FUNCTION__)("invalid BlockchainProfile: ")(
            static_cast<std::uint8_t>(in))
            .Flush();

        OT_FAIL;
    }
}

// https://www.boost.org/doc/libs/1_80_0/libs/json/doc/html/json/examples.html#json.examples.pretty
auto print(
    const boost::json::value& jv,
    std::ostream& os,
    std::string* indent) noexcept -> bool
{
    try {
        auto indent_ = std::string{};

        if (nullptr == indent) { indent = std::addressof(indent_); }

        switch (jv.kind()) {
            case boost::json::kind::object: {
                os << "{\n";
                indent->append(4, ' ');
                auto const& obj = jv.get_object();

                if (!obj.empty()) {
                    const auto* it = obj.begin();

                    for (;;) {
                        os << *indent << boost::json::serialize(it->key())
                           << ": ";

                        if (false == print(it->value(), os, indent)) {

                            return false;
                        }

                        if (++it == obj.end()) { break; }

                        os << ",\n";
                    }
                }

                os << "\n";
                indent->resize(indent->size() - 4);
                os << *indent << "}";
            } break;
            case boost::json::kind::array: {
                os << "[\n";
                indent->append(4, ' ');
                auto const& arr = jv.get_array();

                if (!arr.empty()) {
                    const auto* it = arr.begin();

                    for (;;) {
                        os << *indent;

                        if (false == print(*it, os, indent)) { return false; }

                        if (++it == arr.end()) { break; }

                        os << ",\n";
                    }
                }

                os << "\n";
                indent->resize(indent->size() - 4);
                os << *indent << "]";
            } break;
            case boost::json::kind::string: {
                os << boost::json::serialize(jv.get_string());
            } break;
            case boost::json::kind::uint64: {
                os << jv.get_uint64();
            } break;
            case boost::json::kind::int64: {
                os << jv.get_int64();
            } break;
            case boost::json::kind::double_: {
                os << jv.get_double();
            } break;
            case boost::json::kind::bool_: {
                if (jv.get_bool()) {
                    os << "true";
                } else {
                    os << "false";
                }
            } break;
            case boost::json::kind::null: {
                os << "null";
            } break;
            default: {
                LogAbort()(__func__)(": unknown json value type").Abort();
            }
        }

        if (indent->empty()) { os << "\n"; }

        return true;
    } catch (const std::exception& e) {
        LogError()(__func__)(": ")(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs
