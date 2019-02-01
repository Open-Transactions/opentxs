// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::client::implementation
{
class Manager final : opentxs::api::client::internal::Manager,
                      api::implementation::Core
{
public:
    const api::client::Activity& Activity() const override;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    const api::client::Blockchain& Blockchain() const override;
#endif
    const api::client::Contacts& Contacts() const override;
    const OTAPI_Exec& Exec(const std::string& wallet = "") const override;
    std::recursive_mutex& Lock(
        const identifier::Nym& nymID,
        const Identifier& serverID) const override;
    const OT_API& OTAPI(const std::string& wallet = "") const override;
    const client::OTX& OTX() const override;
    const api::client::Pair& Pair() const override;
    const client::ServerAction& ServerAction() const override;
    const api::client::UI& UI() const override;
    const client::Workflow& Workflow() const override;
    const api::network::ZMQ& ZMQ() const override;

    void StartActivity() override;
    void StartContacts() override;
    opentxs::OTWallet* StartWallet() override;

    ~Manager();

private:
    friend opentxs::Factory;

    std::unique_ptr<api::network::ZMQ> zeromq_;
    std::unique_ptr<api::Identity> identity_;
    std::unique_ptr<api::client::internal::Contacts> contacts_;
    std::unique_ptr<api::client::internal::Activity> activity_;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    std::unique_ptr<api::client::Blockchain> blockchain_;
#endif
    std::unique_ptr<api::client::Workflow> workflow_;
    std::unique_ptr<OT_API> ot_api_;
    std::unique_ptr<OTAPI_Exec> otapi_exec_;
    std::unique_ptr<api::client::ServerAction> server_action_;
    std::unique_ptr<api::client::OTX> otx_;
    std::unique_ptr<api::client::Pair> pair_;
    std::unique_ptr<api::client::UI> ui_;
    mutable std::recursive_mutex lock_;
    mutable std::mutex map_lock_;
    mutable std::map<ContextID, std::recursive_mutex> context_locks_;

    std::recursive_mutex& get_lock(const ContextID context) const;

    void Cleanup();
    void Init();

    Manager(
        const api::Native& parent,
        Flag& running,
        const ArgList& args,
        const api::Settings& config,
        const api::Crypto& crypto,
        const opentxs::network::zeromq::Context& context,
        const std::string& dataFolder,
        const int instance);
    Manager() = delete;
    Manager(const Manager&) = delete;
    Manager(Manager&&) = delete;
    Manager& operator=(const Manager&) = delete;
    Manager& operator=(Manager&&) = delete;
};
}  // namespace opentxs::api::client::implementation
