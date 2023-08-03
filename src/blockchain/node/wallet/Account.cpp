// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/Account.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <algorithm>
#include <chrono>
#include <span>
#include <string_view>
#include <utility>

#include "blockchain/node/wallet/subchain/NotificationStateData.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/wallet/Reorg.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Notification.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Contact.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/AccountSubtype.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"   // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::blockchain::node::wallet
{
auto print(AccountJobs job) noexcept -> std::string_view
{
    try {
        using enum AccountJobs;
        static const auto map = Map<AccountJobs, CString>{
            {shutdown, "shutdown"},
            {contact, "contact"},
            {subaccount, "subaccount"},
            {prepare_reorg, "prepare_reorg"},
            {rescan, "rescan"},
            {finish_reorg, "finish_reorg"},
            {init, "init"},
            {key, "key"},
            {prepare_shutdown, "prepare_shutdown"},
            {statemachine, "statemachine"},
        };

        return map.at(job);
    } catch (...) {
        LogAbort()(__FUNCTION__)("invalid AccountJobs: ")(
            static_cast<OTZMQWorkType>(job))
            .Abort();
    }
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
using enum opentxs::network::zeromq::socket::Direction;

Account::Imp::Imp(
    Reorg& reorg,
    const crypto::Account& account,
    std::shared_ptr<const api::session::Client> api,
    std::shared_ptr<const node::Manager> node,
    CString&& fromParent,
    network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Actor(
          *api,
          LogTrace(),
          [&] {
              using namespace std::literals;

              return CString{alloc}
                  .append(print(node->Internal().Chain()))
                  .append(" account for "sv)
                  .append(account.NymID().asBase58(api->Crypto()));
          }(),
          0ms,
          batch,
          alloc,
          {
              {fromParent, Connect},
              {api->Crypto().Blockchain().Internal().KeyEndpoint(), Connect},
              {api->Endpoints().BlockchainAccountCreated(), Connect},
          })
    , api_p_(std::move(api))
    , node_p_(std::move(node))
    , api_(*api_p_)
    , account_(account)
    , node_(*node_p_)
    , db_(node_.Internal().DB())
    , mempool_(node_.Internal().Mempool())
    , chain_(node_.Internal().Chain())
    , filter_type_(node_.FilterOracle().DefaultType())
    , from_parent_(std::move(fromParent))
    , nym_([&] {
        auto out = api_.Wallet().Nym(account_.NymID());

        OT_ASSERT(out);

        return out;
    }())
    , local_(nym_->PaymentCodePublic())
    , path_([this] {
        auto out = decltype(path_){};
        const auto rc = nym_->Internal().PaymentCodePath(out);

        OT_ASSERT(rc);

        return out;
    }())
    , self_contact_(api_.Contacts().ContactID(account_.NymID()))
    , state_(State::normal)
    , reorgs_(alloc)
    , notification_(alloc)
    , internal_(alloc)
    , external_(alloc)
    , outgoing_(alloc)
    , incoming_(alloc)
    , reorg_(reorg.GetSlave(pipeline_, name_, alloc))
{
    OT_ASSERT(false == self_contact_.empty());
}

Account::Imp::Imp(
    Reorg& reorg,
    const crypto::Account& account,
    std::shared_ptr<const api::session::Client> api,
    std::shared_ptr<const node::Manager> node,
    std::string_view fromParent,
    network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Imp(reorg,
          account,
          std::move(api),
          std::move(node),
          CString{fromParent, alloc},
          std::move(batch),
          alloc)
{
}

auto Account::Imp::check(
    const crypto::Deterministic& subaccount,
    const crypto::Subchain subchain,
    SubchainsIDs& set) noexcept -> void
{
    const auto& log = log_;
    const auto [it, added] = set.emplace(subaccount.ID());

    if (added) {
        log("Instantiating ")(name_)(" subaccount ")(
            subaccount.ID(), api_.Crypto())(" ")(print(subchain))(
            " subchain for ")(subaccount.Parent().NymID(), api_.Crypto())
            .Flush();
        const auto& asio = api_.Network().ZeroMQ().Internal();
        const auto batchID = asio.PreallocateBatch();
        auto ptr = boost::allocate_shared<DeterministicStateData>(
            alloc::PMR<DeterministicStateData>{asio.Alloc(batchID)},
            reorg_,
            subaccount,
            api_p_,
            node_p_,
            subchain,
            from_parent_,
            batchID);

        OT_ASSERT(ptr);

        ptr->Init(ptr);
    }
}

auto Account::Imp::check_hd(const identifier::Account& id) noexcept -> void
{
    check_hd(account_.GetHD().at(id));
}

auto Account::Imp::check_hd(const crypto::HD& subaccount) noexcept -> void
{
    check(subaccount, crypto::Subchain::Internal, internal_);
    check(subaccount, crypto::Subchain::External, external_);
}

auto Account::Imp::check_notification(const identifier::Account& id) noexcept
    -> void
{
    check_notification(account_.GetNotification().at(id));
}

auto Account::Imp::check_notification(
    const crypto::Notification& subaccount) noexcept -> void
{
    const auto [it, added] = notification_.emplace(subaccount.ID());

    if (added) {
        const auto& code = subaccount.LocalPaymentCode();
        log_("Initializing payment code ")(code.asBase58())(" on ")(name_)
            .Flush();
        const auto& asio = api_.Network().ZeroMQ().Internal();
        const auto batchID = asio.PreallocateBatch();
        auto ptr = boost::allocate_shared<NotificationStateData>(
            alloc::PMR<NotificationStateData>{asio.Alloc(batchID)},
            reorg_,
            subaccount,
            code,
            api_p_,
            node_p_,
            crypto::Subchain::NotificationV3,
            from_parent_,
            batchID);

        OT_ASSERT(ptr);

        ptr->Init(ptr);
    }
}

auto Account::Imp::check_pc(const identifier::Account& id) noexcept -> void
{
    check_pc(account_.GetPaymentCode().at(id));
}

auto Account::Imp::check_pc(const crypto::PaymentCode& subaccount) noexcept
    -> void
{
    check(subaccount, crypto::Subchain::Outgoing, outgoing_);
    check(subaccount, crypto::Subchain::Incoming, incoming_);
}

auto Account::Imp::do_reorg(
    const node::HeaderOracle& oracle,
    const node::internal::HeaderOraclePrivate& data,
    Reorg::Params& params) noexcept -> bool
{
    // NOTE no action necessary

    return true;
}

auto Account::Imp::do_shutdown() noexcept -> void
{
    state_ = State::shutdown;
    reorg_.Stop();
    node_p_.reset();
    api_p_.reset();
}

auto Account::Imp::do_startup(allocator_type monotonic) noexcept -> bool
{
    if (reorg_.Start()) { return true; }

    api_.Wallet().Internal().PublishNym(account_.NymID());
    scan_subchains();
    index_nym(account_.NymID());
    scan_contacts(monotonic);

    return false;
}

auto Account::Imp::index_nym(const identifier::Nym& id) noexcept -> void
{
    for (const auto& subaccount : account_.GetNotification()) {
        check_notification(subaccount);
    }
}

auto Account::Imp::Init(boost::shared_ptr<Imp> me) noexcept -> void
{
    signal_startup(me);
}

auto Account::Imp::pipeline(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    switch (state_) {
        case State::normal: {
            state_normal(work, std::move(msg), monotonic);
        } break;
        case State::reorg: {
            state_reorg(work, std::move(msg));
        } break;
        case State::pre_shutdown: {
            state_pre_shutdown(work, std::move(msg));
        } break;
        case State::shutdown: {
            // NOTE do nothing
        } break;
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid state").Abort();
        }
    }
}

auto Account::Imp::process_contact(
    Message&& in,
    allocator_type monotonic) noexcept -> void
{
    const auto body = in.Payload();

    OT_ASSERT(1 < body.size());

    process_contact(
        api_.Factory().IdentifierFromProtobuf(body[1].Bytes()), monotonic);
}

auto Account::Imp::process_contact(
    const identifier::Generic& id,
    allocator_type monotonic) noexcept -> void
{
    const auto contact = api_.Contacts().Contact(id);

    if (contact) { process_contact(*contact, monotonic); }
}

auto Account::Imp::process_contact(
    const opentxs::Contact& contact,
    allocator_type monotonic) noexcept -> void
{
    if (contact.ID() == self_contact_) { return; }

    auto codes = Set<opentxs::PaymentCode>{monotonic};
    codes.clear();
    const auto nyms = contact.Nyms();
    const auto published = contact.PaymentCodes(BlockchainToUnit(chain_));
    const auto parse_nym = [&, this](const auto& id) {
        if (const auto nym = api_.Wallet().Nym(id); nym) {
            codes.emplace(nym->PaymentCodePublic());
        }
    };
    const auto parse_base58 = [&, this](const auto& base58) {
        if (auto code = api_.Factory().PaymentCodeFromBase58(base58); code) {
            codes.emplace(std::move(code));
        }
    };
    const auto reason =
        api_.Factory().PasswordPrompt("Generate payment code channel keys");
    const auto check_account = [&, this](const auto& remote) {
        api_.Crypto().Blockchain().Internal().PaymentCodeSubaccount(
            nym_->ID(), local_, remote, path_, chain_, reason);
    };
    std::for_each(nyms.begin(), nyms.end(), parse_nym);
    std::for_each(published.begin(), published.end(), parse_base58);
    std::for_each(codes.begin(), codes.end(), check_account);
}

auto Account::Imp::process_key(Message&& in) noexcept -> void
{
    const auto body = in.Payload();

    OT_ASSERT(5u < body.size());

    const auto chain = body[1].as<blockchain::Type>();

    if (chain != chain_) { return; }

    const auto owner = api_.Factory().NymIDFromHash(body[2].Bytes());

    if (owner != account_.NymID()) { return; }

    const auto id = api_.Factory().AccountIDFromHash(
        body[3].Bytes(), identifier::AccountSubtype::blockchain_subaccount);
    const auto type = body[5].as<crypto::SubaccountType>();
    process_subaccount(id, type);
}

auto Account::Imp::process_prepare_reorg(Message&& in) noexcept -> void
{
    const auto body = in.Payload();

    OT_ASSERT(1u < body.size());

    transition_state_reorg(body[1].as<StateSequence>());
}

auto Account::Imp::process_rescan(Message&& in) noexcept -> void
{
    // NOTE no action necessary
}

auto Account::Imp::process_subaccount(Message&& in) noexcept -> void
{
    const auto body = in.Payload();

    OT_ASSERT(4 < body.size());

    const auto chain = body[1].as<blockchain::Type>();

    if (chain != chain_) { return; }

    const auto owner = api_.Factory().NymIDFromHash(body[2].Bytes());

    if (owner != account_.NymID()) { return; }

    const auto type = body[3].as<crypto::SubaccountType>();
    const auto id = api_.Factory().AccountIDFromZMQ(body[4]);
    process_subaccount(id, type);
}

auto Account::Imp::process_subaccount(
    const identifier::Account& id,
    const crypto::SubaccountType type) noexcept -> void
{
    switch (type) {
        case crypto::SubaccountType::HD: {
            check_hd(id);
        } break;
        case crypto::SubaccountType::PaymentCode: {
            check_pc(id);
        } break;
        case crypto::SubaccountType::Error:
        case crypto::SubaccountType::Imported:
        case crypto::SubaccountType::Notification:
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid subaccount type")
                .Abort();
        }
    }
}

auto Account::Imp::scan_subchains() noexcept -> void
{
    for (const auto& subaccount : account_.GetHD()) { check_hd(subaccount); }

    for (const auto& subaccount : account_.GetPaymentCode()) {
        check_pc(subaccount);
    }
}

auto Account::Imp::scan_contacts(allocator_type monotonic) noexcept -> void
{
    const auto contacts = api_.Storage().ContactList();
    const auto scan = [this, monotonic](const auto& item) {
        const auto id =
            api_.Factory().IdentifierFromBase58(item.first, monotonic);
        process_contact(id, monotonic);
    };
    std::for_each(contacts.begin(), contacts.end(), scan);
}

auto Account::Imp::state_normal(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    switch (work) {
        case Work::contact: {
            process_contact(std::move(msg), monotonic);
        } break;
        case Work::subaccount: {
            process_subaccount(std::move(msg));
        } break;
        case Work::prepare_reorg: {
            process_prepare_reorg(std::move(msg));
        } break;
        case Work::rescan: {
            process_rescan(std::move(msg));
        } break;
        case Work::key: {
            process_key(std::move(msg));
        } break;
        case Work::prepare_shutdown: {
            transition_state_pre_shutdown();
        } break;
        case Work::finish_reorg: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" wrong state for ")(
                print(work))(" message")
                .Abort();
        }
        case Work::shutdown:
        case Work::init:
        case Work::statemachine: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                print(work))
                .Abort();
        }
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto Account::Imp::state_pre_shutdown(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::contact:
        case Work::subaccount:
        case Work::rescan:
        case Work::key: {
            // NOTE ignore message
        } break;
        case Work::prepare_reorg:
        case Work::finish_reorg:
        case Work::prepare_shutdown: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" wrong state for ")(
                print(work))(" message")
                .Abort();
        }
        case Work::shutdown:
        case Work::init:
        case Work::statemachine: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                print(work))
                .Abort();
        }
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto Account::Imp::state_reorg(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::contact:
        case Work::subaccount:
        case Work::prepare_reorg:
        case Work::rescan:
        case Work::key: {
            defer(std::move(msg));
        } break;
        case Work::finish_reorg: {
            transition_state_normal();
        } break;
        case Work::prepare_shutdown: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" wrong state for ")(
                print(work))(" message")
                .Abort();
        }
        case Work::shutdown:
        case Work::init:
        case Work::statemachine: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                print(work))
                .Abort();
        }
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto Account::Imp::transition_state_normal() noexcept -> void
{
    state_ = State::normal;
    log_(OT_PRETTY_CLASS())(name_)(" transitioned to normal state ").Flush();
    trigger();
}

