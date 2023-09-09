// Copyright (c) 2023 MatterFi, Inc. - All Rights Reserved
// You may use, distribute, and modify this code under the terms of the
// MatterFi Semi-Open License accompanying this file.

#include "matterfi/PaymentCode.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <random>

#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/crypto/Account.hpp"
#include "internal/blockchain/crypto/Types.hpp"
#include "internal/blockchain/crypto/Wallet.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

// This functions in this file implement operations covered by one or more
// claims of US Provisional Patent Application 63/507,085 and 63/512,052
// assigned to MatterFi, Inc.
namespace matterfi
{
using namespace opentxs::literals;

auto paymentcode_extra_notifications(
    const opentxs::Log& log,
    const opentxs::blockchain::crypto::PaymentCode& account,
    boost::container::flat_set<opentxs::PaymentCode>& out) noexcept -> void
{
    if (0_uz == account.IncomingNotificationCount()) {
        log(__func__)(": adding extra notification to self").Flush();
        out.emplace(account.Local());
    } else {
        log(__func__)(": contact has already sent notification").Flush();
    }
}

auto paymentcode_preemptive_notifications(
    const opentxs::Log& log,
    const opentxs::api::Session& api,
    const opentxs::identifier::Nym& sender,
    opentxs::blockchain::Type chain,
    boost::container::flat_set<opentxs::PaymentCode>& out,
    opentxs::alloc::Strategy alloc) noexcept -> void
{
    const auto notif =
        api.Crypto().Blockchain().Internal().GetNotificationStatus(
            sender, alloc);
    auto all = opentxs::Set<opentxs::PaymentCode>{alloc.work_};
    auto notifiedThisChain = opentxs::Set<opentxs::PaymentCode>{alloc.work_};
    auto notifiedOtherChains = opentxs::Set<opentxs::PaymentCode>{alloc.work_};
    const auto process = [&](const auto& item) {
        static constexpr auto copy = [](const auto& src, auto& dst) {
            std::copy(src.begin(), src.end(), std::inserter(dst, dst.end()));
        };
        const auto& [c, n] = item;
        copy(n.incoming_, all);
        copy(n.neither_, all);

        if (c == chain) {
            copy(n.outgoing_, notifiedThisChain);
        } else {
            copy(n.outgoing_, notifiedOtherChains);
        }
    };
    std::for_each(notif.begin(), notif.end(), process);
    const auto temp = [&] {
        auto result = opentxs::Vector<opentxs::PaymentCode>{alloc.work_};
        result.reserve(all.size());
        result.clear();
        std::set_difference(
            all.begin(),
            all.end(),
            notifiedThisChain.begin(),
            notifiedThisChain.end(),
            std::back_inserter(result));

        return result;
    }();
    auto unnotifiedAnyChain = [&] {
        auto result = opentxs::Vector<opentxs::PaymentCode>{alloc.work_};
        result.reserve(all.size());
        result.clear();
        std::set_difference(
            temp.begin(),
            temp.end(),
            notifiedOtherChains.begin(),
            notifiedOtherChains.end(),
            std::back_inserter(result));

        return result;
    }();
    auto unnotifiedThisChain = [&] {
        auto result = opentxs::Vector<opentxs::PaymentCode>{alloc.work_};
        result.reserve(all.size());
        result.clear();
        std::set_intersection(
            temp.begin(),
            temp.end(),
            notifiedOtherChains.begin(),
            notifiedOtherChains.end(),
            std::back_inserter(result));

        return result;
    }();
    log(__func__)(": out of ")(all.size())(" known payment codes, ")(
        unnotifiedAnyChain.size())(
        " have never received a notification on any chain and ")(
        unnotifiedThisChain.size())(
        " have been notified on other chains but not on ")(print(chain))
        .Flush();
    auto rand = std::random_device{};
    std::shuffle(
        unnotifiedAnyChain.begin(),
        unnotifiedAnyChain.end(),
        std::mt19937{rand()});
    std::shuffle(
        unnotifiedThisChain.begin(),
        unnotifiedThisChain.end(),
        std::mt19937{rand()});
    auto queue = [&] {
        auto result = opentxs::Deque<opentxs::PaymentCode>{alloc.work_};
        result.clear();
        std::move(
            unnotifiedAnyChain.begin(),
            unnotifiedAnyChain.end(),
            std::back_inserter(result));
        std::move(
            unnotifiedThisChain.begin(),
            unnotifiedThisChain.end(),
            std::back_inserter(result));

        return result;
    }();
    const auto limit =
        opentxs::blockchain::params::get(chain).MaxNotifications();
    log(__func__)(": notification list contains ")(out.size())(
        " payment codes out of a maximum of ")(limit)(" for ")(print(chain))
        .Flush();

    while ((out.size() < limit) && (false == queue.empty())) {
        auto& pc = queue.front();
        log(__func__)(": adding ")(pc)(" to notification list").Flush();
        out.emplace(std::move(pc));
        queue.pop_front();
    }
}
}  // namespace matterfi
