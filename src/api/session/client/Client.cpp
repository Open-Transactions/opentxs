// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "1_Internal.hpp"                 // IWYU pragma: associated
#include "api/session/client/Client.hpp"  // IWYU pragma: associated

#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <utility>

#include "2_Factory.hpp"
#include "api/session/Session.hpp"
#include "api/session/base/Scheduler.hpp"
#include "api/session/base/Storage.hpp"
#include "core/Shutdown.hpp"
#include "internal/api/Context.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/crypto/Factory.hpp"
#include "internal/api/network/Blockchain.hpp"
#include "internal/api/network/Factory.hpp"
#include "internal/api/session/Contacts.hpp"
#include "internal/api/session/Crypto.hpp"
#include "internal/api/session/Factory.hpp"
#include "internal/api/session/UI.hpp"
#include "internal/otx/client/Factory.hpp"
#include "internal/otx/client/Pair.hpp"
#include "internal/otx/client/ServerAction.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "internal/otx/client/obsolete/OT_API.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/session/Activity.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/api/session/UI.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Workflow.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"

namespace opentxs::factory
{
auto ClientSession(
    const api::Context& parent,
    Flag& running,
    Options&& args,
    const api::Settings& config,
    const api::Crypto& crypto,
    const network::zeromq::Context& context,
    const UnallocatedCString& dataFolder,
    const int instance) noexcept -> std::unique_ptr<api::session::Client>
{
    using ReturnType = api::session::imp::Client;

    try {
        return std::make_unique<ReturnType>(
            parent,
            running,
            std::move(args),
            config,
            crypto,
            context,
            dataFolder,
            instance);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::api::session::imp
{
Client::Client(
    const api::Context& parent,
    Flag& running,
    Options&& args,
    const api::Settings& config,
    const api::Crypto& crypto,
    const opentxs::network::zeromq::Context& context,
    const UnallocatedCString& dataFolder,
    const int instance)
    : Session(
          parent,
          running,
          std::move(args),
          crypto,
          config,
          context,
          dataFolder,
          instance,
          [&](const auto& zmq, const auto& endpoints, auto& config) {
              return factory::NetworkAPI(
                  *this,
                  parent.Asio(),
                  zmq,
                  endpoints,
                  factory::BlockchainNetworkAPI(*this, endpoints, zmq),
                  config,
                  false);
          },
          factory::SessionFactoryAPI(*this))
    , zeromq_(opentxs::Factory::ZMQ(*this, running_))
    , contacts_(factory::ContactAPI(*this))
    , activity_(factory::ActivityAPI(*this, *contacts_))
    , blockchain_(factory::BlockchainAPI(
          *this,
          *activity_,
          *contacts_,
          parent_.Internal().Legacy(),
          dataFolder,
          args_))
    , workflow_(factory::Workflow(*this, *activity_, *contacts_))
    , ot_api_(new OT_API(
          *this,
          *activity_,
          *contacts_,
          *workflow_,
          *zeromq_,
          std::bind(&Client::get_lock, this, std::placeholders::_1)))
    , otapi_exec_(new OTAPI_Exec(
          *this,
          *activity_,
          *contacts_,
          *zeromq_,
          *ot_api_,
          std::bind(&Client::get_lock, this, std::placeholders::_1)))
    , server_action_(factory::ServerAction(
          *this,
          std::bind(&Client::get_lock, this, std::placeholders::_1)))
    , otx_(factory::OTX(
          running_,
          *this,
          std::bind(&Client::get_lock, this, std::placeholders::_1)))
    , pair_(factory::PairAPI(running_, *this))
    , ui_(factory::UI(*this, *blockchain_, running_))
    , map_lock_()
    , context_locks_()
{
    wallet_ = factory::WalletAPI(*this);

    OT_ASSERT(wallet_);
    OT_ASSERT(zeromq_);
    OT_ASSERT(contacts_);
    OT_ASSERT(activity_);
    OT_ASSERT(blockchain_);
    OT_ASSERT(workflow_);
    OT_ASSERT(ot_api_);
    OT_ASSERT(otapi_exec_);
    OT_ASSERT(server_action_);
    OT_ASSERT(otx_);
    OT_ASSERT(ui_);
    OT_ASSERT(pair_);

    network_.Blockchain().Internal().Init(
        *blockchain_, parent_.Internal().Legacy(), dataFolder, args_);
}

auto Client::Activity() const -> const session::Activity&
{
    OT_ASSERT(activity_)

    return *activity_;
}

auto Client::Cleanup() -> void
{
    LogDetail()(OT_PRETTY_CLASS())("Shutting down and cleaning up.").Flush();
    shutdown_sender_.Activate();
    ui_->Internal().Shutdown();
    ui_.reset();
    pair_.reset();
    otx_.reset();
    server_action_.reset();
    otapi_exec_.reset();
    ot_api_.reset();
    workflow_.reset();
#if OT_BLOCKCHAIN
    network_.Blockchain().Internal().Shutdown();
    contacts_->Internal().prepare_shutdown();
#endif  // OT_BLOCKCHAIN
    crypto_.InternalSession().PrepareShutdown();
    blockchain_.reset();
    activity_.reset();
    contacts_.reset();
    zeromq_.reset();
    Session::cleanup();
}

auto Client::Contacts() const -> const api::session::Contacts&
{
    OT_ASSERT(contacts_)

    return *contacts_;
}

auto Client::get_lock(const ContextID context) const -> std::recursive_mutex&
{
    opentxs::Lock lock(map_lock_);

    return context_locks_[context];
}

auto Client::Exec(const UnallocatedCString&) const -> const OTAPI_Exec&
{
    OT_ASSERT(otapi_exec_);

    return *otapi_exec_;
}

auto Client::Init() -> void
{
#if OT_BLOCKCHAIN
    contacts_->Internal().init(blockchain_);
    crypto_.InternalSession().Init(blockchain_);
#endif  // OT_BLOCKCHAIN

    Storage::init(factory_, crypto_.Seed());
    StartContacts();
    StartActivity();
    pair_->init();
    blockchain_->Internal().Init();
    StartBlockchain();
    ui_->Internal().Init();
}

auto Client::Lock(
    const identifier::Nym& nymID,
    const identifier::Notary& serverID) const -> std::recursive_mutex&
{
    return get_lock({nymID.str(), serverID.str()});
}

auto Client::NewNym(const identifier::Nym& id) const noexcept -> void
{
    Session::NewNym(id);
    blockchain_->Internal().NewNym(id);
}

auto Client::OTAPI(const UnallocatedCString&) const -> const OT_API&
{
    OT_ASSERT(ot_api_);

    return *ot_api_;
}

auto Client::OTX() const -> const api::session::OTX&
{
    OT_ASSERT(otx_);

    return *otx_;
}

auto Client::Pair() const -> const otx::client::Pair&
{
    OT_ASSERT(pair_);

    return *pair_;
}

auto Client::ServerAction() const -> const otx::client::ServerAction&
{
    OT_ASSERT(server_action_);

    return *server_action_;
}

void Client::StartActivity()
{
    Scheduler::Start(storage_.get(), network_.DHT());
}

auto Client::StartBlockchain() noexcept -> void
{
#if OT_BLOCKCHAIN
    for (const auto chain : args_.DisabledBlockchains()) {
        network_.Blockchain().Disable(chain);
    }

    network_.Blockchain().Internal().RestoreNetworks();
#endif  // OT_BLOCKCHAIN
}

void Client::StartContacts()
{
    OT_ASSERT(contacts_);

    contacts_->Internal().start();
}

auto Client::UI() const -> const api::session::UI&
{
    OT_ASSERT(ui_)

    return *ui_;
}

auto Client::Workflow() const -> const session::Workflow&
{
    OT_ASSERT(workflow_);

    return *workflow_;
}

auto Client::ZMQ() const -> const api::network::ZMQ&
{
    OT_ASSERT(zeromq_);

    return *zeromq_;
}

Client::~Client()
{
    running_.Off();
    Cleanup();
}
}  // namespace opentxs::api::session::imp