auto Account::Imp::transition_state_pre_shutdown() noexcept -> void
{
    reorg_.AcknowledgeShutdown();
    state_ = State::pre_shutdown;
    log_(OT_PRETTY_CLASS())(name_)(": transitioned to pre_shutdown state")
        .Flush();
}

auto Account::Imp::transition_state_reorg(StateSequence id) noexcept -> void
{
    OT_ASSERT(0_uz < id);

    if (0_uz == reorgs_.count(id)) {
        reorgs_.emplace(id);
        state_ = State::reorg;
        log_(OT_PRETTY_CLASS())(name_)(" ready to process reorg ")(id).Flush();
        reorg_.AcknowledgePrepareReorg(
            [this](const auto& header, const auto& lock, auto& params) {
                return do_reorg(header, lock, params);
            });
    } else {
        LogAbort()(OT_PRETTY_CLASS())(name_)(" reorg ")(id)(" already handled")
            .Abort();
    }
}

auto Account::Imp::work(allocator_type monotonic) noexcept -> bool
{
    return false;
}

Account::Imp::~Imp() = default;
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Account::Account(
    Reorg& reorg,
    const crypto::Account& account,
    std::shared_ptr<const api::session::Client> api,
    std::shared_ptr<const node::Manager> node,
    std::string_view fromParent) noexcept
    : imp_([&] {
        OT_ASSERT(api);
        OT_ASSERT(node);

        const auto& asio = api->Network().ZeroMQ().Internal();
        const auto batchID = asio.PreallocateBatch();
        // TODO the version of libc++ present in android ndk 23.0.7599858
        // has a broken std::allocate_shared function so we're using
        // boost::shared_ptr instead of std::shared_ptr

        return boost::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)},
            reorg,
            account,
            std::move(api),
            std::move(node),
            fromParent,
            batchID);
    }())
{
}

auto Account::Init() noexcept -> void
{
    OT_ASSERT(imp_);

    imp_->Init(imp_);
    imp_.reset();
}

Account::~Account() = default;
}  // namespace opentxs::blockchain::node::wallet
