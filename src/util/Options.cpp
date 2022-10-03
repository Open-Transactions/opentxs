// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"      // IWYU pragma: associated
#include "util/Options.hpp"  // IWYU pragma: associated

#include <boost/lexical_cast/bad_lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <algorithm>
#include <cctype>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/util/BlockchainProfile.hpp"
#include "opentxs/util/ConnectionMode.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

class QObject;

namespace po = boost::program_options;

namespace opentxs
{
struct Options::Imp::Parser {
    using Multistring = UnallocatedVector<UnallocatedCString>;

    static constexpr auto blockchain_disable_{"disable_blockchain"};
    static constexpr auto blockchain_ipv4_bind_{"blockchain_bind_ipv4"};
    static constexpr auto blockchain_ipv6_bind_{"blockchain_bind_ipv6"};
    static constexpr auto blockchain_profile_{"blockchain_profile"};
    static constexpr auto blockchain_sync_provide_{"provide_sync_server"};
    static constexpr auto blockchain_sync_connect_{"blockchain_sync_server"};
    static constexpr auto blockchain_wallet_enable_{"blockchain_wallet"};
    static constexpr auto default_mint_key_bytes_{"mint_key_default_bytes"};
    static constexpr auto experimental_{"ot_experimental"};
    static constexpr auto home_{"ot_home"};
    static constexpr auto ipv4_connection_mode_{"ipv4_connection_mode"};
    static constexpr auto ipv6_connection_mode_{"ipv6_connection_mode"};
    static constexpr auto log_endpoint_{"log_endpoint"};
    static constexpr auto log_level_{"log_level"};
    static constexpr auto max_jobs_{"thread_pool_cap"};
    static constexpr auto notary_inproc_{"notary_inproc"};
    static constexpr auto notary_bind_ip_{"notary_bind_ip"};
    static constexpr auto notary_bind_port_{"notary_bind_port"};
    static constexpr auto notary_name_{"notary_name"};
    static constexpr auto notary_public_eep_{"notary_public_eep"};
    static constexpr auto notary_public_ipv4_{"notary_public_ipv4"};
    static constexpr auto notary_public_ipv6_{"notary_public_ipv6"};
    static constexpr auto notary_public_onion_{"notary_public_onion"};
    static constexpr auto notary_public_port_{"notary_command_port"};
    static constexpr auto notary_terms_{"notary_terms"};
    static constexpr auto storage_plugin_{"ot_storage_plugin"};

    po::variables_map variables_;

