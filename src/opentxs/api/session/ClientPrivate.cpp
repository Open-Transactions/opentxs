// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/ClientPrivate.hpp"  // IWYU pragma: associated

#include <memory>
#include <mutex>
#include <utility>

#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/crypto/Factory.hpp"
#include "internal/api/network/Blockchain.hpp"
#include "internal/api/network/Factory.hpp"
#include "internal/api/network/Network.hpp"
#include "internal/api/session/Contacts.hpp"
#include "internal/api/session/Crypto.hpp"
#include "internal/api/session/UI.hpp"
#include "internal/otx/client/Factory.hpp"
#include "internal/otx/client/Pair.hpp"
#include "internal/otx/client/ServerAction.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "internal/otx/client/obsolete/OT_API.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Context.internal.hpp"
#include "opentxs/api/SessionPrivate.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/session/Activity.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.internal.hpp"  // IWYU pragma: keep
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/api/session/UI.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/api/session/Workflow.hpp"
#include "opentxs/api/session/base/Storage.hpp"
#include "opentxs/api/session/internal.factory.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/internal.factory.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"

namespace opentxs::api::session
{
ClientPrivate::ClientPrivate(
    const api::Context& parent,
    Flag& running,
    Options&& args,
    const api::Settings& config,
    const api::Crypto& crypto,
    const opentxs::network::zeromq::Context& context,
    const std::filesystem::path& dataFolder,
    const int instance)
    : SessionPrivate(
          parent,
          running,
          std::move(args),
          crypto,
          config,
          context,
          dataFolder,
          instance,
          [&](const auto& zmq, const auto& endpoints, auto& scheduler) {
              return factory::NetworkAPI(
                  *this,
                  parent.Asio(),
                  zmq,
                  endpoints,
                  factory::BlockchainNetworkAPI(self_, endpoints, zmq));
          },
          factory::SessionFactoryAPI(
              *static_cast<internal::Client*>(this),
              parent.Factory()))
    , self_(this)
    , zeromq_(opentxs::Factory::ZMQ(self_, running_))
    , contacts_(factory::ContactAPI(self_))
    , activity_(factory::ActivityAPI(self_, *contacts_))
    , blockchain_(factory::BlockchainAPI(
          self_,
          *activity_,
          *contacts_,
          parent_.Internal().Paths(),
          dataFolder.string(),
          args_))
    , workflow_(factory::Workflow(self_, *activity_, *contacts_))
    , ot_api_(new OT_API(
          self_,
          *activity_,
          *contacts_,
          *workflow_,
          *zeromq_,
          [this](const auto& id) -> auto& { return get_lock(id); }))
    , otapi_exec_(new OTAPI_Exec(
          self_,
          *activity_,
          *contacts_,
          *zeromq_,
          *ot_api_,
          [this](const auto& id) -> auto& { return get_lock(id); }))
    , server_action_(factory::ServerAction(
          self_,
          [this](const auto& id) -> auto& { return get_lock(id); }))
    , otx_(factory::OTX(
          running_,
          self_,
          [this](const auto& id) -> auto& { return get_lock(id); }))
    , pair_(factory::PairAPI(running_, self_))
    , ui_(factory::UI(self_, *blockchain_, running_))
    , map_lock_()
    , context_locks_()
    , me_()
{
    wallet_ = factory::WalletAPI(self_);

    assert_false(nullptr == wallet_);
    assert_false(nullptr == zeromq_);
    assert_false(nullptr == contacts_);
    assert_false(nullptr == activity_);
    assert_false(nullptr == blockchain_);
    assert_false(nullptr == workflow_);
    assert_false(nullptr == ot_api_);
    assert_false(nullptr == otapi_exec_);
    assert_false(nullptr == server_action_);
    assert_false(nullptr == otx_);
    assert_false(nullptr == ui_);
    assert_false(nullptr == pair_);
}

auto ClientPrivate::Activity() const -> const session::Activity&
{
    assert_false(nullptr == activity_);

    return *activity_;
}

auto ClientPrivate::Cleanup() -> void
{
    LogDetail()()("Shutting down and cleaning up.").Flush();
    ui_->Internal().Shutdown();
    ui_.reset();
    pair_.reset();
    otx_.reset();
    server_action_.reset();
    otapi_exec_.reset();
    ot_api_.reset();
    workflow_.reset();
    network_->Internal().Shutdown();
    contacts_->Internal().prepare_shutdown();
    crypto_.InternalSession().PrepareShutdown();
    blockchain_.reset();
    activity_.reset();
    contacts_.reset();
    zeromq_.reset();
    SessionPrivate::cleanup();
}

auto ClientPrivate::Contacts() const -> const api::session::Contacts&
{
    assert_false(nullptr == contacts_);

    return *contacts_;
}

auto ClientPrivate::get_lock(const ContextID context) const
    -> std::recursive_mutex&
{
    opentxs::Lock lock(map_lock_);

    return context_locks_[context];
}

auto ClientPrivate::Exec(const UnallocatedCString&) const -> const OTAPI_Exec&
{
    assert_false(nullptr == otapi_exec_);

    return *otapi_exec_;
}

auto ClientPrivate::GetShared() const noexcept
    -> std::shared_ptr<const api::internal::Session>
{
    return SharedClient();
}

auto ClientPrivate::Init() -> void
{
    contacts_->Internal().init(blockchain_);
    crypto_.InternalSession().Init(blockchain_);
    Storage::init(crypto_, factory_, crypto_.Seed());
    StartContacts();
    pair_->init();
    blockchain_->Internal().Init();
    ui_->Internal().Init();
}

auto ClientPrivate::Lock(
    const identifier::Nym& nymID,
    const identifier::Notary& serverID) const -> std::recursive_mutex&
{
    return get_lock({nymID.asBase58(crypto_), serverID.asBase58(crypto_)});
}

auto ClientPrivate::NewNym(const identifier::Nym& id) const noexcept -> void
{
    SessionPrivate::NewNym(id);
    blockchain_->Internal().NewNym(id);
}

auto ClientPrivate::OTAPI(const UnallocatedCString&) const -> const OT_API&
{
    assert_false(nullptr == ot_api_);

    return *ot_api_;
}

auto ClientPrivate::OTX() const -> const api::session::OTX&
{
    assert_false(nullptr == otx_);

    return *otx_;
}

auto ClientPrivate::Pair() const -> const otx::client::Pair&
{
    assert_false(nullptr == pair_);

    return *pair_;
}

auto ClientPrivate::ServerAction() const -> const otx::client::ServerAction&
{
    assert_false(nullptr == server_action_);

    return *server_action_;
}

auto ClientPrivate::SharedClient() const noexcept
    -> std::shared_ptr<const internal::Client>
{
    wait_for_init();
    auto out = me_.lock();

    assert_false(nullptr == out);

    return out;
}

auto ClientPrivate::Start(std::shared_ptr<internal::Client> api) noexcept
    -> void
{
    me_ = api;
    auto me = me_.lock();

    assert_false(nullptr == me);

    SessionPrivate::start(api);
    network_->Internal().Start(
        me,
        crypto_.Blockchain(),
        parent_.Internal().Paths(),
        data_folder_,
        args_);
    blockchain_->Internal().Start(std::move(me));
    StartBlockchain();
}

auto ClientPrivate::StartBlockchain() noexcept -> void
{
    for (const auto chain : args_.DisabledBlockchains()) {
        network_->Blockchain().Disable(chain);
    }

    network_->Blockchain().Internal().RestoreNetworks();
}

auto ClientPrivate::StartContacts() -> void
{
    assert_false(nullptr == contacts_);

    contacts_->Internal().start();
}

auto ClientPrivate::UI() const -> const api::session::UI&
{
    assert_false(nullptr == ui_);

    return *ui_;
}

auto ClientPrivate::Workflow() const -> const session::Workflow&
{
    assert_false(nullptr == workflow_);

    return *workflow_;
}

auto ClientPrivate::ZMQ() const -> const api::network::ZMQ&
{
    assert_false(nullptr == zeromq_);

    return *zeromq_;
}

ClientPrivate::~ClientPrivate()
{
    running_.Off();
    Cleanup();
    shutdown_complete();
    Detach(self_);
}
}  // namespace opentxs::api::session
