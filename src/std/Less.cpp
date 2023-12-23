// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <compare>
#include <memory>

#include "internal/interface/ui/UI.hpp"
#include "internal/otx/client/Client.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "internal/util/Pimpl.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/contract/peer/Reply.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/crypto/Seed.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"

namespace std
{
auto less<opentxs::blockchain::block::Position>::operator()(
    const opentxs::blockchain::block::Position& lhs,
    const opentxs::blockchain::block::Position& rhs) const noexcept -> bool
{
    return lhs < rhs;
}

auto less<opentxs::contract::peer::Reply>::operator()(
    const opentxs::contract::peer::Reply& lhs,
    const opentxs::contract::peer::Reply& rhs) const noexcept -> bool
{
    return lhs.ID() < rhs.ID();
}

auto less<opentxs::contract::peer::Request>::operator()(
    const opentxs::contract::peer::Request& lhs,
    const opentxs::contract::peer::Request& rhs) const noexcept -> bool
{
    return lhs.ID() < rhs.ID();
}

auto less<opentxs::crypto::Seed>::operator()(
    const opentxs::crypto::Seed& lhs,
    const opentxs::crypto::Seed& rhs) const noexcept -> bool
{
    return lhs < rhs;
}

auto less<opentxs::otx::client::MessageTask>::operator()(
    const opentxs::otx::client::MessageTask& lhs,
    const opentxs::otx::client::MessageTask& rhs) const noexcept -> bool
{
    const auto& [lID, lMessage, lFunction] = lhs;
    const auto& [rID, rMessage, rFunction] = rhs;

    if (lID < rID) { return true; }

    if (rID < lID) { return false; }

    if (lMessage < rMessage) { return true; }

    if (rMessage < lMessage) { return false; }

    if (&lFunction < &rFunction) { return true; }

    return false;
}

auto less<opentxs::otx::client::PaymentTask>::operator()(
    const opentxs::otx::client::PaymentTask& lhs,
    const opentxs::otx::client::PaymentTask& rhs) const noexcept -> bool
{
    const auto& [lID, lPayment] = lhs;
    const auto& [rID, rPayment] = rhs;

    if (lID < rID) { return true; }

    if (rID < lID) { return false; }

    auto lPaymentID = opentxs::identifier::Generic{};
    auto rPaymentID = opentxs::identifier::Generic{};

    lPayment->GetIdentifier(lPaymentID);
    rPayment->GetIdentifier(rPaymentID);

    if (lPaymentID < rPaymentID) { return true; }

    return false;
}

auto less<opentxs::otx::client::PeerReplyTask>::operator()(
    const opentxs::otx::client::PeerReplyTask& lhs,
    const opentxs::otx::client::PeerReplyTask& rhs) const noexcept -> bool
{
    const auto& [lNym, lReply, lRequest] = lhs;
    const auto& [rNym, rReply, rRequest] = rhs;

    if (lNym < rNym) { return true; }

    if (rNym < lNym) { return false; }

    if (lReply.ID() < rReply.ID()) { return true; }

    if (rReply.ID() < lReply.ID()) { return false; }

    if (lRequest.ID() < rRequest.ID()) { return true; }

    return false;
}

auto less<opentxs::otx::client::PeerRequestTask>::operator()(
    const opentxs::otx::client::PeerRequestTask& lhs,
    const opentxs::otx::client::PeerRequestTask& rhs) const noexcept -> bool
{
    const auto& [lID, lRequest] = lhs;
    const auto& [rID, rRequest] = rhs;

    if (lID < rID) { return true; }

    if (rID < lID) { return false; }

    if (lRequest.ID() < rRequest.ID()) { return true; }

    return false;
}

auto less<opentxs::ui::implementation::ContactActivityRowID>::operator()(
    const opentxs::ui::implementation::ContactActivityRowID& lhs,
    const opentxs::ui::implementation::ContactActivityRowID& rhs) const -> bool
{
    const auto& [lID, lBox, lAccount] = lhs;
    const auto& [rID, rBox, rAccount] = rhs;

    if (lID < rID) { return true; }

    if (rID < lID) { return false; }

    if (lBox < rBox) { return true; }

    if (rBox < lBox) { return false; }

    if (lAccount < rAccount) { return true; }

    return false;
}

auto less<opentxs::ui::implementation::BlockchainSelectionSortKey>::operator()(
    const opentxs::ui::implementation::BlockchainSelectionSortKey& lhs,
    const opentxs::ui::implementation::BlockchainSelectionSortKey& rhs) const
    -> bool
{
    const auto& [lName, lTestnet] = lhs;
    const auto& [rName, rTestnet] = rhs;

    if ((!lTestnet) && (rTestnet)) { return true; }

    if (lTestnet && (!rTestnet)) { return false; }

    if (lName < rName) { return true; }

    return false;
}

auto less<opentxs::ui::implementation::ContactListSortKey>::operator()(
    const opentxs::ui::implementation::ContactListSortKey& lhs,
    const opentxs::ui::implementation::ContactListSortKey& rhs) const -> bool
{
    const auto& [lSelf, lText] = lhs;
    const auto& [rSelf, rText] = rhs;

    if (lSelf && (!rSelf)) { return true; }

    if (rSelf && (!lSelf)) { return false; }

    if (lText < rText) { return true; }

    return false;
}

auto less<opentxs::OT_DownloadNymboxType>::operator()(
    const opentxs::OT_DownloadNymboxType& lhs,
    const opentxs::OT_DownloadNymboxType& rhs) const noexcept -> bool
{
    return lhs < rhs;
}

auto less<opentxs::OT_GetTransactionNumbersType>::operator()(
    const opentxs::OT_GetTransactionNumbersType& lhs,
    const opentxs::OT_GetTransactionNumbersType& rhs) const noexcept -> bool
{
    return lhs < rhs;
}
}  // namespace std