    auto Args() const noexcept -> const po::options_description&
    {
        static const auto out = [] {
            auto out = po::options_description{"libopentxs options"};

            out.add_options()(
                blockchain_disable_,
                po::value<Multistring>()->multitoken()->composing(),
                "Previously enabled blockchains to remove from the automatic "
                "startup list");
            out.add_options()(
                blockchain_ipv4_bind_,
                po::value<Multistring>()->multitoken()->composing(),
                "Local ipv4 addresses to bind for incoming blockchain "
                "connections");
            out.add_options()(
                blockchain_ipv6_bind_,
                po::value<Multistring>()->multitoken()->composing(),
                "Local ipv6 addresses to bind for incoming blockchain "
                "connections");
            out.add_options()(
                blockchain_profile_,
                po::value<int>(),
                "Blockchain operational mode.\n    0: mobile mode\n    1: "
                "desktop mode\n    2: desktop native mode (does not use DHT "
                "for cfilters, not available on all chains)\n    3: server "
                "mode (downloads complete blockchain)");
            out.add_options()(
                blockchain_sync_provide_,
                po::value<bool>()->implicit_value(true),
                "Enable blockchain sync server support");
            out.add_options()(
                blockchain_sync_connect_,
                po::value<Multistring>()->multitoken()->composing(),
                "Blockchain sync server(s) to connect to as a client");
            out.add_options()(
                blockchain_wallet_enable_,
                po::value<bool>()->implicit_value(true),
                "Blockchain wallet support");
            out.add_options()(
                default_mint_key_bytes_,
                po::value<std::size_t>(),
                "Default key size for blinded mints");
            out.add_options()(
                home_,
                po::value<UnallocatedCString>(),
                "Path to opentxs data directory");
            out.add_options()(
                ipv4_connection_mode_,
                po::value<int>(),
                "Connection policy for ipv4 peers. -1 = ipv4 disabled, 0 = "
                "automatic, 1 = ipv4 enabled");
            out.add_options()(
                ipv6_connection_mode_,
                po::value<int>(),
                "Connection policy for ipv6 peers. -1 = ipv6 disabled, 0 = "
                "automatic, 1 = ipv6 enabled");
            out.add_options()(
                log_endpoint_,
                po::value<UnallocatedCString>(),
                "ZeroMQ endpoint to which to copy log data");
            out.add_options()(
                max_jobs_,
                po::value<int>(),
                "Maximum number of threads allowed in any thread pool");
            out.add_options()(
                log_level_,
                po::value<int>(),
                "Log verbosity. Valid values are -1 through 5. Higher numbers "
                "are more verbose. Default value is 0");
            out.add_options()(
                notary_bind_ip_,
                po::value<UnallocatedCString>(),
                "Local IP address for the notary to listen on");
            out.add_options()(
                notary_bind_port_,
                po::value<std::uint16_t>(),
                "Local TCP port for the notary to listen on");
            out.add_options()(
                notary_name_,
                po::value<UnallocatedCString>(),
                "(only when creating a new notary contract) notary name");
            out.add_options()(
                notary_terms_,
                po::value<UnallocatedCString>(),
                "(only when creating a new notary contract) notary terms and "
                "conditions");
            out.add_options()(
                notary_public_eep_,
                po::value<Multistring>()->multitoken()->composing(),
                "(only when creating a new notary contract) public eep address "
                "to advertise in contract");
            out.add_options()(
                notary_public_ipv4_,
                po::value<Multistring>()->multitoken()->composing(),
                "(only when creating a new notary contract) public ipv4 "
                "address "
                "to advertise in contract");
            out.add_options()(
                notary_public_ipv6_,
                po::value<Multistring>()->multitoken()->composing(),
                "(only when creating a new notary contract) public ipv6 "
                "address "
                "to advertise in contract");
            out.add_options()(
                notary_public_onion_,
                po::value<Multistring>()->multitoken()->composing(),
                "(only when creating a new notary contract) public onion "
                "address to advertise in contract");
            out.add_options()(
                notary_public_port_,
                po::value<UnallocatedCString>(),
                "(only when creating a new notary contract) public listening "
                "port");
            out.add_options()(
                storage_plugin_,
                po::value<UnallocatedCString>(),
                "primary opentxs storage plugin");
            out.add_options()(
                experimental_,
                po::value<bool>()->implicit_value(false),
                "Enable experimental opentxs features");

            return out;
        }();

        return out;
    }
    auto Help() const noexcept -> UnallocatedCString
    {
        // TODO c++20 return allocated
        auto out = std::stringstream{};
        out << Args();

        return out.str();
    }

    Parser() noexcept
        : variables_()
    {
    }
};

Options::Imp::Imp() noexcept
    : blockchain_disabled_chains_()
    , blockchain_ipv4_bind_()
    , blockchain_ipv6_bind_()
    , blockchain_profile_(std::nullopt)
    , blockchain_sync_server_enabled_(std::nullopt)
    , blockchain_sync_servers_()
    , blockchain_wallet_enabled_(std::nullopt)
    , default_mint_key_bytes_(std::nullopt)
    , experimental_(std::nullopt)
    , home_(std::nullopt)
    , ipv4_connection_mode_(std::nullopt)
    , ipv6_connection_mode_(std::nullopt)
    , log_endpoint_(std::nullopt)
    , log_level_(std::nullopt)
    , max_jobs_(std::nullopt)
    , notary_bind_inproc_(std::nullopt)
    , notary_bind_ip_(std::nullopt)
    , notary_bind_port_(std::nullopt)
    , notary_name_(std::nullopt)
    , notary_public_eep_()
    , notary_public_ipv4_()
    , notary_public_ipv6_()
    , notary_public_onion_()
    , notary_public_port_(std::nullopt)
    , notary_terms_(std::nullopt)
    , qt_root_object_(std::nullopt)
    , storage_primary_plugin_(std::nullopt)
    , test_mode_(std::nullopt)
{
}

Options::Imp::Imp(const Imp& rhs) noexcept

