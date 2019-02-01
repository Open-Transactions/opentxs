// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::server::implementation
{
class Manager final : opentxs::api::server::Manager, api::implementation::Core
{
public:
    void DropIncoming(const int count) const override;
    void DropOutgoing(const int count) const override;
    std::string GetAdminNym() const override;
    std::string GetAdminPassword() const override;
    std::string GetCommandPort() const override;
    std::string GetDefaultBindIP() const override;
    std::string GetEEP() const override;
    std::string GetExternalIP() const override;
    std::string GetInproc() const override;
    std::string GetListenCommand() const override;
    std::string GetListenNotify() const override;
    std::string GetOnion() const override;
#if OT_CASH
    std::shared_ptr<blind::Mint> GetPrivateMint(
        const Identifier& unitID,
        std::uint32_t series) const override;
    std::shared_ptr<const blind::Mint> GetPublicMint(
        const Identifier& unitID) const override;
#endif  // OT_CASH
    std::string GetUserName() const override;
    std::string GetUserTerms() const override;
    const Identifier& ID() const override;
    const identifier::Nym& NymID() const override;
#if OT_CASH
    void ScanMints() const override;
#endif  // OT_CASH
    opentxs::server::Server& Server() const override { return server_; }
#if OT_CASH
    void SetMintKeySize(const std::size_t size) const override
    {
        mint_key_size_.store(size);
    }
    void UpdateMint(const Identifier& unitID) const override;
#endif  // OT_CASH

    ~Manager();

private:
    friend opentxs::Factory;

#if OT_CASH
    typedef std::map<std::string, std::shared_ptr<blind::Mint>> MintSeries;
#endif  // OT_CASH

    std::unique_ptr<opentxs::server::Server> server_p_;
    opentxs::server::Server& server_;
    std::unique_ptr<opentxs::server::MessageProcessor> message_processor_p_;
    opentxs::server::MessageProcessor& message_processor_;
#if OT_CASH
    std::thread mint_thread_;
    mutable std::mutex mint_lock_;
    mutable std::mutex mint_update_lock_;
    mutable std::mutex mint_scan_lock_;
    mutable std::map<std::string, MintSeries> mints_;
    mutable std::deque<std::string> mints_to_check_;
    mutable std::atomic<std::size_t> mint_key_size_;
#endif  // OT_CASH

#if OT_CASH
    void generate_mint(
        const std::string& serverID,
        const std::string& unitID,
        const std::uint32_t series) const;
#endif  // OT_CASH
    const std::string get_arg(const std::string& argName) const;
#if OT_CASH
    std::int32_t last_generated_series(
        const std::string& serverID,
        const std::string& unitID) const;
    std::shared_ptr<blind::Mint> load_private_mint(
        const Lock& lock,
        const std::string& unitID,
        const std::string seriesID) const;
    std::shared_ptr<blind::Mint> load_public_mint(
        const Lock& lock,
        const std::string& unitID,
        const std::string seriesID) const;
    void mint() const;
#endif  // OT_CASH
    bool verify_lock(const Lock& lock, const std::mutex& mutex) const;
#if OT_CASH
    std::shared_ptr<blind::Mint> verify_mint(
        const Lock& lock,
        const std::string& unitID,
        const std::string seriesID,
        std::shared_ptr<blind::Mint>& mint) const;
    bool verify_mint_directory(const std::string& serverID) const;
#endif  // OT_CASH

    void Cleanup();
    void Init();
    void Start() override;

    Manager(
        const api::Native& parent,
        Flag& running,
        const ArgList& args,
        const api::Crypto& crypto,
        const api::Settings& config,
        const opentxs::network::zeromq::Context& context,
        const std::string& dataFolder,
        const int instance);
    Manager() = delete;
    Manager(const Manager&) = delete;
    Manager(Manager&&) = delete;
    Manager& operator=(const Manager&) = delete;
    Manager& operator=(Manager&&) = delete;
};
}  // namespace opentxs::api::server::implementation
