// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Container.hpp"

class QObject;

namespace opentxs
{
class OPENTXS_EXPORT Options final
{
public:
    enum class ConnectionMode {
        off = -1,
        automatic = 0,
        on = 1,
    };

    auto BlockchainBindIpv4() const noexcept
        -> const UnallocatedSet<UnallocatedCString>&;
    auto BlockchainBindIpv6() const noexcept
        -> const UnallocatedSet<UnallocatedCString>&;
    auto BlockchainStorageLevel() const noexcept -> int;
    auto BlockchainWalletEnabled() const noexcept -> bool;
    auto DefaultMintKeyBytes() const noexcept -> std::size_t;
    auto DisabledBlockchains() const noexcept
        -> UnallocatedSet<blockchain::Type>;
    auto HelpText() const noexcept -> const UnallocatedCString&;
    auto Home() const noexcept -> const char*;
    auto Ipv4ConnectionMode() const noexcept -> ConnectionMode;
    auto Ipv6ConnectionMode() const noexcept -> ConnectionMode;
    auto LogLevel() const noexcept -> int;
    auto NotaryBindIP() const noexcept -> const char*;
    auto NotaryBindPort() const noexcept -> std::uint16_t;
    auto NotaryInproc() const noexcept -> bool;
    auto NotaryName() const noexcept -> const char*;
    auto NotaryPublicEEP() const noexcept
        -> const UnallocatedSet<UnallocatedCString>&;
    auto NotaryPublicIPv4() const noexcept
        -> const UnallocatedSet<UnallocatedCString>&;
    auto NotaryPublicIPv6() const noexcept
        -> const UnallocatedSet<UnallocatedCString>&;
    auto NotaryPublicOnion() const noexcept
        -> const UnallocatedSet<UnallocatedCString>&;
    auto NotaryPublicPort() const noexcept -> std::uint16_t;
    auto NotaryTerms() const noexcept -> const char*;
    auto ProvideBlockchainSyncServer() const noexcept -> bool;
    auto QtRootObject() const noexcept -> QObject*;
    auto RemoteBlockchainSyncServers() const noexcept
        -> const UnallocatedSet<UnallocatedCString>&;
    auto RemoteLogEndpoint() const noexcept -> const char*;
    auto StoragePrimaryPlugin() const noexcept -> const char*;
    auto TestMode() const noexcept -> bool;

    auto AddBlockchainIpv4Bind(const char* endpoint) noexcept -> Options&;
    auto AddBlockchainIpv6Bind(const char* endpoint) noexcept -> Options&;
    auto AddBlockchainSyncServer(const char* endpoint) noexcept -> Options&;
    auto AddNotaryPublicEEP(const char* value) noexcept -> Options&;
    auto AddNotaryPublicIPv4(const char* value) noexcept -> Options&;
    auto AddNotaryPublicIPv6(const char* value) noexcept -> Options&;
    auto AddNotaryPublicOnion(const char* value) noexcept -> Options&;
    auto DisableBlockchain(blockchain::Type chain) noexcept -> Options&;
    OPENTXS_NO_EXPORT auto ImportOption(
        const char* key,
        const char* value) noexcept -> Options&;
    auto ParseCommandLine(int argc, char** argv) noexcept -> Options&;
    auto SetBlockchainStorageLevel(int value) noexcept -> Options&;
    auto SetBlockchainSyncEnabled(bool enabled) noexcept -> Options&;
    auto SetBlockchainWalletEnabled(bool enabled) noexcept -> Options&;
    auto SetDefaultMintKeyBytes(std::size_t bytes) noexcept -> Options&;
    auto SetHome(const char* path) noexcept -> Options&;
    auto SetIpv4ConnectionMode(ConnectionMode mode) noexcept -> Options&;
    auto SetIpv6ConnectionMode(ConnectionMode mode) noexcept -> Options&;
    auto SetLogEndpoint(const char* endpoint) noexcept -> Options&;
    auto SetLogLevel(int level) noexcept -> Options&;
    auto SetNotaryBindIP(const char* value) noexcept -> Options&;
    auto SetNotaryBindPort(std::uint16_t port) noexcept -> Options&;
    auto SetNotaryInproc(bool inproc) noexcept -> Options&;
    auto SetNotaryName(const char* value) noexcept -> Options&;
    auto SetNotaryPublicPort(std::uint16_t port) noexcept -> Options&;
    auto SetNotaryTerms(const char* value) noexcept -> Options&;
    auto SetQtRootObject(QObject*) noexcept -> Options&;
    auto SetStoragePlugin(const char* name) noexcept -> Options&;
    auto SetTestMode(bool test) noexcept -> Options&;

    Options() noexcept;
    Options(int argc, char** argv) noexcept;
    Options(const Options& rhs) noexcept;
    Options(Options&& rhs) noexcept;

    ~Options();

private:
    friend auto operator+(const Options&, const Options&) noexcept -> Options;

    struct Imp;

    Imp* imp_;

    auto operator=(const Options&) -> Options& = delete;
    auto operator=(Options&&) noexcept -> Options& = delete;
};

auto operator+(const Options& lhs, const Options& rhs) noexcept -> Options;
constexpr auto value(Options::ConnectionMode val) noexcept -> int
{
    return static_cast<int>(val);
}
}  // namespace opentxs
