/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_UI_WIDGET_IMPLEMENTATION_HPP
#define OPENTXS_UI_WIDGET_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/network/zeromq/RequestSocket.hpp"
#include "opentxs/ui/Widget.hpp"

namespace opentxs::ui::implementation
{
class Widget : virtual public opentxs::ui::Widget
{
public:
    OTIdentifier WidgetID() const override;

    virtual ~Widget() = default;

protected:
    const network::zeromq::Context& zmq_;

    void UpdateNotify() const;

    Widget(const network::zeromq::Context& zmq, const Identifier& id);
    Widget(const network::zeromq::Context& zmq);

private:
    const Identifier& widget_id_;
    const OTZMQRequestSocket update_socket_;

    Widget() = delete;
    Widget(const Widget&) = delete;
    Widget(Widget&&) = delete;
    Widget& operator=(const Widget&) = delete;
    Widget& operator=(Widget&&) = delete;
};
}  // opentxs::ui::implementation
#endif  // OPENTXS_UI_WIDGET_IMPLEMENTATION_HPP