    = default;

auto Options::Imp::convert(std::string_view value) const noexcept(false)
    -> blockchain::Type
{
    static const auto& chains = blockchain::DefinedChains();
    static const auto names = [] {
        auto out = Map<CString, blockchain::Type>{};

        for (const auto& chain : chains) {
            // TODO add allocated or string_view version of TickerSymbol
            out.emplace(lower(TickerSymbol(chain).c_str()), chain);
        }

        return out;
    }();

    try {

        return names.at(lower(value));
    } catch (...) {
    }

    try {
        auto temp = UnallocatedCString{value};  // TODO
        const auto candidate = static_cast<blockchain::Type>(std::stoi(temp));

        if (0 < chains.count(candidate)) { return candidate; }
    } catch (...) {
    }

    throw std::out_of_range{"not a blockchain"};
}

auto Options::Imp::get(const std::optional<CString>& data) noexcept
    -> std::string_view
{
    static const auto null = CString{};

    if (const auto& v = data; v.has_value()) {

        return v.value().c_str();
    } else {

        return null.c_str();
    }
}

auto Options::Imp::help() const noexcept -> std::string_view
{
    static const auto text = [&] {
        auto parser = Parser{};

        return parser.Help();
    }();

    return text;
}

auto Options::Imp::import_value(
    std::string_view key,
    std::string_view value) noexcept -> void
{
    const auto sValue = UnallocatedCString{value};

    try {
        if (0 == key.compare(Parser::blockchain_disable_)) {
            blockchain_disabled_chains_.emplace(convert(value));
        } else if (0 == key.compare(Parser::blockchain_ipv4_bind_)) {
            blockchain_ipv4_bind_.emplace(value);
        } else if (0 == key.compare(Parser::blockchain_ipv6_bind_)) {
            blockchain_ipv6_bind_.emplace(value);
        } else if (0 == key.compare(Parser::blockchain_profile_)) {
            using Type = opentxs::BlockchainProfile;

            switch (std::stoi(sValue)) {
                case static_cast<int>(Type::mobile): {
                    blockchain_profile_ = Type::mobile;
                } break;
                case static_cast<int>(Type::desktop): {
                    blockchain_profile_ = Type::desktop;
                } break;
                case static_cast<int>(Type::desktop_native): {
                    blockchain_profile_ = Type::desktop_native;
                } break;
                case static_cast<int>(Type::server): {
                    blockchain_profile_ = Type::server;
                } break;
                default: {
                }
            }
        } else if (0 == key.compare(Parser::blockchain_sync_provide_)) {
            blockchain_sync_server_enabled_ = to_bool(value);

            if (blockchain_sync_server_enabled_) {
                blockchain_wallet_enabled_ = false;
            }
        } else if (0 == key.compare(Parser::blockchain_sync_connect_)) {
            blockchain_sync_servers_.emplace(value);
        } else if (0 == key.compare(Parser::blockchain_wallet_enable_)) {
            blockchain_wallet_enabled_ = to_bool(value);
        } else if (0 == key.compare(Parser::default_mint_key_bytes_)) {
            default_mint_key_bytes_ = std::stoull(sValue);
        } else if (0 == key.compare(Parser::experimental_)) {
            experimental_ = to_bool(value);
        } else if (0 == key.compare(Parser::home_)) {
            home_ = value;
        } else if (0 == key.compare(Parser::ipv4_connection_mode_)) {
            ipv4_connection_mode_ =
                static_cast<ConnectionMode>(std::stoi(sValue));
        } else if (0 == key.compare(Parser::ipv6_connection_mode_)) {
            ipv6_connection_mode_ =
                static_cast<ConnectionMode>(std::stoi(sValue));
        } else if (0 == key.compare(Parser::log_endpoint_)) {
            log_endpoint_ = value;
        } else if (0 == key.compare(Parser::log_level_)) {
            log_level_ = std::stoi(sValue);
        } else if (0 == key.compare(Parser::max_jobs_)) {
            max_jobs_ = std::max(std::stoi(sValue), 0);
        } else if (0 == key.compare(Parser::notary_inproc_)) {
            notary_bind_inproc_ = to_bool(value);
        } else if (0 == key.compare(Parser::notary_bind_ip_)) {
            notary_bind_ip_ = value;
        } else if (0 == key.compare(Parser::notary_bind_port_)) {
            notary_bind_port_ = std::stoi(sValue);
        } else if (0 == key.compare(Parser::notary_name_)) {
            notary_name_ = value;
        } else if (0 == key.compare(Parser::notary_public_eep_)) {
            notary_public_eep_.emplace(value);
        } else if (0 == key.compare(Parser::notary_public_ipv4_)) {
            notary_public_ipv4_.emplace(value);
        } else if (0 == key.compare(Parser::notary_public_ipv6_)) {
            notary_public_ipv6_.emplace(value);
        } else if (0 == key.compare(Parser::notary_public_onion_)) {
            notary_public_onion_.emplace(value);
        } else if (0 == key.compare(Parser::notary_public_port_)) {
            notary_public_port_ = std::stoi(sValue);
        } else if (0 == key.compare(Parser::notary_terms_)) {
            notary_terms_ = value;
        } else if (0 == key.compare(Parser::storage_plugin_)) {
            storage_primary_plugin_ = value;
        }
    } catch (...) {
    }
}

auto Options::Imp::lower(std::string_view in) noexcept -> CString
{
    auto out = CString{};
    std::transform(in.begin(), in.end(), std::back_inserter(out), [](auto c) {
        return std::tolower(c);
    });

    return out;
}

auto Options::Imp::parse(int argc, char** argv) noexcept(false) -> void
{
    auto parser = Parser{};

    try {
        const auto parsed = po::command_line_parser(argc, argv)
                                .options(parser.Args())
                                .allow_unregistered()
                                .run();
        po::store(parsed, parser.variables_);
        po::notify(parser.variables_);
    } catch (po::error& e) {

        throw std::runtime_error{e.what()};
    }

    for (const auto& [name, value] : parser.variables_) {
        if (name == Parser::blockchain_disable_) {
            try {
                const auto& chains = value.as<Parser::Multistring>();

                for (const auto& chain : chains) {
                    try {
                        blockchain_disabled_chains_.emplace(convert(chain));
                    } catch (...) {
                    }
                }
            } catch (...) {
            }
        } else if (name == Parser::blockchain_ipv4_bind_) {
            try {
                const auto& servers = value.as<Parser::Multistring>();
                auto& dest = blockchain_ipv4_bind_;

                for (const auto& server : servers) {
                    dest.emplace(server.c_str());
                }
            } catch (...) {
            }
        } else if (name == Parser::blockchain_ipv6_bind_) {
            try {
                const auto& servers = value.as<Parser::Multistring>();
                auto& dest = blockchain_ipv6_bind_;

                for (const auto& server : servers) {
                    dest.emplace(server.c_str());
                }
            } catch (...) {
            }
        } else if (name == Parser::blockchain_profile_) {
            try {
                using Type = opentxs::BlockchainProfile;

                switch (value.as<int>()) {
                    case 0: {
                        blockchain_profile_ = Type::mobile;
                    } break;
                    case 1: {
                        blockchain_profile_ = Type::desktop;
                    } break;
                    case 2: {
                        blockchain_profile_ = Type::desktop_native;
                    } break;
                    case 3: {
                        blockchain_profile_ = Type::server;
                    } break;
                    default: {
                    }
                }
            } catch (...) {
            }
        } else if (name == Parser::blockchain_sync_provide_) {
            try {
                blockchain_sync_server_enabled_ = value.as<bool>();

                if (blockchain_sync_server_enabled_) {
                    blockchain_wallet_enabled_ = false;
                }
            } catch (...) {
            }
        } else if (name == Parser::blockchain_sync_connect_) {
            try {
                const auto& servers = value.as<Parser::Multistring>();
                auto& dest = blockchain_sync_servers_;

                for (const auto& server : servers) {
                    dest.emplace(server.c_str());
                }
            } catch (...) {
            }
        } else if (name == Parser::blockchain_wallet_enable_) {
            try {
                blockchain_wallet_enabled_ = value.as<bool>();
            } catch (...) {
            }
        } else if (name == Parser::default_mint_key_bytes_) {
            try {
                default_mint_key_bytes_ = value.as<std::size_t>();
            } catch (...) {
            }
        } else if (name == Parser::home_) {
            try {
                home_ = value.as<UnallocatedCString>().c_str();
            } catch (...) {
            }
        } else if (name == Parser::ipv4_connection_mode_) {
            try {
                ipv4_connection_mode_ =
                    static_cast<ConnectionMode>(value.as<int>());
            } catch (...) {
            }
        } else if (name == Parser::ipv6_connection_mode_) {
            try {
                ipv6_connection_mode_ =
                    static_cast<ConnectionMode>(value.as<int>());
            } catch (...) {
            }
        } else if (name == Parser::log_endpoint_) {
            try {
                log_endpoint_ = value.as<UnallocatedCString>().c_str();
            } catch (...) {
            }
        } else if (name == Parser::log_level_) {
            try {
                log_level_ = value.as<int>();
            } catch (...) {
            }
        } else if (name == Parser::max_jobs_) {
            try {
                max_jobs_ =
                    static_cast<unsigned int>(std::max(value.as<int>(), 0));
            } catch (...) {
            }
        } else if (name == Parser::notary_bind_ip_) {
            try {
                notary_bind_ip_ = value.as<UnallocatedCString>().c_str();
            } catch (...) {
            }
        } else if (name == Parser::notary_bind_port_) {
            try {
                notary_bind_port_ = value.as<std::uint16_t>();
            } catch (...) {
            }
        } else if (name == Parser::notary_name_) {
            try {
                notary_name_ = value.as<UnallocatedCString>().c_str();
            } catch (...) {
            }
        } else if (name == Parser::notary_terms_) {
            try {
                notary_terms_ = value.as<UnallocatedCString>().c_str();
            } catch (...) {
            }
        } else if (name == Parser::notary_public_eep_) {
            try {
                const auto& servers = value.as<Parser::Multistring>();
                auto& dest = notary_public_eep_;

                for (const auto& server : servers) {
                    dest.emplace(server.c_str());
                }
            } catch (...) {
            }
        } else if (name == Parser::notary_public_ipv4_) {
            try {
                const auto& servers = value.as<Parser::Multistring>();
                auto& dest = notary_public_ipv4_;

                for (const auto& server : servers) {
                    dest.emplace(server.c_str());
                }
            } catch (...) {
            }
        } else if (name == Parser::notary_public_ipv6_) {
            try {
                const auto& servers = value.as<Parser::Multistring>();
                auto& dest = notary_public_ipv6_;

                for (const auto& server : servers) {
                    dest.emplace(server.c_str());
                }
            } catch (...) {
            }
        } else if (name == Parser::notary_public_onion_) {
            try {
                const auto& servers = value.as<Parser::Multistring>();
                auto& dest = notary_public_onion_;

                for (const auto& server : servers) {
                    dest.emplace(server.c_str());
                }
            } catch (...) {
            }
        } else if (name == Parser::notary_public_port_) {
            try {
                notary_public_port_ = value.as<std::uint16_t>();
            } catch (...) {
            }
        } else if (name == Parser::storage_plugin_) {
            try {
                storage_primary_plugin_ =
                    value.as<UnallocatedCString>().c_str();
            } catch (...) {
            }
        }
    }
}

auto Options::Imp::to_bool(std::string_view value) noexcept -> bool
{
    try {
        const auto sValue = UnallocatedCString{value};

        return 0 != std::stoi(sValue);
    } catch (...) {
    }

    const auto normal = lower(value);

    if (normal == "true") {

        return true;
    } else if (normal == "on") {

        return true;
    } else if (normal == "yes") {

        return true;
    }

    return false;
}

Options::Imp::~Imp() = default;
}  // namespace opentxs

