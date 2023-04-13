// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string_view>

#include "internal/util/Options.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/Types.hpp"

class QObject;

namespace opentxs
{
// NOLINTBEGIN(clang-analyzer-optin.performance.Padding)
struct Options::Imp final : public internal::Options {
    Set<blockchain::Type> blockchain_disabled_chains_;
    Set<blockchain::Type> blockchain_reset_cfilter_;
    Set<CString> blockchain_ipv4_bind_;
    Set<CString> blockchain_ipv6_bind_;
    std::optional<opentxs::BlockchainProfile> blockchain_profile_;
    std::optional<bool> blockchain_sync_server_enabled_;
    Set<CString> blockchain_sync_servers_;
    std::optional<bool> blockchain_wallet_enabled_;
    std::optional<bool> debug_allocations_;
    std::optional<std::size_t> default_mint_key_bytes_;
    std::optional<bool> experimental_;
    std::optional<std::filesystem::path> home_;
    std::optional<ConnectionMode> ipv4_connection_mode_;
    std::optional<ConnectionMode> ipv6_connection_mode_;
    std::optional<CString> log_endpoint_;
    std::optional<int> log_level_;
    std::optional<bool> loopback_dht_;
    std::optional<unsigned int> max_jobs_;
    std::optional<bool> notary_bind_inproc_;
    std::optional<CString> notary_bind_ip_;
    std::optional<std::uint16_t> notary_bind_port_;
    std::optional<CString> notary_name_;
    Set<CString> notary_public_eep_;
    Set<CString> notary_public_ipv4_;
    Set<CString> notary_public_ipv6_;
    Set<CString> notary_public_onion_;
    std::optional<std::uint16_t> notary_public_port_;
    std::optional<CString> notary_terms_;
    Vector<Listener> otdht_listeners_;
    std::optional<QObject*> qt_root_object_;
    std::optional<CString> storage_primary_plugin_;
    std::optional<bool> test_mode_;

    template <typename T>
    static auto get(const std::optional<T>& data, T defaultValue = {}) noexcept
        -> T
    {
        return data.value_or(defaultValue);
    }
    static auto get(const std::optional<CString>& data) noexcept
        -> std::string_view;
    static auto to_bool(std::string_view value) noexcept -> bool;

    auto help() const noexcept -> std::string_view;

    auto import_value(std::string_view key, std::string_view value) noexcept
        -> void;
    auto OTDHTListeners() const noexcept -> std::span<const Listener> final
    {
        return otdht_listeners_;
    }
    auto parse(int argc, char** argv) noexcept(false) -> void;

    Imp() noexcept;
    Imp(const Imp& rhs) noexcept;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp() final;

private:
    struct Parser;

    static auto lower(std::string_view in) noexcept -> CString;

    auto convert(std::string_view value) const noexcept(false)
        -> blockchain::Type;
};
// NOLINTEND(clang-analyzer-optin.performance.Padding)
}  // namespace opentxs
