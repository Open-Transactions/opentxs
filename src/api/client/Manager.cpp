// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Activity.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/api/client/Blockchain.hpp"
#endif
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/client/Pair.hpp"
#include "opentxs/api/client/ServerAction.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Identity.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/client/OTWallet.hpp"
#include "opentxs/client/SwigWrap.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"

#include "api/storage/StorageInternal.hpp"
#include "api/Core.hpp"
#include "internal/api/Internal.hpp"
#include "InternalClient.hpp"

#include <set>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "Manager.hpp"

namespace opentxs
{
api::client::internal::Manager* Factory::ClientManager(
    const api::Native& parent,
    Flag& running,
    const ArgList& args,
    const api::Settings& config,
    const api::Crypto& crypto,
    const network::zeromq::Context& context,
    const std::string& dataFolder,
    const int instance)
{
    return new api::client::implementation::Manager(
        parent, running, args, config, crypto, context, dataFolder, instance);
}
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
Manager::Manager(
    const api::Native& parent,
    Flag& running,
    const ArgList& args,
    const api::Settings& config,
    const api::Crypto& crypto,
    const opentxs::network::zeromq::Context& context,
    const std::string& dataFolder,
    const int instance)
    : Core(
          parent,
          running,
          args,
          crypto,
          config,
          context,
          dataFolder,
          instance,
          false,
          opentxs::Factory::FactoryAPIClient(*this))
    , zeromq_(opentxs::Factory::ZMQ(*this, running_))
    , identity_(opentxs::Factory::Identity(*this))
    , contacts_(opentxs::Factory::Contacts(*this))
    , activity_(opentxs::Factory::Activity(*this, *contacts_))
#if OT_CRYPTO_SUPPORTED_KEY_HD
    , blockchain_(opentxs::Factory::Blockchain(*this, *activity_))
#endif
    , workflow_(opentxs::Factory::Workflow(*this, *activity_, *contacts_))
    , ot_api_(new OT_API(
          *this,
          *activity_,
          *contacts_,
          *workflow_,
          *zeromq_,
          std::bind(&Manager::get_lock, this, std::placeholders::_1)))
    , otapi_exec_(new OTAPI_Exec(
          *this,
          *activity_,
          *contacts_,
          *zeromq_,
          *identity_,
          *ot_api_,
          std::bind(&Manager::get_lock, this, std::placeholders::_1)))
    , server_action_(opentxs::Factory::ServerAction(
          *this,
          std::bind(&Manager::get_lock, this, std::placeholders::_1)))
    , otx_(opentxs::Factory::OTX(
          running_,
          *this,
          *ot_api_->m_pClient,
          std::bind(&Manager::get_lock, this, std::placeholders::_1)))
    , pair_(opentxs::Factory::Pair(running_, *this))
    , ui_(opentxs::Factory::UI(*this, running_))
    , lock_()
    , map_lock_()
    , context_locks_()
{
    wallet_.reset(opentxs::Factory::Wallet(*this));

    OT_ASSERT(wallet_);
    OT_ASSERT(zeromq_);
    OT_ASSERT(identity_);
    OT_ASSERT(contacts_);
    OT_ASSERT(activity_);
#if OT_CRYPTO_SUPPORTED_KEY_HD
    OT_ASSERT(blockchain_);
#endif
    OT_ASSERT(workflow_);
    OT_ASSERT(ot_api_);
    OT_ASSERT(otapi_exec_);
    OT_ASSERT(server_action_);
    OT_ASSERT(otx_);
    OT_ASSERT(ui_);
    OT_ASSERT(pair_);

    if (0 == instance_) {
        SwigWrap::client_ = this;

        OT_ASSERT(nullptr != SwigWrap::client_)
    }

    Init();
}

const api::client::Activity& Manager::Activity() const
{
    OT_ASSERT(activity_)

    return *activity_;
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
const api::client::Blockchain& Manager::Blockchain() const
{
    OT_ASSERT(blockchain_)

    return *blockchain_;
}
#endif

void Manager::Cleanup()
{
    if (0 == instance_) { SwigWrap::client_ = nullptr; }

    ui_.reset();
    pair_.reset();
    otx_.reset();
    server_action_.reset();
    otapi_exec_.reset();
    ot_api_.reset();
    workflow_.reset();
#if OT_CRYPTO_SUPPORTED_KEY_HD
    blockchain_.reset();
#endif
    activity_.reset();
    contacts_.reset();
    identity_.reset();
    zeromq_.reset();
    Core::cleanup();
}

const api::client::Contacts& Manager::Contacts() const
{
    OT_ASSERT(contacts_)

    return *contacts_;
}

std::recursive_mutex& Manager::get_lock(const ContextID context) const
{
    opentxs::Lock lock(map_lock_);

    return context_locks_[context];
}

const OTAPI_Exec& Manager::Exec(const std::string&) const
{
    OT_ASSERT(otapi_exec_);

    return *otapi_exec_;
}

void Manager::Init()
{
    StartWallet();

    OT_ASSERT(seeds_)

    StorageParent::init(*seeds_);
    StartContacts();
    StartActivity();
}

std::recursive_mutex& Manager::Lock(
    const identifier::Nym& nymID,
    const Identifier& serverID) const
{
    return get_lock({nymID.str(), serverID.str()});
}

const OT_API& Manager::OTAPI(const std::string&) const
{
    OT_ASSERT(ot_api_);

    return *ot_api_;
}

const api::client::OTX& Manager::OTX() const
{
    OT_ASSERT(otx_);

    return *otx_;
}

const api::client::Pair& Manager::Pair() const
{
    OT_ASSERT(pair_);

    return *pair_;
}

const api::client::ServerAction& Manager::ServerAction() const
{
    OT_ASSERT(server_action_);

    return *server_action_;
}

void Manager::StartActivity()
{
    OT_ASSERT(activity_)

    activity_->MigrateLegacyThreads();

    OT_ASSERT(dht_)

    Scheduler::Start(storage_.get(), dht_.get());
}

void Manager::StartContacts()
{
    OT_ASSERT(contacts_);

    contacts_->start();
}

opentxs::OTWallet* Manager::StartWallet()
{
    OT_ASSERT(ot_api_)

    const bool loaded = ot_api_->LoadWallet();

    OT_ASSERT(loaded);

    auto wallet = ot_api_->GetWallet(nullptr);

    OT_ASSERT(nullptr != wallet);

    wallet->SaveWallet();

    return wallet;
}

const api::client::UI& Manager::UI() const
{
    OT_ASSERT(ui_)

    return *ui_;
}

const api::client::Workflow& Manager::Workflow() const
{
    OT_ASSERT(workflow_);

    return *workflow_;
}

const api::network::ZMQ& Manager::ZMQ() const
{
    OT_ASSERT(zeromq_);

    return *zeromq_;
}

Manager::~Manager()
{
    running_.Off();
    Cleanup();
}
}  // namespace opentxs::api::client::implementation