namespace opentxs
{
auto operator+(const Options& lhs, const Options& rhs) noexcept -> Options
{
    auto out{lhs};
    auto& l = *out.imp_;
    const auto& r = *rhs.imp_;

    std::copy(
        r.blockchain_disabled_chains_.begin(),
        r.blockchain_disabled_chains_.end(),
        std::inserter(
            l.blockchain_disabled_chains_,
            l.blockchain_disabled_chains_.end()));
    std::copy(
        r.blockchain_ipv4_bind_.begin(),
        r.blockchain_ipv4_bind_.end(),
        std::inserter(l.blockchain_ipv4_bind_, l.blockchain_ipv4_bind_.end()));
    std::copy(
        r.blockchain_ipv6_bind_.begin(),
        r.blockchain_ipv6_bind_.end(),
        std::inserter(l.blockchain_ipv6_bind_, l.blockchain_ipv6_bind_.end()));

    if (const auto& v = r.blockchain_profile_; v.has_value()) {
        l.blockchain_profile_ = v.value();
    }

    if (const auto& v = r.blockchain_sync_server_enabled_; v.has_value()) {
        l.blockchain_sync_server_enabled_ = v.value();
    }

    std::copy(
        r.blockchain_sync_servers_.begin(),
        r.blockchain_sync_servers_.end(),
        std::inserter(
            l.blockchain_sync_servers_, l.blockchain_sync_servers_.end()));

    if (const auto& v = r.blockchain_wallet_enabled_; v.has_value()) {
        l.blockchain_wallet_enabled_ = v.value();
    }

    if (const auto& v = r.default_mint_key_bytes_; v.has_value()) {
        l.default_mint_key_bytes_ = v.value();
    }

    if (const auto& v = r.experimental_; v.has_value()) {
        l.experimental_ = v.value();
    }

    if (const auto& v = r.home_; v.has_value()) { l.home_ = v.value(); }

    if (const auto& v = r.ipv4_connection_mode_; v.has_value()) {
        l.ipv4_connection_mode_ = v.value();
    }

    if (const auto& v = r.ipv6_connection_mode_; v.has_value()) {
        l.ipv6_connection_mode_ = v.value();
    }

    if (const auto& v = r.log_endpoint_; v.has_value()) {
        l.log_endpoint_ = v.value();
    }

    if (const auto& v = r.log_level_; v.has_value()) {
        l.log_level_ = v.value();
    }

    if (const auto& v = r.max_jobs_; v.has_value()) { l.max_jobs_ = v.value(); }

    if (const auto& v = r.notary_bind_inproc_; v.has_value()) {
        l.notary_bind_inproc_ = v.value();
    }

    if (const auto& v = r.notary_bind_ip_; v.has_value()) {
        l.notary_bind_ip_ = v.value();
    }

    if (const auto& v = r.notary_bind_port_; v.has_value()) {
        l.notary_bind_port_ = v.value();
    }

    if (const auto& v = r.notary_name_; v.has_value()) {
        l.notary_name_ = v.value();
    }

    std::copy(
        r.notary_public_eep_.begin(),
        r.notary_public_eep_.end(),
        std::inserter(l.notary_public_eep_, l.notary_public_eep_.end()));
    std::copy(
        r.notary_public_ipv4_.begin(),
        r.notary_public_ipv4_.end(),
        std::inserter(l.notary_public_ipv4_, l.notary_public_ipv4_.end()));
    std::copy(
        r.notary_public_ipv6_.begin(),
        r.notary_public_ipv6_.end(),
        std::inserter(l.notary_public_ipv6_, l.notary_public_ipv6_.end()));
    std::copy(
        r.notary_public_onion_.begin(),
        r.notary_public_onion_.end(),
        std::inserter(l.notary_public_onion_, l.notary_public_onion_.end()));

    if (const auto& v = r.notary_public_port_; v.has_value()) {
        l.notary_public_port_ = v.value();
    }

    if (const auto& v = r.notary_terms_; v.has_value()) {
        l.notary_terms_ = v.value();
    }

    if (const auto& v = r.qt_root_object_; v.has_value()) {
        l.qt_root_object_ = v.value();
    }

    if (const auto& v = r.storage_primary_plugin_; v.has_value()) {
        l.storage_primary_plugin_ = v.value();
    }

    if (const auto& v = r.test_mode_; v.has_value()) {
        l.test_mode_ = v.value();
    }

    return out;
}

Options::Options() noexcept
    : imp_(std::make_unique<Imp>().release())
{
    OT_ASSERT(nullptr != imp_);
}

Options::Options(int argc, char** argv) noexcept
    : Options()
{
    ParseCommandLine(argc, argv);
}

Options::Options(const Options& rhs) noexcept
    : imp_(std::make_unique<Imp>(*rhs.imp_).release())
{
    OT_ASSERT(nullptr != imp_);
}

Options::Options(Options&& rhs) noexcept
    : imp_(rhs.imp_)
{
    rhs.imp_ = nullptr;

    OT_ASSERT(nullptr != imp_);
}

auto Options::AddBlockchainIpv4Bind(std::string_view endpoint) noexcept
    -> Options&
{
    imp_->blockchain_ipv4_bind_.emplace(endpoint);

    return *this;
}

auto Options::AddBlockchainIpv6Bind(std::string_view endpoint) noexcept
    -> Options&
{
    imp_->blockchain_ipv6_bind_.emplace(endpoint);

    return *this;
}

auto Options::AddBlockchainSyncServer(std::string_view endpoint) noexcept
    -> Options&
{
    imp_->blockchain_sync_servers_.emplace(endpoint);

    return *this;
}

auto Options::AddNotaryPublicEEP(std::string_view value) noexcept -> Options&
{
    imp_->notary_public_eep_.emplace(value);

    return *this;
}

auto Options::AddNotaryPublicIPv4(std::string_view value) noexcept -> Options&
{
    imp_->notary_public_ipv4_.emplace(value);

    return *this;
}

auto Options::AddNotaryPublicIPv6(std::string_view value) noexcept -> Options&
{
    imp_->notary_public_ipv6_.emplace(value);

    return *this;
}

auto Options::AddNotaryPublicOnion(std::string_view value) noexcept -> Options&
{
    imp_->notary_public_onion_.emplace(value);

    return *this;
}

auto Options::BlockchainBindIpv4() const noexcept -> const Set<CString>&
{
    return imp_->blockchain_ipv4_bind_;
}

auto Options::BlockchainBindIpv6() const noexcept -> const Set<CString>&
{
    return imp_->blockchain_ipv6_bind_;
}

auto Options::BlockchainProfile() const noexcept -> opentxs::BlockchainProfile
{
    return Imp::get(
        imp_->blockchain_profile_, opentxs::BlockchainProfile::desktop);
}

auto Options::BlockchainWalletEnabled() const noexcept -> bool
{
    return Imp::get(imp_->blockchain_wallet_enabled_, true);
}

auto Options::DefaultMintKeyBytes() const noexcept -> std::size_t
{
    return Imp::get(
        imp_->default_mint_key_bytes_,
        api::session::Notary::DefaultMintKeyBytes());
}

auto Options::DisableBlockchain(blockchain::Type chain) noexcept -> Options&
{
    imp_->blockchain_disabled_chains_.emplace(chain);

    return *this;
}

auto Options::DisabledBlockchains() const noexcept
    -> const Set<blockchain::Type>&
{
    return imp_->blockchain_disabled_chains_;
}

auto Options::Experimental() const noexcept -> bool
{
    return Imp::get(imp_->experimental_, false);
}

auto Options::HelpText() const noexcept -> std::string_view
{
    return imp_->help();
}

auto Options::Home() const noexcept -> std::filesystem::path
{
    return Imp::get(imp_->home_);
}

auto Options::ImportOption(
    std::string_view key,
    std::string_view value) noexcept -> Options&
{
    imp_->import_value(key, value);

    return *this;
}

auto Options::Ipv4ConnectionMode() const noexcept -> ConnectionMode
{
    return Imp::get(imp_->ipv4_connection_mode_, ConnectionMode::automatic);
}

auto Options::Ipv6ConnectionMode() const noexcept -> ConnectionMode
{
    return Imp::get(imp_->ipv6_connection_mode_, ConnectionMode::automatic);
}

auto Options::LogLevel() const noexcept -> int
{
    return Imp::get(imp_->log_level_);
}

auto Options::MaxJobs() const noexcept -> unsigned int
{
    return Imp::get(imp_->max_jobs_);
}

auto Options::NotaryBindIP() const noexcept -> std::string_view
{
    return Imp::get(imp_->notary_bind_ip_);
}

auto Options::NotaryBindPort() const noexcept -> std::uint16_t
{
    return Imp::get(imp_->notary_bind_port_);
}

auto Options::NotaryInproc() const noexcept -> bool
{
    return Imp::get(imp_->notary_bind_inproc_);
}

auto Options::NotaryName() const noexcept -> std::string_view
{
    return Imp::get(imp_->notary_name_);
}

auto Options::NotaryPublicEEP() const noexcept -> const Set<CString>&
{
    return imp_->notary_public_eep_;
}

auto Options::NotaryPublicIPv4() const noexcept -> const Set<CString>&
{
    return imp_->notary_public_ipv4_;
}

auto Options::NotaryPublicIPv6() const noexcept -> const Set<CString>&
{
    return imp_->notary_public_ipv6_;
}

auto Options::NotaryPublicOnion() const noexcept -> const Set<CString>&
{
    return imp_->notary_public_onion_;
}

auto Options::NotaryPublicPort() const noexcept -> std::uint16_t
{
    return Imp::get(imp_->notary_public_port_);
}

auto Options::NotaryTerms() const noexcept -> std::string_view
{
    return Imp::get(imp_->notary_terms_);
}

auto Options::ParseCommandLine(int argc, char** argv) noexcept -> Options&
{
    try {
        imp_->parse(argc, argv);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }

    return *this;
}

auto Options::ProvideBlockchainSyncServer() const noexcept -> bool
{
    return Imp::get(imp_->blockchain_sync_server_enabled_);
}

auto Options::QtRootObject() const noexcept -> QObject*
{
    return imp_->qt_root_object_.value_or(nullptr);
}

auto Options::RemoteBlockchainSyncServers() const noexcept
    -> const Set<CString>&
{
    return imp_->blockchain_sync_servers_;
}

auto Options::RemoteLogEndpoint() const noexcept -> std::string_view
{
    return Imp::get(imp_->log_endpoint_);
}

auto Options::SetBlockchainProfile(opentxs::BlockchainProfile value) noexcept
    -> Options&
{
    imp_->blockchain_profile_ = value;

    return *this;
}

auto Options::SetBlockchainSyncEnabled(bool enabled) noexcept -> Options&
{
    imp_->blockchain_sync_server_enabled_ = enabled;
    imp_->blockchain_wallet_enabled_ = false;

    return *this;
}

auto Options::SetBlockchainWalletEnabled(bool enabled) noexcept -> Options&
{
    imp_->blockchain_wallet_enabled_ = enabled;

    return *this;
}

auto Options::SetDefaultMintKeyBytes(std::size_t bytes) noexcept -> Options&
{
    imp_->default_mint_key_bytes_ = bytes;

    return *this;
}

auto Options::SetExperimental(bool enabled) noexcept -> Options&
{
    imp_->experimental_ = enabled;

    return *this;
}

auto Options::SetHome(const std::filesystem::path& path) noexcept -> Options&
{
    imp_->home_ = path;

    return *this;
}

auto Options::SetIpv4ConnectionMode(ConnectionMode mode) noexcept -> Options&
{
    imp_->ipv4_connection_mode_ = mode;

    return *this;
}

auto Options::SetIpv6ConnectionMode(ConnectionMode mode) noexcept -> Options&
{
    imp_->ipv6_connection_mode_ = mode;

    return *this;
}

auto Options::SetLogEndpoint(std::string_view endpoint) noexcept -> Options&
{
    imp_->log_endpoint_ = endpoint;

    return *this;
}

auto Options::SetLogLevel(int level) noexcept -> Options&
{
    imp_->log_level_ = level;

    return *this;
}

auto Options::SetMaxJobs(unsigned int value) noexcept -> Options&
{
    imp_->max_jobs_ = value;

    return *this;
}

auto Options::SetNotaryBindIP(std::string_view value) noexcept -> Options&
{
    imp_->notary_bind_ip_ = value;

    return *this;
}

auto Options::SetNotaryBindPort(std::uint16_t port) noexcept -> Options&
{
    imp_->notary_bind_port_ = port;

    return *this;
}

auto Options::SetNotaryInproc(bool inproc) noexcept -> Options&
{
    imp_->notary_bind_inproc_ = inproc;

    return *this;
}

auto Options::SetNotaryName(std::string_view value) noexcept -> Options&
{
    imp_->notary_name_ = value;

    return *this;
}

auto Options::SetNotaryPublicPort(std::uint16_t port) noexcept -> Options&
{
    imp_->notary_public_port_ = port;

    return *this;
}

auto Options::SetNotaryTerms(std::string_view value) noexcept -> Options&
{
    imp_->notary_terms_ = value;

    return *this;
}

auto Options::SetQtRootObject(QObject* ptr) noexcept -> Options&
{
    imp_->qt_root_object_ = ptr;

    return *this;
}

auto Options::SetStoragePlugin(std::string_view name) noexcept -> Options&
{
    imp_->storage_primary_plugin_ = name;

    return *this;
}

auto Options::SetTestMode(bool test) noexcept -> Options&
{
    imp_->test_mode_ = test;

    return *this;
}

auto Options::StoragePrimaryPlugin() const noexcept -> std::string_view
{
    return Imp::get(imp_->storage_primary_plugin_);
}

auto Options::TestMode() const noexcept -> bool
{
    return Imp::get(imp_->test_mode_);
}

Options::~Options()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs
