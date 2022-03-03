// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Options.hpp"

class QObject;

namespace opentxs
{
struct Options::Imp final {
    UnallocatedSet<blockchain::Type> blockchain_disabled_chains_;
    UnallocatedSet<UnallocatedCString> blockchain_ipv4_bind_;
    UnallocatedSet<UnallocatedCString> blockchain_ipv6_bind_;
    std::optional<int> blockchain_storage_level_;
    std::optional<bool> blockchain_sync_server_enabled_;
    UnallocatedSet<UnallocatedCString> blockchain_sync_servers_;
    std::optional<bool> blockchain_wallet_enabled_;
    std::optional<std::size_t> default_mint_key_bytes_;
    std::optional<UnallocatedCString> home_;
    std::optional<UnallocatedCString> log_endpoint_;
    std::optional<ConnectionMode> ipv4_connection_mode_;
    std::optional<ConnectionMode> ipv6_connection_mode_;
    std::optional<int> log_level_;
    std::optional<bool> notary_bind_inproc_;
    std::optional<UnallocatedCString> notary_bind_ip_;
    std::optional<std::uint16_t> notary_bind_port_;
    std::optional<UnallocatedCString> notary_name_;
    UnallocatedSet<UnallocatedCString> notary_public_eep_;
    UnallocatedSet<UnallocatedCString> notary_public_ipv4_;
    UnallocatedSet<UnallocatedCString> notary_public_ipv6_;
    UnallocatedSet<UnallocatedCString> notary_public_onion_;
    std::optional<std::uint16_t> notary_public_port_;
    std::optional<UnallocatedCString> notary_terms_;
    std::optional<QObject*> qt_root_object_;
    std::optional<UnallocatedCString> storage_primary_plugin_;
    std::optional<bool> test_mode_;

    template <typename T>
    static auto get(const std::optional<T>& data, T defaultValue = {}) noexcept
        -> T
    {
        return data.value_or(defaultValue);
    }
    static auto get(const std::optional<UnallocatedCString>& data) noexcept
        -> const char*;
    static auto to_bool(const char* value) noexcept -> bool;

    auto help() const noexcept -> const UnallocatedCString&;

    auto import_value(const char* key, const char* value) noexcept -> void;
    auto parse(int argc, char** argv) noexcept(false) -> void;

    Imp() noexcept;
    Imp(const Imp& rhs) noexcept;

    ~Imp();

private:
    struct Parser;

    static auto lower(const UnallocatedCString& in) noexcept
        -> UnallocatedCString;

    auto convert(const UnallocatedCString& value) const noexcept(false)
        -> blockchain::Type;

    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};
}  // namespace opentxs
