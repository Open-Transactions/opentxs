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

// OTSmartContract is derived from OTCronItem.
//
// WHAT DOES IT DO?
//
// 1) The clauses and bylaws can be written in script language by
//    the users, without having to change OT itself. SCRIPTABLE CLAUSES!
//    Invent your own financial instruments and processes, involving
//    multiple agents and asset accounts.
// 2) Any number of parties can sign these contracts.
// 3) Each Party has a name, and can be referred to using that name
//    from within the script code. Parties can also bring accounts and
//    other assets into these agreements, and scripts can manipulate them.
// 4) A party to an agreement can be an individual nym, OR it can be a
//    fictional ENTITY formed by some prior agreement.
// 5) A party may have many agents. Since some parties are fictional
//    entities (e.g. a corporation, a democracy, an estate for a deceased
//    person, etc), parties are able to appoint agents to act on their
//    behalf. An individual Nym is really just an individual Party who
//    acts as his own agent, whereas a corporation is owned by a voting group,
//    and appoints a Nym to act as its agent. Voting groups will soon also be
//    able to act as agents IN SOME RESPECTS. (Voting groups are coming
//    next, after smart contracts.) These will be able to edit bylaws, etc.
// 6) A Smart Contract has a list of parties, each with a list of agents.
//    Parties also bring asset accounts to the agreement, for use in the
// scripts.
// 7) A Smart Contract also has a list of bylaws, each with a list of clauses.
//    Each set of Bylaws also has its own variables, which can be used by the
//    scripts.
// 8) A Smart Contract can be activated (provided all parties have properly
//    signed), and it can process repeatedly over time until it expires or gets
//    deactivated. While it's active, parties can trigger specific clauses and
//    the smart contract will also occasionally trigger AUTOMATICALLY (depending
//    on how it has been configured.)
// 9) Users can decide which functions will activate--and when--and which
//    powers will be vested in the various parties and agents.
// 10) HOOKS -- Scripts trigger upon various EVENTS such as onActivate,
//    onDeactivate, onTrigger, etc. Perhaps a timer is set causing the custom
//    function "TransferEmergencyFunds" to trigger in 24 hours, or 30 days,
//    or when the price of gold reaches X... or however you code the scripted
//    clauses in your contract...
//
// The design makes it easy to swap in different script languages. (Currently
// for experimental purposes I am using chaiscript.)
//

/*
 RUNNING LIST of standard hooks, functions, constants, variables, etc..

 - I'm thinking that Constants and Variables should be available in OTBylaw AND
OTScriptable.
 - Ah maybe just have them in Bylaws only, and then have a "library" Bylaw that
is "global" to the OTScriptable.

 - OT Internal Functions will be available on server side inside scripts, just
like OT API is
   on client side. They're written in C++.
 - Callbacks are the opposite: they're written in script, and available for the
C++ to call when it needs to.
 - Script Functions can be called by all parties, unless CanTriggerClause(name,
party) returns false.
 - Hooks should be stored as a list on the Bylaws. If hook "OnActivate" is
triggered, then I ask each Bylaw to
   run any relevant scripts for that hook.


 VARIABLES  (These are changed values you can store inside your smart contract,
which stay persistent between runs.)

 -- The first one of these will probably be a timer variable, used with
OnProcess() to trigger some
    timed clause.  You'll set it to 100, then decrement every call, then trigger
when it hits zero.
    (Or trigger every time X person withdraws money, etc.)

 -- Let's say a contract processes 100 times but only does something important
the last time. Do I REALLY
    need a receipt for every fucking 100 times just because some timer variable
changed? Therefore, probably
    want to strictly define two types of variables:  Those that require a
receipt when they change, and those
    that do not.  Therefore:

 TYPES:   Constant (cannot change), Persistent (stores over time; change doesn't
require noticing the parties),
          Important (Stores over time; change requires notice to the parties).

 Any notice should be accompanied by a notice # so parties can see if they
missed a notice.

 ------------------------------------------------------------------------


 CONSTANTS   (These are standard values available inside any script.)

 -- ProcessInterval. (The contract will activate every X seconds. Default 30.)

 NOTE:  I probably won't let users change this, but rather, will let them
 use a multiple of it.  Or maybe I'll let them have fine-tuned timing, but just
 charge them for it via server fees.

 ------------------------------------------------------------------------

 CALLBACKS   (OT will call these scripts when it needs an answer to something.)

 -- CanDeactivate(Party). Script returns true/false whether Party is allowed to
deactivate the contract.
    Anyone party to the contract can cancel it, by default. (You can change that
with this callback.)

    can_execute_clause(party_name, clause_name)

 -- CanTriggerClause(Party, Clause).  Script returns whether Party is allowed to
call this clause. Default true.

 ------------------------------------------------------------------------

 OT INTERNAL FUNCTIONS   (Calls into OT that you can make from inside your
scripts.)

 -- These are like the OT API calls, except on the server side. Examples?

 -- ExecuteClause(Bylaw, Clause)
 -- ExecuteClause(Clause)

    These allow you to, from inside a clause, execute any other clause on the
same contract.

 -- FlagForRemoval()    This removes the script from Cron. Deactivates it.

 -- Imagine OTParty->SendMessage(OtherParty). CANNOT DO THIS. Why not? Because
on the client side, the sender
    can actually sign the thing before sending it. But on the server side, the
server cannot sign with someone's
    nym since it doesn't have that private key. It CAN verify contracts, and
transfer money, check balances, etc.

 -- Definitely a call that transfers money from one Party's account to another,
dropping a receipt.

 -- Calls that withdraw money and return a financial instrument. For example:
"Take 100 clams out of Jim's acct,
    issue a server cheque for 100 clams, and send it to George."  You might ask,
why not just have the cheque come
    from Jim directly?  Answer: Because the server can't forge Jim's signature.
Instead, the server justifies the
    action to Jim by providing a copy of the script contract in his inbox
receipt when it takes the money. It signs
    its own cheque, which is something it actually can do, and it sends it to
George. A copy is sent to all parties
    so they can prove later on whether the voucher was redeemed. But they can't
redeem it themselves, since it is
    made out to George.

 -- Interesting: Certain instruments REQUIRE the client side! The server can't
withdraw cash on your behalf since
    it can't generate the prototokens, nor can it unblind them. The server can't
write a cheque on your behalf since
    it can't forge your signature.  It CAN withdraw cash on its own, and send to
whoever you want, but then you have
    to trust the server not trace that cash (it's traceable in that case.) It
CAN write a cheque on your behalf, but
    of course you have to trust the server that the money will be there when
that cheque is cashed.  It CANNOT create
    a market order on your behalf! You have to sign that when you do it.  It
CANNOT activate some new smart contract
    on your behalf, since it can't forge your signature. It CANNOT initiate a
transfer (since you must sign.) Instead
    it just moves the funds and leaves you a copy of the script as your receipt.
(You DID sign the script, so it IS
    a good enough receipt for that.)  I suppose that it CAN exchange you in/out
of a basket, although normally you'd
    expect a receipt showing your request. I guess future versions of OT can be
smart enough to interpret a basket
    receipt in different ways, depending on whether the InRefTo contains an
exchange request or a smart contract.

 -- A call that ADDS a script to a hook. Imagine there 100 different hooks: you
don't actually have scripts for
    all of them!!! But perhaps you have a certain script, that gets ATTACHED to
a hook at some point, based on
    script logic, and then THEREAFTER activates on that hook when it previously
didn't. Therefore even OnActivate()
    and OnDeactivate() should have their own actual script names, and then those
scripts are attached to the hooks
    known as OnActivate and OnDeactivate().  The portion of the smart contract
that attaches them changes over time,
    as this call is made, adding them and removing them.
    It could be that ONE script is registered to a hook, and then some event
causes it to be de-registered
    and then another one is registered to take its place!

 -- Functions to send notices to various parties and inboxes. (The server can't
forge a party's message to
    another, but it CAN send them all a message from the server itself.)
Therefore NOTICES.

 -- Anytime funds are moved between accounts, they should get a scriptReceipt
aka paymentReceipt type deal.

 ------------------------------------------------------------------------

 SCRIPT FUNCTIONS   (Scripts you add to your smart contract, which can be
triggered by parties.)

 -- First one will probably be "DisputeEscrow()" for the escrow contract.

 -- Might be nice to have clauses that are available for others to call.
    Perhaps a "library" Bylaw that other bylaws have access to.

 ------------------------------------------------------------------------

 HOOKS       (Scripts you add to your smart contract, which will trigger
automatically upon specific events.)

 -- OnActivate()        This happens when script is first activated.
 -- OnDeactivate()        This happens when script is deactivated.
 -- OnProcess()            This happens every ProcessInterval.
 -- OnExpire()            Happens when the script is deactivated due to reaching
end of valid date/time range.



 OTHER HOOKS?


 Bylaws need to have a list of hooks, where the hook name corresponds to the
clause name.
 Just as they have a list of clauses, they also need a list of the hooks those
clauses are triggered by.






 Still need todo on smart contracts:

 -- Serialization
 -- Confirmation
 -- Verification
 -- Native Functions such as:

    move_funds(from_acct, to_acct)                (from_acct and to_acct must be
a party to the agreement)
    send_cashiers_cheque(from_acct, to_nym)        (from_acct must be a party to
the agreement. to_nym doesn't have to be.)
    stash_funds(from_acct, "stash_one", 100)    ("stash_one" is stored inside
the bylaw. Server-side only.)
    unstash_funds("stash_one", to_acct, 100)    (Smartcontract must be activated
with no stashes. Server creates/maintains them.)
    unstash_funds_to_nym("stash_one", to_nym, 100)    (This is like
send_cashiers_cheque, except from a stash.)
    get_balance(acct)                            (acct must be party to
agreement.)

    send_notice(to_nym)                            (Like sendMessage, except it
comes from the server, not another user.)
    send_notice_to_parties()                    (Does a send_notice to ALL
parties.)

    can_execute_clause(party_name, clause_name) (See if a party is allowed to
execute any given clause.)


 -- Dirtiness:
    Important variables changed require a Nymbox notice sent. (agreementReceipt)
    Any movement of funds requires Inbox notice sent to relevant parties. (can I
just use paymentReceipt? Otherwise agreementReceipt here too.)
    FinalReceipt requires Nymbox AND Inbox notices to be sent.  (finalReceipt.)

 -- Client command to message server and activate a new SmartContract.
 -- Server functions to process said message, and activate the script on cron.

 -- (Postponed) Make a new OTScriptable-derived class called OTTitle (which
represents a registered piece of property)
    and add client commands / server functions for functionality like
SmartContract, except you are registering an
    OTTitle for a static piece of property, versus activating an OTCronItem that
processes and triggers repeatedly
    over time. OTTitle will probably derive from Contract, in the same way
that OTUnitDefinition derives from
    Contract. You will be able to register title on a server similar to
registering an asset contract. Except,
    instead of getting an issuer acct, you get a deed...

 -- (Postponed) There are several ways to own title to property. One is to have
an OTDeed class, which allows any specific Nym to
    have a title registered to him based on a specific transaction #, which is
released whenever the deed is transferred.
    This enables destruction-of-acct-history similar to unit-based accounts, but
it also necessitates the use of an "asset
    account" (with an inbox) specifically for holding deed-based property. (This
is necessary in order to prove ownership
    and everything else--the same as with unit-based accounts.)  I probably will
NOT code this "deed" piece anytime soon...

 -- (Upcoming) ...HOWEVER, ANOTHER WAY to own title (WITHOUT A DEED) is through
SHAREHOLDING. That is where my interest currently
    lies. Therefore I will add the OTEntity class. Instead of being derived from
Contract like OTTitle, it will be derived from
    OTScriptable (similar to the smart contracts.)  This will enable it to have
parties and bylaws.
    Similar to cron items, which save the original copy but also save updated
copies whenever something changes, I will cause
    OTEntity to generate a unique ID based on the entity as it originally
appeared when first registered. The original version
    is always retrievable by a shareholder and hashing it should produce the
actual entity ID.
    But from there, the Entity can evolve over time. Since it is
OTScriptable-derived, it can have parties, and bylaws. The
    parties can take actions to change the bylaws and appointments.
    The political structure of the entity is determined by the parties. There
might be 3 parties who are all Nyms, in which case
    you have a partnership. If the entity's charter says, "Party A is a Nym:
FellowTraveler", and if all parties by default have
    the power to alter the bylaws, appoint employees, activate clauses to move
funds, etc, then you now have an individual
    controlling the entity.
    WHERE THIS IS GOING: A party whose agent is a voting group, whose members
are determined by shareholdership in STOCK of
    the entity. (Really could be configured to stock in any stock ID, but people
will WANT the stock to be in the entity.) Future
    versions of OT will allow creation of multiple classes of stock, and indeed
there is nothing stopping entities now from
    issuing whatever currencies they want, with the issuer accounts controlled
by them or their agents. Of course it's better
    to have it strict in the bylaws, and have OT just execute it that way, and
for now that's how it will be with the DEFAULT,
    HARDCODED current path of a SINGLE class of stock for each entity, with the
Entity's ID serving as the Instrument Definition ID. I think
    it would be a LONG TIME before we ever need more functionality than that
anyway, so that's what I'll focus on.

 -- So I need OTVotingGroup class which will determine members based on various
types of groups.
    1) Shareholders. Expects a weighted vote among shareholders based on asset
type ID matching entity ID.
    2) Appointed members. A list of members (NymIDs) is explicitly stored in
this group. (Like the list of board members.)

    (1) Will take advantage of existing asset account code (for weighted
voting),
    and (2) will take advantage of existing digital cash code (for secret
ballots.)

    For both types of groups, votes will be possible such as Majority rules,
2/3rds, 3/4ths, 9/10ths, Unanimous, etc.
    In the case of appointed members (2), permissions can be granted to members,
such as dictator, dictator subject to veto,
    veto power, etc etc. In this way, democracies, republics, corporations, etc
all become possible through simple config changes.

    But all of the actual voting will not happen until I code classes like
OTBallot, OTElection, etc. That's not my current
    focus. We will get to that in time. Rather, my focus is on the use of
SHAREHOLDERS to allow weighted, account-based ownership
    of STOCK in ENTITIES.  My interest right now is on the ENTITIES THEMSELVES,
and their ability to be party to agreements, and
    to hire employees, and to own property and asset accounts.

    So an EntityID is similar to a TitleID, in the sense that an entity is a
piece of property that can be owned. But an EntityID
    is also similar to an InstrumentDefinitionID, since there can be a currency
issued for
that entity. And an entity is also similar to a smart
    contract, in the sense that it can have bylaws, and it can have "parties to
the agreement".

    Beyond that, entities will also have the ability to hire employees, sign
contracts, own property, etc. Distribute funds up and
    down the hierarchy, and have access the communications and receipts up and
down the hierarchy.

 -- Entity will not be derived from a Title, because they are fundamentally
different enough. Entities are owned by the voting groups
    that make them up, according to their laws. Whereas Titles are owned based
on holder of Deed.

 -- Therefore, NOT needed for now: OTTitle, OTDeed, OTBallot, OTElection.

 -- NEEDED and coming soon: OTVotingGroup, OTEntity.  I'll start on these once I
have proven the first SMART CONTRACT (Escrow!!!!!)


===> NEW THOUGHT:  I think OTUnitDefinition should be derived from OTScriptable
instead of Contract. There's no reason why an
     issuer shouldn't be able to attach scripts to certain aspects of a
currency, right? Hooks could trigger at various currency-related
     events.  There's also no reason why an issued currency shouldn't be able to
have parties. But who are the parties, if not the issuer?
     In the case of OTUnitDefinition, the parties will be whoever are configured
to do whatever the clauses need them to do, which will often
     be nothing, though technically they could be used for AUTOMATED BAILMENT
PROCESSES!! (Say one of the parties is a voting group consisting
     of anyone who holds stock. And that party has the right to trigger the
"bailout" clause, which has the power the transfer the funds auto-
     matically (via a script) to another party or account.)  I don't know how
people might use this, but it seems potentially useful.

 ===> Once this is in place, then it's easy to have OTEntity be derived from
OTUnitDefinition!!  And that's where we'll get our Entity ID.

 ===> ANOTHER IDEA:  SHould be easy to move funds ONTO THE SMART CONTRACT
ITSELF. Just have an XML tag where stored funds go, perhaps with the
      funds double in a server backing account (similar to how the cash already
is.)  The script can enable peeps to move funds from an acct
      and to a safe place (inside the smart contract itself) until some other
rule causes it to be moved somewhere else. This is needed for ESCROW!!!
      Perhaps I'll call this a "fund".  OTFund.  The only other way for an
entity to control funds is to open accounts, meaning it must trust the
      Nym who's been appointed to the role of managing that account. The entity
ITSELF can't open accounts because there is no signer!!! But the
      entity CAN control FUNDS, because they are stored inside the entity
itself!! (relevant parties get receipts for changes to entities...) And
      unlike a Nym, everyone can trust a cold, hard, script.


 ===> REALIZATION:  For my audit protocol, I wanted to have some kind of DHT to
store the receipts, where issuer and customer both have access to
      them, but where the system itself cannot otherwise be shut down. And you
can't just put some IP address of a server into the contract (as the
      location for those receipts) because then a "power that be" can read the
contract, and shut down the server at that IP.
        BUT NOW, possibly the solution may somehow lie in Namecoin, which is a
censorship-proof DHT where I could store an I2P address. Do I still need
      a DHT for the receipts themselves, as long as DNS is safe from
interference, and I can control where it points to? Since all parties (issuer
and
      users) would read their receipts from the same place (whichever address is
listed in Namecoin) and since it's on an anonymous network, then I
      believe the issuer and transaction server can exchange receipts in a safe
way. Possibly this solves any auditing issues, though I still need to
      devise the protocol.
      But this still means that someone is running a server somewhere, and must
be in order for people to get their receipts, whereas using a full DHT
      solution is more... certifiable. notarizable. Because you have the whole
blockchain verifying that receipt was posted...

        Maybe the issuer just needs to demand that all receipts are numbered
sequentially? hm.

 ===> NEW THOUGHT: For Auditing protocol:  EVERY RECEIPT for any specific asset
type should be sequentially numbered, AND should contain the hash of
      the receipt that came before it.  This way, the auditor can receive a
verifiable stream of receipts, which can also be queried by transaction #,
      which is unique to all transactions on the transaction server, as well as
queried by sequence number, which is unique to the issuer, as well as
      queried by hash value, which is used to prove sequence.
      Every receipt should be encrypted to a random key, and then that key
should be encrypted to three recipients: the server, the auditor, and the
      party doing the transaction. The receipts should otherwise be publicly
available (though encrypted) and auditors and parties should have to retrieve
      them from the same place.  Even if I allow the parties to keep the
receipts directly in response to their server messages, they can still compare
      notes directly with the auditor and each other on the hashes for the
various sequence numbers. Hm.
 */

#include "opentxs/stdafx.hpp"

#include "opentxs/core/script/OTSmartContract.hpp"

#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/api/Identity.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/consensus/ClientContext.hpp"
#include "opentxs/consensus/Context.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/core/cron/OTCron.hpp"
#include "opentxs/core/cron/OTCronItem.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/trade/OTMarket.hpp"
#include "opentxs/core/trade/OTOffer.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/cron/OTCron.hpp"
#include "opentxs/core/script/OTAgent.hpp"
#include "opentxs/core/script/OTBylaw.hpp"
#include "opentxs/core/script/OTClause.hpp"
#include "opentxs/core/script/OTParty.hpp"
#include "opentxs/core/script/OTPartyAccount.hpp"
#if OT_SCRIPT_CHAI
#include "opentxs/core/script/OTScriptChai.hpp"
#else
#include "opentxs/core/script/OTScript.hpp"
#endif
#include "opentxs/core/script/OTScriptable.hpp"
#include "opentxs/core/script/OTStash.hpp"
#include "opentxs/core/script/OTStashItem.hpp"
#include "opentxs/core/script/OTVariable.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/AccountList.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTStringXML.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/OT.hpp"

#if OT_SCRIPT_CHAI
#include <chaiscript/chaiscript.hpp>
#ifdef OT_USE_CHAI_STDLIB
#include <chaiscript/chaiscript_stdlib.hpp>
#endif
#endif
#include <irrxml/irrXML.hpp>

#include <ctime>
#include <memory>

#ifndef SMART_CONTRACT_PROCESS_INTERVAL
#define SMART_CONTRACT_PROCESS_INTERVAL                                        \
    30  // 30 seconds, for testing. Should be: based on fees. Otherwise once per
        // day should be enough... right?
#endif

// CALLBACK:  Party may cancel contract?
//
// Called in OnRemove and OnExpire,
// at the bottom.
//
#ifndef SMARTCONTRACT_CALLBACK_PARTY_MAY_CANCEL
#define SMARTCONTRACT_CALLBACK_PARTY_MAY_CANCEL                                \
    "callback_party_may_cancel_contract"
#endif

// FYI:
//#ifndef SCRIPTABLE_CALLBACK_PARTY_MAY_EXECUTE
//#define SCRIPTABLE_CALLBACK_PARTY_MAY_EXECUTE
//"callback_party_may_execute_clause"
//#endif

// HOOKS
//
// The server will call these hooks, from time to time, and give you the
// opportunity to provide your own scripts linked to these names, which
// will trigger at those times. (Parties are normally disallowed from
// directly triggering these special "hook" scripts, as they might normally
// be allowed to trigger other clauses.)
//

// Called regularly in OTSmartContract::ProcessCron()
// based on SMART_CONTRACT_PROCESS_INTERVAL.
//
#ifndef SMARTCONTRACT_HOOK_ON_PROCESS
#define SMARTCONTRACT_HOOK_ON_PROCESS "cron_process"
#endif

// This is called when the contract is
// first activated. Todo.
//
#ifndef SMARTCONTRACT_HOOK_ON_ACTIVATE
#define SMARTCONTRACT_HOOK_ON_ACTIVATE "cron_activate"
#endif

namespace opentxs
{

// TODO: Finish up Smart Contracts (this file.)

// DONE: Client test code, plus server message: for activating smart contracts.

// DONE: OT API calls for smart contracts.

// DONE: Build escrow script.

// TODO: Entities and roles.  (AFTER smart contracts are working.)

// TODO:  Finish move_funds, which should be super easy now. The script-callable
// version
// HAS to be in OTSmartContract. Why? Because it can't be in OTScriptable, since
// those shouldn't
// be allowed to move funds (they are unlike payment plan / trades, which have
// an open trans#
// that can be put onto receipts, and a final receipt with a closing trans#,
// which allow recurring
// processing (multiple receipts for the same transaction) -- this all fits into
// the "destruction
// of acct history" system perfectly, because it's built into OTCronItem.
// OTScriptable isn't derived
// from OTCronItem, so it can't do those things, therefore it can't be allowed
// to move money.
//
// Fair enough. But then, why not put move_funds into OTCronItem? After all, it
// IS derived from
// OTScriptable, if you go far enough back, and so it technically can play both
// sides. Here's why not:
// Because OTTrade and OTPaymentPlan were both written before I wrote all the
// smart contract stuff.
// It was only when I added smart contracts that I finally implemented Parties,
// Agents, Bylaws, Clauses,
// etc etc. This means that the way things are done is vastly different.
// Therefore while OTSmartContract
// is derived from OTCronItem, OTSmartContract is the first Cron Item that does
// it the SCRIPTABLE way,
// whereas OTTrade and OTPaymentPlan still do it the "OTCronItem" way.
//
// For example, in OTSmartContract, the Opening Transaction # is stored ONE FOR
// EACH PARTY, and the
// Closing Transaction # is stored ONE FOR EACH ASSET ACCOUNT.  You can have
// many many parties AND
// asset accounts on a smart contract.  This is simply not the case for OTTrade
// and OTPaymentPlan. They
// use their own little built-in system for managing their strict system for
// storing the exact numbers
// they need. They always use the exact same number of Nyms and accounts.
//
// But smart contracts obviously do much more, so a more elegant system has been
// crafted into OTSmartContract
// which blends the best qualities of OTScriptable AND OTCronItem, and which is
// the only place where you can
// have BOTH script interpretation (from OTScriptable) AND money-moving,
// receipts, transactions, etc (OTCronItem).
//
// Therefore, OTCronItem already has the lower-level call to MoveFunds, which
// you can see commented below.
// And OTSmartContract will have a higher-level version that can be available
// inside scripts as an "OT Native call",
// and which searches the parties/accounts/agents etc so that it is able to then
// make the lower-level call.
// Therefore move_funds goes in OTSmartContract, and MoveFunds goes in
// OTCronItem.
//
//
//

/*
CALLBACKS

DONE party_may_cancel_contract(party_name)                (See if a party is
allowed to cancel the contract from processing.)
DONE party_may_execute_clause(party_name, clause_name)    (See if a party is
allowed to execute any given clause.)

NATIVE CALLS

DONE move_funds(from_acct, to_acct, amount)        (from_acct and to_acct must
be a party to the agreement)
DONE stash_funds(from_acct, "stash_one", 100)    ("stash_one" is stored INSIDE
the smartcontract! Server-side only.)
DONE unstash_funds(to_acct, "stash_one", 100)    (Smartcontract must be
activated with NO stashes. Server creates/maintains stashes AFTER activation.)

DONE get_acct_balance(acct)                        Returns std::int64_t. (Named
acct must be party to agreement with legitimate authorized agent.)
DONE get_acct_instrument_definition_id(acct)                Returns std::string.
(Named
acct must be party to agreement with legitimate authorized agent.)
DONE get_stash_balance(stash, instrument_definition_id)    Returns std::int64_t.
(Named
stash must exist.)

DONE send_notice(to_party)        (Like sendMessage, except it comes from the
server, not another user. Drops current state of smart contract to Nymbox of all
agents for party named.)
DONE send_notice_to_parties()    (Does a send_notice to ALL parties.)

DONE deactivate_contract()        (Deactivates and finalizes the smart
contract.)

HOOKS

DONE cron_process        (Triggers every time the smart contract "processes" on
cron.)
DONE cron_activate        (Triggers when the smart contract is first activated.)
*/

// Global version. (string parameter)
// typedef bool (*OT_SM_RetBool_ThrStr)(OTSmartContract* pContract,
//                                       const std::string from_acct_name,
//                                       const std::string to_acct_name,
//                                       const std::string str_Amount);
// Test result:  WORKS calling chaiscript
// Cron: Processing smart contract clauses for hook: cron_process
// OTSmartContract::MoveAcctFunds: error: from_acct
// (sSBcoTlTkYY8pPv6vh2KD6mIVrRdIwodgsWDoJzIfpV) not found on any party.  //
// debug this in move_funds
// OTSmartContract::ExecuteClauses: Success executing script clause:
// process_clause.

// Global version. (std::int64_t parameter)
// typedef bool (*OT_SM_RetBool_TwoStr_OneL)(OTSmartContract* pContract,
//                                            const std::string from_acct_name,
//                                            const std::string to_acct_name,
//                                            const std::int64_t lAmount);
// TEST Result:  FAILS calling chaiscript: Cannot perform boxed_cast.   (Must be
// the LONG!!)
// Cron: Processing smart contract clauses for hook: cron_process
// OTScriptChai::ExecuteScript: Caught chaiscript::exception::bad_boxed_cast :
// Cannot perform boxed_cast.
// OTSmartContract::ExecuteClauses: Error while running script: process_clause

// Class member, with string parameter.
typedef bool (OTSmartContract::*OT_SM_RetBool_ThrStr)(
    std::string from_acct_name,
    std::string to_acct_name,
    std::string str_Amount);

// TEST RESULT: WORKS calling Chaiscript
// Cron: Processing smart contract clauses for hook: cron_process
// OTSmartContract::MoveAcctFunds: error: from_acct
// (sSBcoTlTkYY8pPv6vh2KD6mIVrRdIwodgsWDoJzIfpV) not found on any party.
// OTSmartContract::ExecuteClauses: Success executing script clause:
// process_clause.

// Class member, with std::int64_t parameter.
// typedef bool (OTSmartContract::*OT_SM_RetBool_TwoStr_OneL)(const std::string
// from_acct_name,
//                                                             const std::string
// to_acct_name,
//                                                             std::int64_t
//                                                             lAmount);

void OTSmartContract::RegisterOTNativeCallsWithScript(OTScript& theScript)
{
    // CALL THE PARENT
    OTScriptable::RegisterOTNativeCallsWithScript(theScript);

#if OT_SCRIPT_CHAI
    using namespace chaiscript;

    OTScriptChai* pScript = dynamic_cast<OTScriptChai*>(&theScript);

    if (nullptr != pScript) {
        OT_ASSERT(nullptr != pScript->chai_)

        // OT NATIVE FUNCTIONS
        // (These functions can be called from INSIDE the scripted clauses.)
        //                                                                                        //
        // Parameters must match as described below. Return value will be as
        // described below.
        //
        //      pScript->chai_->add(base_class<OTScriptable,
        //      OTSmartContract>());

        pScript->chai_->add(
            fun<OT_SM_RetBool_ThrStr>(&OTSmartContract::MoveAcctFundsStr, this),
            "move_funds");

        pScript->chai_->add(
            fun(&OTSmartContract::StashAcctFunds, this), "stash_funds");
        pScript->chai_->add(
            fun(&OTSmartContract::UnstashAcctFunds, this), "unstash_funds");
        pScript->chai_->add(
            fun(&OTSmartContract::GetAcctBalance, this), "get_acct_balance");
        pScript->chai_->add(
            fun(&OTSmartContract::GetInstrumentDefinitionIDofAcct, this),
            "get_acct_instrument_definition_id");
        pScript->chai_->add(
            fun(&OTSmartContract::GetStashBalance, this), "get_stash_balance");
        pScript->chai_->add(
            fun(&OTSmartContract::SendNoticeToParty, this), "send_notice");
        pScript->chai_->add(
            fun(&OTSmartContract::SendANoticeToAllParties, this),
            "send_notice_to_parties");
        pScript->chai_->add(
            fun(&OTSmartContract::SetRemainingTimer, this),
            "set_seconds_until_timer");
        pScript->chai_->add(
            fun(&OTSmartContract::GetRemainingTimer, this),
            "get_remaining_timer");

        pScript->chai_->add(
            fun(&OTSmartContract::DeactivateSmartContract, this),
            "deactivate_contract");

        // CALLBACKS
        // (Called by OT at key moments) todo security: What if these are
        // recursive? Need to lock down, put the smack down, on these smart
        // contracts.
        //
        // FYI:    pScript->chai_->add(fun(&(OTScriptable::CanExecuteClause),
        // (*this)), "party_may_execute_clause");    // From OTScriptable (FYI)
        // param_party_name and param_clause_name will be available inside
        // script. Script must return bool.
        // FYI:    #define SCRIPTABLE_CALLBACK_PARTY_MAY_EXECUTE
        // "callback_party_may_execute_clause"   <=== THE CALLBACK WITH THIS
        // NAME must be connected to a script clause, and then the clause will
        // trigger when the callback is needed.

        pScript->chai_->add(
            fun(&OTSmartContract::CanCancelContract, this),
            "party_may_cancel_contract");  // param_party_name
                                           // will be available
                                           // inside script.
                                           // Script must return
                                           // bool.
        // FYI:    #define SMARTCONTRACT_CALLBACK_PARTY_MAY_CANCEL
        // "callback_party_may_cancel_contract"  <=== THE CALLBACK WITH THIS
        // NAME must be connected to a script clause, and then the clause will
        // trigger when the callback is needed.

        // Callback USAGE:    Your clause, in your smart contract, may have
        // whatever name you want. (Within limits.)
        //                    There must be a callback entry in the smart
        // contract, linking your clause the the appropriate callback.
        //                    The CALLBACK ENTRY uses the names
        // "callback_party_may_execute_clause" and
        // "callback_party_may_cancel_contract".
        //                    If you want to call these from INSIDE YOUR SCRIPT,
        // then use the names "party_may_execute_clause" and
        // "party_may_cancel_contract".

        // HOOKS:
        //
        // Hooks are not native calls needing to be registered with the script.
        // (Like the above functions are.)
        // Rather, hooks are SCRIPT CLAUSES, that you have a CHOICE to provide
        // inside your SMART CONTRACT.
        // *IF* you have provided those clauses, then OT *WILL* call them, at
        // the appropriate times. (When
        // specific events occur.) Specifically, Hook entries must be in your
        // smartcontract, linking the below
        // standard hooks to your clauses.
        //
        // FYI:    #define SMARTCONTRACT_HOOK_ON_PROCESS        "cron_process"
        // // Called regularly in OTSmartContract::ProcessCron() based on
        // SMART_CONTRACT_PROCESS_INTERVAL.
        // FYI:    #define SMARTCONTRACT_HOOK_ON_ACTIVATE        "cron_activate"
        // // Done. This is called when the contract is first activated.
    } else
#endif  // OT_SCRIPT_CHAI
    {
        otErr << "OTSmartContract::RegisterOTNativeCallsWithScript: Failed "
                 "dynamic casting OTScript to OTScriptChai \n";
    }

}  // void function

void OTSmartContract::DeactivateSmartContract()  // Called from within script.
{
    // WARNING: If a party has the right to execute a clause that calls
    // DeactivateSmartContract(),
    // then that party can deactivate the smartcontract by calling that clause,
    // regardless of what
    // "CanPartyCancelClause()" says he can do.
    // (And by default, any legitimate party can trigger any clause, at any
    // time, unless your script
    // override says differently.)

    otOut << "OTSmartContract::DeactivateSmartContract: deactivate_contract() "
             "was called from within the script. "
             "Flagging smartcontract for removal from Cron ("
          << GetTransactionNum() << ").\n";

    FlagForRemoval();  // Remove it from future Cron processing, please.
}

// These are from OTScriptable (super-grandparent-class to *this):
/* ----------------------------------------------------
OTParty        * GetParty    (std::string str_party_name);
OTBylaw        * GetBylaw    (std::string str_bylaw_name);
OTClause    * GetClause    (std::string str_clause_name);
OTParty * FindPartyBasedOnNymAsAgent(OTPseudonym& theNym, OTAgent **
ppAgent=nullptr);
OTParty * FindPartyBasedOnNymAsAuthAgent(OTPseudonym& theNym, OTAgent **
ppAgent=nullptr);
OTParty * FindPartyBasedOnAccount(OTAccount& theAccount, OTPartyAccount **
ppPartyAccount=nullptr);
OTParty * FindPartyBasedOnNymIDAsAgent(const Identifier& theNymID, OTAgent **
ppAgent=nullptr);
OTParty * FindPartyBasedOnNymIDAsAuthAgent(const Identifier& theNymID,
OTAgent ** ppAgent=nullptr);
OTParty * FindPartyBasedOnAccountID(const Identifier& theAcctID,
OTPartyAccount ** ppPartyAccount=nullptr);
OTAgent            * GetAgent(std::string str_agent_name);
OTPartyAccount    * GetPartyAccount(std::string str_acct_name);
OTPartyAccount    * GetPartyAccountByID(const Identifier& theAcctID);
*/

// Returns true if it was empty (and thus successfully set.)
// Otherwise, if it wasn't empty (it had been already set) then
// it will fail to set in this call, and return false.
//
bool OTSmartContract::SetNotaryIDIfEmpty(const Identifier& theID)
{
    if (GetNotaryID().IsEmpty()) {
        SetNotaryID(theID);
        return true;
    }
    return false;
}

bool OTSmartContract::IsValidOpeningNumber(
    const std::int64_t& lOpeningNum) const
{
    for (const auto& it : m_mapParties) {
        OTParty* pParty = it.second;
        OT_ASSERT(nullptr != pParty);

        if (pParty->GetOpeningTransNo() == lOpeningNum) return true;
    }

    return false;
}

// Checks opening number on parties, and closing numbers on each party's
// accounts.
// Overrides from OTTrackable.
//
bool OTSmartContract::HasTransactionNum(const std::int64_t& lInput) const
{
    for (const auto& it : m_mapParties) {
        const OTParty* pParty = it.second;
        OT_ASSERT(nullptr != pParty);

        if (pParty->HasTransactionNum(lInput)) return true;
    }

    return false;
}

void OTSmartContract::GetAllTransactionNumbers(NumList& numlistOutput) const
{
    for (const auto& it : m_mapParties) {
        const OTParty* pParty = it.second;
        OT_ASSERT(nullptr != pParty);

        pParty->GetAllTransactionNumbers(numlistOutput);
    }
}

std::int64_t OTSmartContract::GetOpeningNumber(const Identifier& theNymID) const
{
    OTAgent* pAgent = nullptr;
    OTParty* pParty = FindPartyBasedOnNymIDAsAgent(theNymID, &pAgent);

    if (nullptr != pParty) {
        OT_ASSERT_MSG(
            nullptr != pAgent,
            "OT_ASSERT: nullptr != pAgent in "
            "OTSmartContract::GetOpeningNumber.\n");
        return pParty->GetOpeningTransNo();
    }

    return 0;
}

std::int64_t OTSmartContract::GetClosingNumber(
    const Identifier& theAcctID) const
{
    OTPartyAccount* pPartyAcct =
        GetPartyAccountByID(theAcctID);  // from OTScriptable.

    if (nullptr != pPartyAcct) {
        return pPartyAcct->GetClosingTransNo();
    }

    return 0;
}

/*
 " 6 minutes    ==      360 seconds\n"
 "10 minutes    ==      600 seconds\n"
 " 1 hour        ==     3600 seconds\n"
 " 1 day        ==    86400 seconds\n"
 "30 days        ==  2592000 seconds\n"
 " 3 months        ==  7776000 seconds\n"
 " 6 months        == 15552000 seconds\n\n"
 "12 months        == 31104000 seconds\n\n"
 */

// onProcess will trigger X seconds from now...
//
void OTSmartContract::SetRemainingTimer(
    std::string str_seconds_from_now)  // if this is <=0, then it sets next
                                       // process date to 0.
{
    if (str_seconds_from_now.size() <= 0)  // string length...
    {
        otOut << "OTSmartContract::" << __FUNCTION__
              << ": blank input (str_seconds_from_now).\n";
    } else {
        const std::int64_t tPlus = String::StringToLong(str_seconds_from_now);

        if (tPlus > 0)
            SetNextProcessDate(
                OTTimeAddTimeInterval(OTTimeGetCurrentTime(), tPlus));
        else
            SetNextProcessDate(OT_TIME_ZERO);  // This way, you can deactivate
                                               // the timer, by setting the next
                                               // process date to 0.
    }
}

std::string OTSmartContract::GetRemainingTimer() const  // returns seconds left
                                                        // on the timer, in
                                                        // string format, or
                                                        // "0".
{
    const time64_t& tNextDate = GetNextProcessDate();
    const time64_t tCurrent = OTTimeGetCurrentTime();

    String strReturnVal("0");  // the default return value is "0".

    if (tNextDate > OT_TIME_ZERO) {
        const std::int64_t tSecondsLeft =
            OTTimeGetTimeInterval(tNextDate, tCurrent);
        strReturnVal.Format("%" PRId64 "", tSecondsLeft);
    }

    return strReturnVal.Get();
}

void OTSmartContract::onRemovalFromCron()
{
    // Not much needed here.  Done, I guess.

    otErr << "FYI:  OTSmartContract::onRemovalFromCron was just called. \n";

    // Trigger a script maybe.
    // OR maybe it's too late for scripts.
    // I give myself an onRemoval() here in C++, but perhaps I cut
    // off the SCRIPTS after onFinalReceipt(). I think that's best.
}

// Done.
// called by HookActivationOnCron().
//
void OTSmartContract::onActivate()
{
    OT_ASSERT(nullptr != GetCron());

    if (GetCron()->GetTransactionCount() < 1) {
        otOut << __FUNCTION__ << ": Failed to process smart contract "
              << GetTransactionNum()
              << ": Out of transaction numbers for "
                 "receipts! Flagging for removal.\n";
        FlagForRemoval();
        return;
    }

    // Execute the scripts (clauses) that have registered for this hook.

    const std::string str_HookName(SMARTCONTRACT_HOOK_ON_ACTIVATE);
    mapOfClauses theMatchingClauses;

    if (GetHooks(str_HookName, theMatchingClauses)) {
        otOut << "Cron: Processing smart contract clauses for hook: "
              << SMARTCONTRACT_HOOK_ON_ACTIVATE << " \n";

        ExecuteClauses(
            theMatchingClauses);  // <============================================
    }
}

std::string OTSmartContract::GetAcctBalance(std::string from_acct_name)
{
    OTCron* pCron = GetCron();
    OT_ASSERT(nullptr != pCron);

    Nym* pServerNym = pCron->GetServerNym();
    OT_ASSERT(nullptr != pServerNym);

    // Below this point, these are all good:
    //
    //        pServerNym, pCron.
    //

    if (from_acct_name.size() <= 0) {
        otErr << "OTSmartContract::GetAcctBalance: error: from_acct_name is "
                 "non-existent.\n";
        return 0;
    }

    // Below this point, these are all good:
    //
    //        from_acct_name,
    //        pServerNym, pCron.

    OTPartyAccount* pFromAcct = GetPartyAccount(from_acct_name);

    if (nullptr == pFromAcct) {
        otOut << "OTSmartContract::GetAcctBalances: error: from_acct ("
              << from_acct_name << ") not found on any party.\n";
        return 0;
    }

    // Below this point, these are all good:
    //
    //        pFromAcct,    from_acct_name,
    //        pServerNym, pCron.

    OTAgent* pFromAgent =
        pFromAcct->GetAuthorizedAgent();  // This searches the account's party
                                          // for the account's authorized agent.
    // (That way it's impossible to get an agent for any other party.)

    if (nullptr == pFromAgent) {
        otOut << "OTSmartContract::GetAcctBalance: error: authorized agent ("
              << pFromAcct->GetAgentName() << ") not found for from_acct ("
              << from_acct_name << ") on acct's party.\n";
        return 0;
    }

    if (!pFromAgent->IsAnIndividual()) {
        otOut << "OTSmartContract::GetAcctBalance: error: authorized agent ("
              << pFromAcct->GetAgentName() << ") for from_acct ("
              << from_acct_name << ") is not an active agent.\n";
        return 0;
    }
    //
    // Below this point, these are all good:
    //
    //        pFromAcct,    from_acct_name,    pFromAgent,
    //        pServerNym, pCron.

    OTParty* pFromParty = pFromAgent->GetParty();

    if (nullptr == pFromParty) {
        otErr
            << "OTSmartContract::GetAcctBalance: error: Party pointer nullptr "
               "on authorized agent ("
            << pFromAcct->GetAgentName() << ") for from_acct ("
            << from_acct_name << ").\n";
        return 0;
    }
    //
    // Below this point, these are all good:
    //
    //        pFromAcct,    from_acct_name,    pFromAgent,    pFromParty,
    //        pServerNym, pCron.

    // Done: I can see that THIS VERIFICATION CODE WILL GET CALLED EVERY SINGLE
    // TIME THE SCRIPT
    // CALLS MOVE FUNDS.  Maybe that's good, but since technically this only
    // needs to be verified before the
    // first call, and not for EVERY call during any of a script's runs, I
    // should probably move this verification
    // higher, such as each time the OTCronItem triggers, plus each time a party
    // triggers a clause directly
    // through the API (server message). As long as those are covered, I will be
    // able to remove it from here
    // which should be a significant improvement for performance.
    // It will be at the bottom of those same functions that
    // "ClearTemporaryPointers()" should finally be called.
    //
    //
    //    FINAL DECISION: Redundant, already verified upon activation on cron.
    //  See longer comment in OTSmartContract::StashAcctFunds()
    //
    //    const OTString strNotaryID(GetNotaryID());
    //
    //    mapOfNyms    map_Nyms_Already_Loaded;
    //    RetrieveNymPointers(map_Nyms_Already_Loaded);
    //
    //
    //    if (!VerifyPartyAuthorization(*pFromParty, *pServerNym,
    // strNotaryID, &map_Nyms_Already_Loaded))
    //    {
    //        otErr << "OTSmartContract::GetAcctBalance: error: 'From' Party
    // (%s) not authorized for this contract.\n",
    //                      pFromParty->GetPartyName().c_str());
    //        return 0;
    //    }

    // A party might have many agents who are only voting groups, and cannot
    // actually sign for things
    // the way that nyms can. But at least ONE of those agents IS a Nym --
    // because there must have been
    // an authorizing agent who initially signed to accept the agreement, and
    // who fronted the opening
    // transaction number that activated it.
    //
    // Similarly, the authorized agent for any given party's account (each
    // account has its own authorized
    // agent) MUST be an active agent (an active agent is one with a
    // Nym--whether that Nym is representing
    // himself as the party, or whether representing some entity as an employee
    // in a role). Why MUST the
    // authorized agent be an active agent? Because when funds are moved, that
    // Nym must be loaded since
    // the account must show that Nym as a legal owner/agent. The MoveFunds will
    // cause a paymentReceipt to
    // drop into the Inbox for the relevant asset accounts, and that
    // paymentReceipt can ONLY be accepted
    // by that same Nym, who must use a transaction # that he signed for
    // previously and received through
    // his nymbox. There is actually no justification at all to take funds from
    // that account, since the
    // new balance has not yet been signed, UNLESS THE PAYMENTRECEIPT CONTAINS A
    // VALID, SIGNED AUTHORIZATION
    // FROM THE ACCOUNT HOLDER. *That* is why the authorizing agent must either
    // be the Party's Owner himself
    // (representing himself as an agent, which most will do) in which case he
    // will appear as the valid
    // owner of the account, OR he MUST be a Nym working in a Valid Role for an
    // Entity, where said Entity is
    // the valid owner on the account in question. Either OT, it will be
    // possible in OT for him to sign for
    // the paymentReceipts when they come in, and impossible for him to escape
    // liability for them.
    // (That's the idea anyway.)
    //
    // Since we know that the Authorized Agent for an account must be an ACTIVE
    // agent (one way or the other)
    // then we can error out here if he's not.  We can then pass in his Nym ID.
    //

    auto theFromAgentID = Identifier::Factory();
    const bool bFromAgentID = pFromAgent->GetSignerID(theFromAgentID);

    if (!bFromAgentID) {
        otErr << "OTSmartContract::GetAcctBalance: Failed to find FromAgent's "
                 "Signer ID: "
              << pFromAgent->GetName() << " \n";
        return 0;
    }

    if (!pFromAcct->GetAcctID().Exists()) {
        otErr << "OTSmartContract::GetAcctBalance: Error: FromAcct has empty "
                 "AcctID: "
              << from_acct_name << " \n";
        return 0;
    }

    const auto theFromAcctID = Identifier::Factory(pFromAcct->GetAcctID());
    //
    // BELOW THIS POINT, theFromAcctID and theFromAgentID available.

    const auto NOTARY_ID = Identifier::Factory(pCron->GetNotaryID());
    const auto NOTARY_NYM_ID = Identifier::Factory(*pServerNym);

    const std::string str_party_id = pFromParty->GetPartyID();
    const String strPartyID(str_party_id);
    const auto PARTY_NYM_ID = Identifier::Factory(strPartyID);

    const auto PARTY_ACCT_ID = Identifier::Factory(pFromAcct->GetAcctID());

    // Load up the party's account so we can get the balance.
    //
    Account* pPartyAssetAcct =
        Account::LoadExistingAccount(PARTY_ACCT_ID, NOTARY_ID);

    if (nullptr == pPartyAssetAcct) {
        otOut << "OTSmartContract::GetAcctBalance: ERROR verifying existence "
                 "of source account.\n";
        FlagForRemoval();  // Remove it from future Cron processing, please.
        return 0;
    } else if (!pPartyAssetAcct->VerifySignature(*pServerNym)) {
        otOut << "OTSmartContract::GetAcctBalance: ERROR failed to verify the "
                 "server's signature on the party's account.\n";
        FlagForRemoval();  // Remove it from future Cron processing, please.
        return 0;
    } else if (!pPartyAssetAcct->VerifyOwnerByID(PARTY_NYM_ID)) {
        otOut << "OTSmartContract::GetAcctBalance: ERROR failed to verify "
                 "party user ownership of party account.\n";
        FlagForRemoval();  // Remove it from future Cron processing, please.
        return 0;
    }
    // Past this point we know pPartyAssetAcct is good and will clean itself up.
    std::unique_ptr<Account> theSourceAcctSmrtPtr(pPartyAssetAcct);

    String strBalance;
    strBalance.Format("%" PRId64, pPartyAssetAcct->GetBalance());

    return strBalance.Get();
}

std::string OTSmartContract::GetInstrumentDefinitionIDofAcct(
    std::string from_acct_name)
{
    OTCron* pCron = GetCron();
    OT_ASSERT(nullptr != pCron);

    Nym* pServerNym = pCron->GetServerNym();
    OT_ASSERT(nullptr != pServerNym);

    // Below this point, these are all good:
    //
    //        pServerNym, pCron.
    //

    std::string str_return_value;

    if (from_acct_name.size() <= 0) {
        otErr << "OTSmartContract::GetInstrumentDefinitionIDofAcct: error: "
                 "from_acct_name "
                 "is non-existent.\n";
        return str_return_value;
    }

    // Below this point, these are all good:
    //
    //        from_acct_name,
    //        pServerNym, pCron.

    OTPartyAccount* pFromAcct = GetPartyAccount(from_acct_name);

    if (nullptr == pFromAcct) {
        otOut << "OTSmartContract::GetInstrumentDefinitionIDofAcct: error: "
                 "from_acct ("
              << from_acct_name << ") not found on any party.\n";
        return str_return_value;
    }

    // Below this point, these are all good:
    //
    //        pFromAcct,    from_acct_name,
    //        pServerNym, pCron.

    OTAgent* pFromAgent =
        pFromAcct->GetAuthorizedAgent();  // This searches the account's party
                                          // for the account's authorized agent.
    // (That way it's impossible to get an agent for any other party.)

    if (nullptr == pFromAgent) {
        otOut << "OTSmartContract::GetInstrumentDefinitionIDofAcct: error: "
                 "authorized "
                 "agent ("
              << pFromAcct->GetAgentName() << ") not found for from_acct ("
              << from_acct_name << ") on acct's party.\n";
        return str_return_value;
    }

    if (!pFromAgent->IsAnIndividual()) {
        otOut << "OTSmartContract::GetInstrumentDefinitionIDofAcct: error: "
                 "authorized "
                 "agent ("
              << pFromAcct->GetAgentName() << ") for from_acct ("
              << from_acct_name << ") is not an active agent.\n";
        return str_return_value;
    }
    //
    // Below this point, these are all good:
    //
    //        pFromAcct,    from_acct_name,    pFromAgent,
    //        pServerNym, pCron.

    OTParty* pFromParty = pFromAgent->GetParty();

    if (nullptr == pFromParty) {
        otErr << "OTSmartContract::GetInstrumentDefinitionIDofAcct: error: "
                 "Party pointer "
                 "nullptr on authorized agent ("
              << pFromAcct->GetAgentName() << ") for from_acct ("
              << from_acct_name << ").\n";
        return str_return_value;
    }
    //
    // Below this point, these are all good:
    //
    //        pFromAcct,    from_acct_name,    pFromAgent,    pFromParty,
    //        pServerNym, pCron.

    // Done: I can see that THIS VERIFICATION CODE WILL GET CALLED EVERY SINGLE
    // TIME THE SCRIPT
    // CALLS MOVE FUNDS.  Maybe that's good, but since technically this only
    // needs to be verified before the
    // first call, and not for EVERY call during any of a script's runs, I
    // should probably move this verification
    // higher, such as each time the OTCronItem triggers, plus each time a party
    // triggers a clause directly
    // through the API (server message). As long as those are covered, I will be
    // able to remove it from here
    // which should be a significant improvement for performance.
    // It will be at the bottom of those same functions that
    // "ClearTemporaryPointers()" should finally be called.
    //
    //
    // FINAL DECISION: Redunant, already done upon activation onto cron.
    // Furthermore, expects no stashes to exist,
    // since they can only be created after activation. (Interfered with script
    // operations by complaining whenever
    // there was a stash.) See longer comment in StashAcctFunds().
    //
    //    const OTString strNotaryID(GetNotaryID());
    //
    //    mapOfNyms    map_Nyms_Already_Loaded;
    //    RetrieveNymPointers(map_Nyms_Already_Loaded);
    //
    //
    //    if (!VerifyPartyAuthorization(*pFromParty, *pServerNym,
    // strNotaryID, &map_Nyms_Already_Loaded))
    //    {
    //        otErr << "OTSmartContract::GetInstrumentDefinitionIDofAcct: error:
    //        'From'
    // Party (%s) not authorized for this contract.\n",
    //                      pFromParty->GetPartyName().c_str());
    //        return str_return_value;
    //    }

    // A party might have many agents who are only voting groups, and cannot
    // actually sign for things
    // the way that nyms can. But at least ONE of those agents IS a Nym --
    // because there must have been
    // an authorizing agent who initially signed to accept the agreement, and
    // who fronted the opening
    // transaction number that activated it.
    //
    // Similarly, the authorized agent for any given party's account (each
    // account has its own authorized
    // agent) MUST be an active agent (an active agent is one with a
    // Nym--whether that Nym is representing
    // himself as the party, or whether representing some entity as an employee
    // in a role). Why MUST the
    // authorized agent be an active agent? Because when funds are moved, that
    // Nym must be loaded since
    // the account must show that Nym as a legal owner/agent. The MoveFunds will
    // cause a paymentReceipt to
    // drop into the Inbox for the relevant asset accounts, and that
    // paymentReceipt can ONLY be accepted
    // by that same Nym, who must use a transaction # that he signed for
    // previously and received through
    // his nymbox. There is actually no justification at all to take funds from
    // that account, since the
    // new balance has not yet been signed, UNLESS THE PAYMENTRECEIPT CONTAINS A
    // VALID, SIGNED AUTHORIZATION
    // FROM THE ACCOUNT HOLDER. *That* is why the authorizing agent must either
    // be the Party's Owner himself
    // (representing himself as an agent, which most will do) in which case he
    // will appear as the valid
    // owner of the account, OR he MUST be a Nym working in a Valid Role for an
    // Entity, where said Entity is
    // the valid owner on the account in question. Either OT, it will be
    // possible in OT for him to sign for
    // the paymentReceipts when they come in, and impossible for him to escape
    // liability for them.
    // (That's the idea anyway.)
    //
    // Since we know that the Authorized Agent for an account must be an ACTIVE
    // agent (one way or the other)
    // then we can error out here if he's not.  We can then pass in his Nym ID.
    //

    auto theFromAgentID = Identifier::Factory();
    const bool bFromAgentID = pFromAgent->GetSignerID(theFromAgentID);

    if (!bFromAgentID) {
        otErr << "OTSmartContract::GetInstrumentDefinitionIDofAcct: Failed to "
                 "find "
                 "FromAgent's Signer ID: "
              << pFromAgent->GetName() << " \n";
        return str_return_value;
    }

    if (!pFromAcct->GetAcctID().Exists()) {
        otErr << "OTSmartContract::GetInstrumentDefinitionIDofAcct: Error: "
                 "FromAcct has "
                 "empty AcctID: "
              << from_acct_name << " \n";
        return str_return_value;
    }

    const auto theFromAcctID = Identifier::Factory(pFromAcct->GetAcctID());
    //
    // BELOW THIS POINT, theFromAcctID and theFromAgentID available.

    const auto NOTARY_ID = Identifier::Factory(pCron->GetNotaryID());
    const auto NOTARY_NYM_ID = Identifier::Factory(*pServerNym);

    const std::string str_party_id = pFromParty->GetPartyID();
    const String strPartyID(str_party_id);
    const auto PARTY_NYM_ID = Identifier::Factory(strPartyID);

    const auto PARTY_ACCT_ID = Identifier::Factory(pFromAcct->GetAcctID());

    // Load up the party's account and get the instrument definition.
    //
    Account* pPartyAssetAcct =
        Account::LoadExistingAccount(PARTY_ACCT_ID, NOTARY_ID);

    if (nullptr == pPartyAssetAcct) {
        otOut << "OTSmartContract::GetInstrumentDefinitionIDofAcct: ERROR "
                 "verifying "
                 "existence of source account.\n";
        FlagForRemoval();  // Remove it from future Cron processing, please.
        return str_return_value;
    } else if (!pPartyAssetAcct->VerifySignature(*pServerNym)) {
        otOut << "OTSmartContract::GetInstrumentDefinitionIDofAcct: ERROR "
                 "failed to "
                 "verify the server's signature on the party's account.\n";
        FlagForRemoval();  // Remove it from future Cron processing, please.
        return str_return_value;
    } else if (!pPartyAssetAcct->VerifyOwnerByID(PARTY_NYM_ID)) {
        otOut << "OTSmartContract::GetInstrumentDefinitionIDofAcct: ERROR "
                 "failed to "
                 "verify party user ownership of party account.\n";
        FlagForRemoval();  // Remove it from future Cron processing, please.
        return str_return_value;
    }
    // Past this point we know pPartyAssetAcct is good and will clean itself up.
    std::unique_ptr<Account> theSourceAcctSmrtPtr(pPartyAssetAcct);

    const String strInstrumentDefinitionID(
        pPartyAssetAcct->GetInstrumentDefinitionID());
    str_return_value = strInstrumentDefinitionID.Get();

    return str_return_value;
}

std::string OTSmartContract::GetStashBalance(
    std::string from_stash_name,
    std::string instrument_definition_id)
{
    OTCron* pCron = GetCron();
    OT_ASSERT(nullptr != pCron);

    Nym* pServerNym = pCron->GetServerNym();
    OT_ASSERT(nullptr != pServerNym);

    // Below this point, these are all good:
    //
    //        pServerNym, pCron.
    //

    if (from_stash_name.size() <= 0) {
        otErr << "OTSmartContract::GetStashBalance: error: from_stash_name is "
                 "non-existent.\n";
        return 0;
    }
    if (instrument_definition_id.size() <= 0) {
        otErr << "OTSmartContract::GetStashBalance: error: "
                 "instrument_definition_id is "
                 "non-existent.\n";
        return 0;
    }

    // Below this point, these are all good:
    //
    //        from_stash_name,
    //        instrument_definition_id
    //        pServerNym, pCron.
    //

    OTStash* pStash = GetStash(from_stash_name);  // This ALWAYS succeeds.
                                                  // (It will OT_ASSERT()
                                                  // if failure.)

    //
    // Below this point, these are all good:
    //
    //        pStash,        from_stash_name,
    //        instrument_definition_id
    //        pServerNym, pCron.
    //
    String strBalance;
    strBalance.Format("%" PRId64, pStash->GetAmount(instrument_definition_id));
    return strBalance.Get();
}

bool OTSmartContract::SendANoticeToAllParties()
{
    OTCron* pCron = GetCron();
    OT_ASSERT(nullptr != pCron);

    Nym* pServerNym = pCron->GetServerNym();
    OT_ASSERT(nullptr != pServerNym);

    // Below this point, these are all good:
    //
    //        pServerNym, pCron.
    //

    const std::int64_t lNewTransactionNumber =
        pCron->GetNextTransactionNumber();
    bool bDroppedNotice = false;

    //    OT_ASSERT(lNewTransactionNumber > 0); // this can be my reminder.
    if (0 == lNewTransactionNumber) {
        otErr << __FUNCTION__
              << ": ** ERROR: Notice not sent to parties, "
                 "since no transaction numbers were "
                 "available!\n";
    } else {
        ReleaseSignatures();
        SignContract(*pServerNym);
        SaveContract();

        const String strReference(*this);
        bDroppedNotice = SendNoticeToAllParties(
            true,  // bSuccessMsg=true
            *pServerNym,
            GetNotaryID(),
            lNewTransactionNumber,
            // GetTransactionNum(), // each party has its own opening number.
            strReference);  // pstrNote and pstrAttachment aren't used in this
                            // case.

        otOut << __FUNCTION__
              << ": Dropping notifications into all parties' nymboxes: "
              << (bDroppedNotice ? "Success" : "Failure") << "\n";
    }

    return bDroppedNotice;
}

bool OTSmartContract::SendNoticeToParty(std::string party_name)
{
    OTCron* pCron = GetCron();
    OT_ASSERT(nullptr != pCron);

    Nym* pServerNym = pCron->GetServerNym();
    OT_ASSERT(nullptr != pServerNym);

    // Below this point, these are all good:
    //
    //        pServerNym, pCron.

    OTParty* pParty = GetParty(party_name);

    if (nullptr == pParty) {
        otOut << __FUNCTION__ << ": Unable to find this party: " << party_name
              << "\n";
        return false;
    }
    // Below this point, pParty is good.

    // ...This WILL check to see if pParty has its Opening number verified as
    // issued.
    // (If the opening number is > 0 then VerifyPartyAuthorization() is smart
    // enough to verify it.)
    //
    // To KNOW that a party has the right to even ASK the script to cancel a
    // contract, MEANS that
    // (1) The party is listed as a party on the contract. (2) The party's copy
    // of that contract
    // is signed by the authorizing agent for that party. and (3) The opening
    // transaction number for
    // that party is verified as issued for authorizing agent. (2 and 3 are both
    // performed at the same
    // time, in VerifyPartyAuthorization(), since the agent may need to be
    // loaded in order to verify
    // them.) 1 is already done by this point, as it's performed above.
    //
    // Done: notice this code appears in CanCancelContract() (this function) as
    // well as
    // OTScriptable::CanExecuteClause.
    // Therefore I can see that THIS VERIFICATION CODE WILL GET CALLED EVERY
    // SINGLE TIME THE SCRIPT
    // CALLS ANY CLAUSE OR OT NATIVE FUNCTION.  Since technically this only
    // needs to be verified before the
    // first call, and not for EVERY call during any of a script's runs, I
    // should probably move this verification
    // higher, such as each time the OTCronItem triggers, plus each time a party
    // triggers a clause directly
    // through the API (server message). As long as those are covered, I will be
    // able to remove it from here
    // which should be a significant improvement for performance.
    // It will be at the bottom of those same functions that
    // "ClearTemporaryPointers()" should finally be called.
    //
    // Also todo:  Need to implement MOVE CONSTRUCTORS and MOVE COPY
    // CONSTRUCTORS all over the place,
    // once I'm sure C++0x build environments are available for all of the
    // various OT platforms. That should
    // be another great performance boost!
    //
    //    FINAL DECISION: See longer comment in
    // OTSmartContract::StashAcctFunds(). This is redundant and was
    //  removed. It expects itself to be executed only upon the initial
    // activation of a smart contract, and
    //  not again after. (For example, it disallows stashes, which cannot exist
    // prior to activation.)
    //
    bool bDroppedNotice = false;
    const std::int64_t lNewTransactionNumber =
        pCron->GetNextTransactionNumber();

    //    OT_ASSERT(lNewTransactionNumber > 0); // this can be my reminder.
    if (0 == lNewTransactionNumber) {
        otErr << __FUNCTION__
              << ": ** ERROR: Notice not sent to party, since "
                 "no transaction numbers were available!\n";
    } else {
        ReleaseSignatures();
        SignContract(*pServerNym);
        SaveContract();

        const String strReference(*this);

        bDroppedNotice = pParty->SendNoticeToParty(
            true,  // bSuccessMsg=true. True in general means "success" and
                   // false means "failure."
            *pServerNym,
            GetNotaryID(),
            lNewTransactionNumber,
            // GetTransactionNum(), // each party has its own opening trans #
            // and supplies it internally.
            strReference);

        otOut << __FUNCTION__ << ": "
              << (bDroppedNotice ? "Success" : "Failure")
              << " dropping notification into party's nymbox: "
              << pParty->GetPartyName() << "\n";
    }

    return bDroppedNotice;
}

// Higher-level. Can be called from inside scripts.
//
// Returns success if funds were moved.
// This function does not run any scripts, but it CAN be executed from within
// the scripts.
// Any movement of funds to-or-from any account will automatically try to
// load/use the
// appropriate authorizing agent for that account (or use him, if he's already
// loaded on
// this smart contract.)
//
// DONE: audit security. Whenever I add any funds to a stash, there should be an
// internal
// server account where the backing funds are stored, the same as with cash.
// This is so that
// stashed funds will show up properly on an audit.
//
bool OTSmartContract::StashAcctFunds(
    std::string from_acct_name,
    std::string to_stash_name,
    std::string str_Amount)
{
    OTCron* pCron = GetCron();
    OT_ASSERT(nullptr != pCron);

    Nym* pServerNym = pCron->GetServerNym();
    OT_ASSERT(nullptr != pServerNym);

    // Below this point, these are all good:
    //
    //        pServerNym, pCron.

    if (str_Amount.size() < 1) {
        otOut << "OTSmartContract::StashAcctFunds: Error: empty amount.\n";
        return false;
    }

    const std::int64_t lAmount = String::StringToLong(str_Amount.c_str());

    if (lAmount <= 0) {
        otOut << "OTSmartContract::StashAcctFunds: Error: lAmount cannot be 0 "
                 "or <0. (Value passed in was "
              << lAmount << ".)\n";
        return false;
    }

    if (from_acct_name.size() <= 0) {
        otErr << "OTSmartContract::StashAcctFunds: error: from_acct_name is "
                 "non-existent.\n";
        return false;
    }
    if (to_stash_name.size() <= 0) {
        otErr << "OTSmartContract::StashAcctFunds: error: to_stash_name is "
                 "non-existent.\n";
        return false;
    }

    // Below this point, these are all good:
    //
    //        from_acct_name,
    //        to_stash_name,
    //        pServerNym, pCron.
    //

    OTPartyAccount* pFromAcct = GetPartyAccount(from_acct_name);
    OTStash* pStash = GetStash(to_stash_name);  // This ALWAYS succeeds.
                                                // (It will OT_ASSERT() if
                                                // failure.)

    if (nullptr == pFromAcct) {
        otOut << "OTSmartContract::StashAcctFunds: error: from_acct ("
              << from_acct_name << ") not found on any party.\n";
        return false;
    }

    // Below this point, these are all good:
    //
    //        pFromAcct,    from_acct_name,
    //        pStash,        to_stash_name,
    //        pServerNym, pCron.
    //

    OTAgent* pFromAgent =
        pFromAcct->GetAuthorizedAgent();  // This searches the account's party
                                          // for the account's authorized agent.
    // (That way it's impossible to get an agent for any other party.)

    if (nullptr == pFromAgent) {
        otOut << "OTSmartContract::StashAcctFunds: error: authorized agent ("
              << pFromAcct->GetAgentName() << ") not found for from_acct ("
              << from_acct_name << ") on acct's party.\n";
        return false;
    }

    if (!pFromAgent->IsAnIndividual()) {
        otOut << "OTSmartContract::StashAcctFunds: error: authorized agent ("
              << pFromAcct->GetAgentName() << ") for from_acct ("
              << from_acct_name << ") is not an active agent.\n";
        return false;
    }
    //
    // Below this point, these are all good:
    //
    //        pFromAcct,    from_acct_name,    pFromAgent,
    //        pStash,        to_stash_name,
    //        pServerNym, pCron.

    OTParty* pFromParty = pFromAgent->GetParty();

    if (nullptr == pFromParty) {
        otErr
            << "OTSmartContract::StashAcctFunds: error: Party pointer nullptr "
               "on authorized agent ("
            << pFromAcct->GetAgentName() << ") for from_acct ("
            << from_acct_name << ").\n";
        return false;
    }
    //
    // Below this point, these are all good:
    //
    //        pFromAcct,    from_acct_name,    pFromAgent,    pFromParty,
    //        pStash,        to_stash_name,
    //        pServerNym, pCron.

    // DONE: Problem: I can see that THIS VERIFICATION CODE WILL GET CALLED
    // EVERY SINGLE TIME THE SCRIPT
    // CALLS MOVE FUNDS.  Maybe that's good, but since technically this only
    // needs to be verified before the
    // first call, and not for EVERY call during any of a script's runs, I
    // should probably move this verification
    // higher, such as each time the OTCronItem triggers, plus each time a party
    // triggers a clause directly
    // through the API (server message). As long as those are covered, I will be
    // able to remove it from here
    // which should be a significant improvement for performance.
    // It will be at the bottom of those same functions that
    // "ClearTemporaryPointers()" should finally be called.
    //
    //
    //    const OTString strNotaryID(GetNotaryID());

    //    mapOfNyms    map_Nyms_Already_Loaded;
    //    RetrieveNymPointers(map_Nyms_Already_Loaded);

    // FINAL DECISION: Since this verification is already done for the smart
    // contract to have been activated onto
    // cron in the first place, it is redundant to have it here.
    // Furthermore, what if a single clause calls stash_funds 30 times in a
    // loop? Does that mean we need to verify
    // all the parties and accounts 30 times, every single time we call that
    // clause? CLearly such is redundant, and
    // would be targeted in any optimization effort.
    // Furthermore, since actually running the software, it's become apparent
    // that it is no longer even appropriate
    // to Verify anymore, since stashes can exist after a smartcontract has been
    // activated, while this function
    // specifically verifies that no stashes exist, since it presumes that it is
    // only called during the initial
    // activation of the cron item, when no stashes SHOULD exist (since they are
    // only created after activation, by
    // the server itself, at the behest of the scripts.)

    // A party might have many agents who are only voting groups, and cannot
    // actually sign for things
    // the way that nyms can. But at least ONE of those agents IS a Nym --
    // because there must have been
    // an authorizing agent who initially signed to accept the agreement, and
    // who fronted the opening
    // transaction number that activated it.
    //
    // Similarly, the authorized agent for any given party's account (each
    // account has its own authorized
    // agent) MUST be an active agent (an active agent is one with a
    // Nym--whether that Nym is representing
    // himself as the party, or whether representing some entity as an employee
    // in a role). Why MUST the
    // authorized agent be an active agent? Because when funds are moved, that
    // Nym must be loaded since
    // the account must show that Nym as a legal owner/agent. The MoveFunds will
    // cause a paymentReceipt to
    // drop into the Inbox for the relevant asset accounts, and that
    // paymentReceipt can ONLY be accepted
    // by that same Nym, who must use a transaction # that he signed for
    // previously and received through
    // his nymbox. There is actually no justification at all to take funds from
    // that account, since the
    // new balance has not yet been signed, UNLESS THE PAYMENTRECEIPT CONTAINS A
    // VALID, SIGNED AUTHORIZATION
    // FROM THE ACCOUNT HOLDER. *That* is why the authorizing agent must either
    // be the Party's Owner himself
    // (representing himself as an agent, which most will do) in which case he
    // will appear as the valid
    // owner of the account, OR he MUST be a Nym working in a Valid Role for an
    // Entity, where said Entity is
    // the valid owner on the account in question. Either OT, it will be
    // possible in OT for him to sign for
    // the paymentReceipts when they come in, and impossible for him to escape
    // liability for them.
    // (That's the idea anyway.)
    //
    // Since we know that the Authorized Agent for an account must be an ACTIVE
    // agent (one way or the other)
    // then we can error out here if he's not.  We can then pass in his Nym ID.
    //

    auto theFromAgentID = Identifier::Factory();
    const bool bFromAgentID = pFromAgent->GetSignerID(theFromAgentID);

    if (!bFromAgentID) {
        otErr << "OTSmartContract::StashAcctFunds: Failed to find FromAgent's "
                 "Signer ID: "
              << pFromAgent->GetName() << " \n";
        return false;
    }

    if (!pFromAcct->GetAcctID().Exists()) {
        otErr << "OTSmartContract::StashAcctFunds: Error: FromAcct has empty "
                 "AcctID: "
              << from_acct_name << " \n";
        return false;
    }

    const auto theFromAcctID = Identifier::Factory(pFromAcct->GetAcctID());
    //
    // BELOW THIS POINT, theFromAcctID and theFromAgentID available.

    // WE SET THESE HERE SO THE RECEIPT SHOWS, SUCCESS OR FAIL,
    // WHO THE INTENDED SENDER / RECIPIENT ARE FOR THAT RECEIPT.
    //
    ReleaseLastSenderRecipientIDs();

    theFromAgentID->GetString(m_strLastSenderUser);  // This is the last Nym ID
    // of a party who SENT money.
    theFromAcctID->GetString(
        m_strLastSenderAcct);  // This is the last Acct ID of
                               // a party who SENT money.
    //    theToAgentID.GetString(m_strLastRecipientUser);    // This is the last
    // Nym ID of a party who RECEIVED money.
    //    theToAcctID.GetString(m_strLastRecipientAcct);    // This is the last
    // Acct ID of a party who RECEIVED money.
    // Above: the ToAgent and ToAcct are commented out,
    // since the funds are going into a stash.

    mapOfConstNyms map_Nyms_Already_Loaded;
    RetrieveNymPointers(map_Nyms_Already_Loaded);

    bool bMoved = StashFunds(
        map_Nyms_Already_Loaded,
        lAmount,
        theFromAcctID,
        theFromAgentID,
        *pStash);
    if (!bMoved) {
        otOut << "OTSmartContract::StashAcctFunds: Failed in final call. "
                 "Values: from_acct: "
              << from_acct_name << ", to_stash: " << to_stash_name
              << ", lAmount: " << lAmount << ". \n";
        return false;
    }

    return true;
}

// Higher-level. Can be called from inside scripts.
//
// Returns success if funds were moved.
// This function does not run any scripts, but it CAN be executed from within
// the scripts.
// Any movement of funds to-or-from any account will automatically try to
// load/use the
// appropriate authorizing agent for that account (or use him, if he's already
// loaded on
// this smart contract.)
//
// DONE: audit security. Whenever I add any funds to a stash, there should be an
// internal
// server account where the backing funds are stored, the same as with cash.
// This is so that
// stashed funds will show up properly on an audit.
//
bool OTSmartContract::UnstashAcctFunds(
    std::string to_acct_name,
    std::string from_stash_name,
    std::string str_Amount)
{
    OTCron* pCron = GetCron();
    OT_ASSERT(nullptr != pCron);

    Nym* pServerNym = pCron->GetServerNym();
    OT_ASSERT(nullptr != pServerNym);

    // Below this point, these are all good:
    //
    //        pServerNym, pCron.
    //

    if (str_Amount.size() < 1) {
        otOut << "OTSmartContract::UnstashAcctFunds: Error: empty amount.\n";
        return false;
    }

    const std::int64_t lAmount = String::StringToLong(str_Amount.c_str());

    if (lAmount <= 0) {
        otOut << "OTSmartContract::UnstashAcctFunds: Error: lAmount cannot be "
                 "0 or <0. (Value passed in was "
              << lAmount << ".)\n";
        return false;
    }

    if (to_acct_name.size() <= 0) {
        otErr << "OTSmartContract::UnstashAcctFunds: error: to_acct_name is "
                 "non-existent.\n";
        return false;
    }
    if (from_stash_name.size() <= 0) {
        otErr << "OTSmartContract::UnstashAcctFunds: error: from_stash_name is "
                 "non-existent.\n";
        return false;
    }

    // Below this point, these are all good:
    //
    //        to_acct_name,
    //        from_stash_name,
    //        pServerNym, pCron.

    OTPartyAccount* pToAcct = GetPartyAccount(to_acct_name);
    OTStash* pStash = GetStash(from_stash_name);  // This ALWAYS succeeds.
                                                  // (It will OT_ASSERT()
                                                  // if failure.)

    if (nullptr == pToAcct) {
        otOut << "OTSmartContract::UnstashAcctFunds: error: to_acct ("
              << to_acct_name << ") not found on any party.\n";
        return false;
    }

    // Below this point, these are all good:
    //
    //        pToAcct,    to_acct_name,
    //        pStash,        from_stash_name,
    //        pServerNym, pCron.

    OTAgent* pToAgent =
        pToAcct->GetAuthorizedAgent();  // This searches the account's party for
                                        // the account's authorized agent.
    // (That way it's impossible to get an agent for any other party.)

    if (nullptr == pToAgent) {
        otOut << "OTSmartContract::UnstashAcctFunds: error: authorized agent ("
              << pToAcct->GetAgentName().Get() << ") not found for to_acct ("
              << to_acct_name << ") on acct's party.\n";
        return false;
    }

    if (!pToAgent->IsAnIndividual()) {
        otOut << "OTSmartContract::UnstashAcctFunds: error: authorized agent ("
              << pToAcct->GetAgentName().Get() << ") for to_acct ("
              << to_acct_name << ") is not an active agent.\n";
        return false;
    }
    //
    // Below this point, these are all good:
    //
    //        pToAcct,    to_acct_name,        pToAgent,
    //        pStash,        from_stash_name,
    //        pServerNym, pCron.

    OTParty* pToParty = pToAgent->GetParty();

    if (nullptr == pToParty) {
        otErr << "OTSmartContract::UnstashAcctFunds: error: Party pointer "
                 "nullptr "
                 "on authorized agent ("
              << pToAcct->GetAgentName() << ") for to_acct (" << to_acct_name
              << ").\n";
        return false;
    }
    //
    // Below this point, these are all good:
    //
    //        pToAcct,    to_acct_name,        pToAgent,    pToParty,
    //        pStash,        from_stash_name,
    //        pServerNym, pCron.

    // Done: I can see that THIS VERIFICATION CODE WILL GET CALLED EVERY SINGLE
    // TIME THE SCRIPT
    // CALLS MOVE FUNDS.  Maybe that's good, but since technically this only
    // needs to be verified before the
    // first call, and not for EVERY call during any of a script's runs, I
    // should probably move this verification
    // higher, such as each time the OTCronItem triggers, plus each time a party
    // triggers a clause directly
    // through the API (server message). As long as those are covered, I will be
    // able to remove it from here
    // which should be a significant improvement for performance.
    // It will be at the bottom of those same functions that
    // "ClearTemporaryPointers()" should finally be called.
    //
    // FINAL DECISION: Redundant, removed. See comments in StashAcctFunds().
    //
    // A party might have many agents who are only voting groups, and cannot
    // actually sign for things
    // the way that nyms can. But at least ONE of those agents IS a Nym --
    // because there must have been
    // an authorizing agent who initially signed to accept the agreement, and
    // who fronted the opening
    // transaction number that activated it.
    //
    // Similarly, the authorized agent for any given party's account (each
    // account has its own authorized
    // agent) MUST be an active agent (an active agent is one with a
    // Nym--whether that Nym is representing
    // himself as the party, or whether representing some entity as an employee
    // in a role). Why MUST the
    // authorized agent be an active agent? Because when funds are moved, that
    // Nym must be loaded since
    // the account must show that Nym as a legal owner/agent. The MoveFunds will
    // cause a paymentReceipt to
    // drop into the Inbox for the relevant asset accounts, and that
    // paymentReceipt can ONLY be accepted
    // by that same Nym, who must use a transaction # that he signed for
    // previously and received through
    // his nymbox. There is actually no justification at all to take funds from
    // that account, since the
    // new balance has not yet been signed, UNLESS THE PAYMENTRECEIPT CONTAINS A
    // VALID, SIGNED AUTHORIZATION
    // FROM THE ACCOUNT HOLDER. *That* is why the authorizing agent must either
    // be the Party's Owner himself
    // (representing himself as an agent, which most will do) in which case he
    // will appear as the valid
    // owner of the account, OR he MUST be a Nym working in a Valid Role for an
    // Entity, where said Entity is
    // the valid owner on the account in question. Either OT, it will be
    // possible in OT for him to sign for
    // the paymentReceipts when they come in, and impossible for him to escape
    // liability for them.
    // (That's the idea anyway.)
    //
    // Since we know that the Authorized Agent for an account must be an ACTIVE
    // agent (one way or the other)
    // then we can error out here if he's not.  We can then pass in his Nym ID.
    //

    auto theToAgentID = Identifier::Factory();
    const bool bToAgentID = pToAgent->GetSignerID(theToAgentID);

    if (!bToAgentID) {
        otErr << "OTSmartContract::UnstashAcctFunds: Failed to find 'To' "
                 "Agent's Signer ID: "
              << pToAgent->GetName() << " \n";
        return false;
    }

    if (!pToAcct->GetAcctID().Exists()) {
        otErr << "OTSmartContract::UnstashAcctFunds: Error: ToAcct has empty "
                 "AcctID: "
              << to_acct_name << " \n";
        return false;
    }

    const auto theToAcctID = Identifier::Factory(pToAcct->GetAcctID());
    //
    // BELOW THIS POINT, theToAcctID and theToAgentID available.

    // WE SET THESE HERE SO THE RECEIPT SHOWS, SUCCESS OR FAIL,
    // WHO THE INTENDED SENDER / RECIPIENT ARE FOR THAT RECEIPT.
    //
    ReleaseLastSenderRecipientIDs();

    theToAgentID->GetString(m_strLastRecipientUser);  // This is the last Nym ID
                                                      // of a party who RECEIVED
                                                      // money.
    theToAcctID->GetString(m_strLastRecipientAcct);  // This is the last Acct ID
                                                     // of a party who RECEIVED
                                                     // money.
    // Above: the FromAgent and FromAcct are commented out,
    // since the funds are coming from a stash.

    const std::int64_t lNegativeAmount = (lAmount * (-1));

    mapOfConstNyms map_Nyms_Already_Loaded;
    RetrieveNymPointers(map_Nyms_Already_Loaded);

    bool bMoved = StashFunds(
        map_Nyms_Already_Loaded,
        lNegativeAmount,
        theToAcctID,
        theToAgentID,
        *pStash);
    if (!bMoved) {
        otOut << "OTSmartContract::UnstashAcctFunds: Failed in final call. "
                 "Values: to_acct: "
              << to_acct_name << ", from_stash: " << from_stash_name
              << ", lAmount: " << lAmount << ". \n";
        return false;
    }

    return true;
}

// OTSmartContract::StashFunds is lower-level; it's used inside StashAcctFunds()
// and UnstashAcctFunds(). (Similarly to how OTCronItem::MoveFunds() is used in-
// side OTSmartContract::MoveAcctFunds().
//
// true == success, false == failure.
//
bool OTSmartContract::StashFunds(
    const mapOfConstNyms& map_NymsAlreadyLoaded,
    const std::int64_t& lAmount,  // negative amount here means UNstash.
                                  // Positive means STASH.
    const Identifier& PARTY_ACCT_ID,
    const Identifier& PARTY_NYM_ID,
    OTStash& theStash)
{
    OTCron* pCron = GetCron();
    OT_ASSERT(nullptr != pCron);

    Nym* pServerNym = pCron->GetServerNym();
    OT_ASSERT(nullptr != pServerNym);

    if (0 == lAmount) {
        otOut << "OTSmartContract::StashFunds: a zero amount is not allowed.\n";
        return false;
    }

    const auto NOTARY_ID = Identifier::Factory(pCron->GetNotaryID());
    const auto NOTARY_NYM_ID = Identifier::Factory(*pServerNym);

    // Load up the party's account and get the instrument definition, so we know
    // which
    // stash to get off the stash.
    //
    Account* pPartyAssetAcct =
        Account::LoadExistingAccount(PARTY_ACCT_ID, NOTARY_ID);

    if (nullptr == pPartyAssetAcct) {
        otOut << "OTSmartContract::StashFunds: ERROR verifying existence of "
                 "source account.\n";
        FlagForRemoval();  // Remove it from future Cron processing, please.
        return false;
    } else if (!pPartyAssetAcct->VerifySignature(*pServerNym)) {
        otOut << "OTSmartContract::StashFunds: ERROR failed to verify the "
                 "server's signature on the party's account.\n";
        FlagForRemoval();  // Remove it from future Cron processing, please.
        return false;
    } else if (!pPartyAssetAcct->VerifyOwnerByID(PARTY_NYM_ID)) {
        otOut << "OTSmartContract::StashFunds: ERROR failed to verify party "
                 "user ownership of party account.\n";
        FlagForRemoval();  // Remove it from future Cron processing, please.
        return false;
    }
    // Past this point we know pPartyAssetAcct is good and will clean itself up.
    std::unique_ptr<Account> theSourceAcctSmrtPtr(pPartyAssetAcct);

    //
    // There could be many stashes, each with a name. (One was passed in
    // here...)
    // And inside each one is a stash for each instrument definition. So let's
    // get the one
    // for the instrument definition matching the party's account.
    //
    const String strInstrumentDefinitionID(
        pPartyAssetAcct->GetInstrumentDefinitionID());
    const std::string str_instrument_definition_id =
        strInstrumentDefinitionID.Get();

    OTStashItem* pStashItem = theStash.GetStash(str_instrument_definition_id);
    OT_ASSERT(nullptr != pStashItem);  // should never happen. Creates if
                                       // non-existent.

    // Below this point, pStashItem is good, and has the right
    // instrument_definition_id.

    //
    const bool bUnstashing =
        (lAmount < 0);  // If the amount is negative, then we're UNSTASHING.
    const std::int64_t lAbsoluteAmount =
        bUnstashing
            ? (lAmount * (-1))
            : lAmount;  // NEGATIVE AMOUNT SHOULD BEHAVE AS "UNSTASH FUNDS" !!

    // Normally if you stash 10 clams, then your account is -10 clams, and your
    // stash is +10 clams.
    // Therefore if you unstash 5 gg, then your gg acct is +5 grams and your
    // stash is -5 grams.
    //
    // Thus if lAmount is > 0, as normal, that amount should be DEBITED from the
    // Party Acct, and CREDITED to the Stash Acct.
    // Whereas if lAmount were < 0, then that amount should be DEBITED from the
    // Stash Acct, and CREDITED to the Party Acct.

    // Below this point, lAbsoluteAmount is always a positive number.
    // Whereas if lAmount < 0, that means we are doing an UNSTASH in this
    // function.
    //

    // Whether the funds are coming from the party's acct, or from the stash,
    // WHEREVER they
    // are coming from, is that source LARGE enough to accommodate the amount
    // we're trying to move?
    //

    const std::int64_t lPartyAssetBalance = pPartyAssetAcct->GetBalance();
    const std::int64_t lStashItemAmount = pStashItem->GetAmount();

    const std::int64_t lSourceAmount =
        bUnstashing ? lStashItemAmount : lPartyAssetBalance;

    // If the source, minus amount, is less than 0, then it CANNOT accommodate
    // the action.
    //
    if ((lSourceAmount - lAbsoluteAmount) < 0) {
        otOut << "OTSmartContract::StashFunds: Not enough funds available in "
                 "the source to accommodate this action.\n";
        return false;
    }

    bool bWasAcctCreated =
        false;  // GetOrRegisterAccount() will verifyContractID
                // and verifySignature on the account
                // internally.
    std::shared_ptr<Account> pStashAccount = m_StashAccts.GetOrRegisterAccount(
        *pServerNym,
        NOTARY_NYM_ID,
        pPartyAssetAcct->GetInstrumentDefinitionID(),
        NOTARY_ID,
        bWasAcctCreated,
        GetTransactionNum());

    if (!pStashAccount) {
        OT_FAIL_MSG("ASSERT in OTSmartContract::StashFunds: returned nullptr "
                    "pointer (should never happen.)\n");
    }

    if (bWasAcctCreated) {
        String strAcctID;
        pStashAccount->GetIdentifier(strAcctID);

        otOut << "OTSmartContract::StashFunds: Successfully created stash "
                 "account ID: "
              << strAcctID << "\n (Stash acct has Instrument Definition ID: "
              << strInstrumentDefinitionID << ") \n";

        // Todo: Some kind of save here?
        // It's kind of unnecessary, since I've already verified that there's
        // enough funds at the source
        // to successfully do the transfer, AND I will already save at the end
        // of this call, since funds are
        // being moved.
    }

    if (!pStashAccount) {
        OT_FAIL_MSG("ASSERT in OTSmartContract::StashFunds: returned nullptr "
                    "pointer (should never happen.)\n");
    }

    // This code is similar to above, but it checks the stash ACCT itself
    // instead of the stash entry.
    //
    // Whether the funds are coming from the party's acct, or from the stash
    // acct, WHEREVER they
    // are coming from, is that source LARGE enough to accommodate the amount
    // we're trying to move?
    //
    const std::int64_t lSourceAmount2 = bUnstashing
                                            ? pStashAccount->GetBalance()
                                            : pPartyAssetAcct->GetBalance();

    // If the source, minus amount, is less than 0, then it CANNOT accommodate
    // the action.
    //
    if ((lSourceAmount2 - lAbsoluteAmount) < 0) {
        otOut << "OTSmartContract::StashFunds: Not enough funds available in "
                 "the stash acct to accommodate this action.\n";
        return false;  // THIS SHOULD NEVER HAPPEN, SINCE WE ALREADY VERIFIED
                       // THE AMOUNT BEFORE LOADING THE ACCOUNT. FYI.
    }

    // Make sure they're not the same Account IDs ...
    // Otherwise we would have to take care not to load them twice, like with
    // the Nyms below.
    // (Instead I just disallow it entirely. After all, even if I DID allow the
    // account to transfer
    // to itself, there would be no difference in balance than disallowing it.)
    //
    const auto STASH_ACCT_ID =
        Identifier::Factory(pStashAccount->GetRealAccountID());

    if (PARTY_ACCT_ID == STASH_ACCT_ID) {
        otErr << "OTSmartContract::StashFunds: ERROR: both account IDs were "
                 "identical.\n";
        FlagForRemoval();  // Remove from Cron
        return false;  // TODO: should have a "Validate Scripts" function that
                       // weeds this crap out before we even get here. (There
                       // are other examples...)
    }

    // SHOULD NEVER HAPPEN
    if (pPartyAssetAcct->GetInstrumentDefinitionID() !=
        pStashAccount->GetInstrumentDefinitionID()) {
        otErr << "OTSmartContract::StashFunds: Aborted stash: Asset type ID "
                 "doesn't match. THIS SHOULD NEVER HAPPEN.\n";
        FlagForRemoval();  // Remove from Cron
        return false;
    }

    if (!pStashAccount->VerifyOwnerByID(NOTARY_NYM_ID)) {
        otErr << "OTSmartContract::StashFunds: Error: Somehow the stash "
                 "account isn't server-nym owned.\n";
        FlagForRemoval();  // Remove from Cron
        return false;
    }
    const auto STASH_NYM_ID = Identifier::Factory(pStashAccount->GetNymID());

    bool bSuccess = false;  // The return value.

    String strPartyNymID(PARTY_NYM_ID), strStashNymID(STASH_NYM_ID),
        strPartyAcctID(PARTY_ACCT_ID), strStashAcctID(STASH_ACCT_ID),
        strServerNymID(NOTARY_NYM_ID);

    // Need to load up the ORIGINAL VERSION OF THIS SMART CONTRACT
    // Will need to verify the party's signature, as well as attach a copy of it
    // to the receipt.

    // OTCronItem::LoadCronReceipt loads the original version with the user's
    // signature.
    // (Updated versions, as processing occurs, are signed by the server.)
    std::unique_ptr<OTCronItem> pOrigCronItem(
        OTCronItem::LoadCronReceipt(GetTransactionNum()));

    OT_ASSERT(nullptr != pOrigCronItem);  // How am I processing it now if the
                                          // receipt wasn't saved in the first
                                          // place??
    // TODO: Decide global policy for handling situations where the hard drive
    // stops working, etc.

    // strOrigPlan is a String copy (a PGP-signed XML file, in string form) of
    // the original smart contract activation request...
    String strOrigPlan(*pOrigCronItem);  // <====== Farther down in the code, I
                                         // attach this string to the receipts.

    // -------------- Make sure have both nyms loaded and checked out.
    // --------------------------------------------------
    // WARNING: The party's Nym could be also the Server Nym. But the Stash Nym
    // is ALWAYS the server.
    // In all of those different cases, I don't want to load the same file twice
    // and overwrite it with itself, losing
    // half of all my changes. I have to check all three IDs carefully and set
    // the pointers accordingly, and then operate
    // using the pointers from there.

    Nym thePartyNym;
    // Find out if party Nym is actually also the server nym.
    const bool bPartyNymIsServerNym =
        ((PARTY_NYM_ID == NOTARY_NYM_ID) ? true : false);
    const Nym* pPartyNym = nullptr;
    const std::string str_party_id = strPartyNymID.Get();
    auto it_party = map_NymsAlreadyLoaded.find(str_party_id);

    if (map_NymsAlreadyLoaded.end() !=
        it_party)  // found the party in list of Nyms that are already loaded.
    {
        pPartyNym = it_party->second;
        OT_ASSERT(
            (nullptr != pPartyNym) && (pPartyNym->CompareID(PARTY_NYM_ID)));
    }

    // Figure out if party Nym is also Server Nym.
    if (bPartyNymIsServerNym) {
        // If the First Nym is the server, then just point to that.
        pPartyNym = pServerNym;
    } else if (nullptr == pPartyNym)  // Else load the First Nym from storage,
                                      // if still not found.
    {
        thePartyNym.SetIdentifier(PARTY_NYM_ID);  // thePartyNym is pPartyNym

        if (!thePartyNym.LoadPublicKey()) {
            otErr << "OTSmartContract::StashFunds: Failure loading party Nym "
                     "public key: "
                  << strPartyNymID << "\n";
            FlagForRemoval();  // Remove it from future Cron processing, please.
            return false;
        }

        if (thePartyNym.VerifyPseudonym() &&
            thePartyNym.LoadSignedNymfile(*pServerNym))  // ServerNym here is
                                                         // not thePartyNym's
                                                         // identity, but merely
                                                         // the signer on this
                                                         // file.
        {
            otWarn << "OTSmartContract::StashFunds: Loading party Nym, since "
                      "he apparently wasn't already loaded.\n"
                      "(On a cron item processing, this is normal. But if you "
                      "triggered a clause directly, then your Nym SHOULD be "
                      "already loaded...)\n";
            pPartyNym = &thePartyNym;  //  <=====
        } else {
            otErr << "OTSmartContract::StashFunds: Failure loading or "
                     "verifying party Nym public key: "
                  << strPartyNymID << "\n";
            FlagForRemoval();  // Remove it from future Cron processing, please.
            return false;
        }
    }
    // Below this point, both Nyms are loaded and good-to-go.

    mapOfConstNyms map_ALREADY_LOADED;  // I know I passed in one of these, but
                                        // now I have processed the Nym pointers
                                        // (above) and have better data here
                                        // now.
    map_ALREADY_LOADED.insert(std::pair<std::string, const Nym*>(
        strServerNymID.Get(),
        pServerNym));  // Add Server Nym to list of Nyms already loaded.

    auto it_temp = map_ALREADY_LOADED.find(strPartyNymID.Get());
    if (map_ALREADY_LOADED.end() == it_temp)
        map_ALREADY_LOADED.insert(std::pair<std::string, const Nym*>(
            strPartyNymID.Get(),
            pPartyNym));  // Add party Nym to list of Nyms already loaded.

    // In this function, pStashNym and pServerNym are always the same.

    if (!pOrigCronItem->VerifyNymAsAgent(
            *pPartyNym,
            *pServerNym,
            // In case it needs to load the AUTHORIZING agent, and that agent
            // is already loaded, it will have access here.
            &map_ALREADY_LOADED)) {
        otErr << "OTSmartContract::StashFunds: Failed authorization for party "
                 "Nym: "
              << strPartyNymID << "\n";
        FlagForRemoval();  // Remove it from Cron.
        return false;
    }

    // AT THIS POINT, I have:    pServerNym, pPartyNym, and pStashNym,
    // PLUS:                    pStashAccount and pPartyAssetAcct

    // VerifySignature, VerifyContractID, and VerifyOwner have all been called
    // already
    // by this point. This is new:
    // (They might fall away in favor of this, once I start building.)
    //
    if (!VerifyNymAsAgentForAccount(*pPartyNym, *pPartyAssetAcct)) {
        otOut << "OTSmartContract::StashFunds: ERROR verifying ownership on "
                 "source account.\n";
        FlagForRemoval();  // Remove it from future Cron processing, please.
        return false;
    } else {
        // Okay then, everything checks out. Let's add a receipt to the party's
        // inbox.
        // (No need for the stash's inbox -- the server owns it.)

        // Load the inbox in case it already exists
        Ledger thePartyInbox(PARTY_NYM_ID, PARTY_ACCT_ID, NOTARY_ID);

        // ALL inboxes -- no outboxes. All will receive notification of
        // something ALREADY DONE.
        bool bSuccessLoadingPartyInbox = thePartyInbox.LoadInbox();

        // ...or generate them otherwise...

        if (true == bSuccessLoadingPartyInbox)
            bSuccessLoadingPartyInbox =
                thePartyInbox.VerifyAccount(*pServerNym);
        else
            otErr << "OTSmartContract::StashFunds: Failed trying to load "
                     "party's inbox.\n";
        //            OT_FAIL_MSG("ASSERT:  TRYING TO GENERATE INBOX IN STASH
        // FUNDS!!!\n");
        //            bSuccessLoadingPartyInbox        =
        // thePartyInbox.GenerateLedger(PARTY_ACCT_ID, NOTARY_ID,
        // OTLedger::inbox, true); // bGenerateFile=true

        if (!bSuccessLoadingPartyInbox) {
            otErr << "OTSmartContract::StashFunds: ERROR loading or generating "
                     "inbox ledger.\n";
        } else {
            // Generate new transaction numbers for these new transactions
            std::int64_t lNewTransactionNumber =
                pCron->GetNextTransactionNumber();

            //          OT_ASSERT(lNewTransactionNumber > 0); // this can be my
            //          reminder.
            if (0 == lNewTransactionNumber) {
                otOut << "OTSmartContract::StashFunds: Aborted move: There are "
                         "no more transaction numbers available in Cron.\n";
                // (Here I do NOT flag for removal.)
                return false;
            }

            OTTransaction* pTransParty = OTTransaction::GenerateTransaction(
                thePartyInbox,
                OTTransaction::paymentReceipt,
                originType::origin_smart_contract,
                lNewTransactionNumber);
            // (No need to OT_ASSERT on the above new transaction since it
            // occurs in GenerateTransaction().)

            // The party's inbox will get a receipt with a new transaction ID on
            // it, owned by the server.
            // It will have a "In reference to" field containing the original
            // signed smart contract.
            // (with all party's signatures from their authorizing agents.)

            // set up the transaction item (each transaction may have multiple
            // items... but not in this case.)
            //
            Item* pItemParty = Item::CreateItemFromTransaction(
                *pTransParty, Item::paymentReceipt);
            OT_ASSERT(
                nullptr != pItemParty);  //  may be unnecessary, I'll have to
                                         // check CreateItemFromTransaction.
                                         // I'll leave for now.

            pItemParty->SetStatus(Item::rejection);  // the default.

            //          const std::int64_t lPartyTransRefNo =
            //          GetTransactionNum();
            const std::int64_t lPartyTransRefNo =
                GetOpeningNumber(PARTY_NYM_ID);

            // Here I make sure that each receipt (each inbox notice) references
            // the original
            // transaction number that was used to set the cron item into
            // place...
            // This number is used to track all cron items. (All Cron items
            // require a transaction
            // number from the user in order to add them to Cron in the first
            // place.)
            //
            // The number is also used to uniquely identify all other
            // transactions, as you
            // might guess from its name.
            //
            // UPDATE: Notice I'm now looking up a different number based on the
            // NymID. This is to support smart contracts, which have many
            // parties, agents, and accounts.
            //
            //          pItemParty->SetReferenceToNum(lPartyTransRefNo);
            pTransParty->SetReferenceToNum(lPartyTransRefNo);

            // The TRANSACTION (a receipt in my inbox) will be sent with "In
            // Reference To" information
            // containing the ORIGINAL SIGNED SMARTCONTRACT. (With all parties'
            // original signatures on it.)
            //
            // Whereas the TRANSACTION ITEM will include an "attachment"
            // containing the UPDATED
            // SMART CONTRACT (this time with the SERVER's signature on it.)
            //
            // Here's the original one going onto the transaction:
            //
            pTransParty->SetReferenceString(strOrigPlan);

            //
            // MOVE THE DIGITAL ASSETS FROM ONE ACCOUNT TO ANOTHER...
            //
            // Calculate the amount and debit/ credit the accounts
            // Make sure each Account can afford it, and roll back in case of
            // failure.

            // Normally if you stash 10 clams, then your account is -10 clams,
            // and your stash is +10 clams.
            // Therefore if you unstash 5 gg, then your gg acct is +5 grams and
            // your stash is -5 grams.
            //
            // Thus if lAmount is > 0, as normal, that amount should be DEBITED
            // from the Party Acct, and CREDITED to the Stash Acct.
            // Whereas if lAmount were < 0, then that amount should be DEBITED
            // from the Stash Acct, and CREDITED to the Party Acct.

            bool bMoveParty = false;
            bool bMoveStash = false;

            if (bUnstashing)  //  Debit Stash, Credit Party
            {
                if (pStashAccount->GetBalance() >= lAbsoluteAmount) {
                    // Debit the stash account.
                    bMoveStash = pStashAccount->Debit(
                        lAbsoluteAmount);  // <====== DEBIT FUNDS

                    // IF success, credit the party.
                    if (bMoveStash) {
                        bMoveParty = pPartyAssetAcct->Credit(
                            lAbsoluteAmount);  // <=== CREDIT FUNDS

                        // Okay, we already took it from the stash account.
                        // But if we FAIL to credit the party, then we need to
                        // PUT IT BACK in the stash acct.
                        // (EVEN THOUGH we'll just "NOT SAVE" after any failure,
                        // so it's really superfluous.)
                        //
                        if (!bMoveParty) {
                            bool bErr = pStashAccount->Credit(
                                lAbsoluteAmount);  // put the money back

                            otErr << "OTSmartContract::StashFunds: While "
                                     "succeeded debiting the stash account, "
                                     "FAILED in: "
                                     "pPartyAssetAcct->Credit(lAbsoluteAmount);"
                                     " \n"
                                     "Also, tried to credit stash account back "
                                     "again. Result: "
                                  << (bErr ? "success" : "failure") << ".\n";
                        } else {  // SUCCESS!
                            //
                            bool bStashSuccess = pStashItem->DebitStash(
                                lAbsoluteAmount);  // we already verified above
                                                   // that this stash item has
                                                   // enough funds to
                                                   // successfully debit.

                            if (bStashSuccess)
                                bSuccess = true;
                            else
                                otErr << "OTSmartContract::StashFunds: ERROR: "
                                         "Debited stash account and credited "
                                         "party account, but "
                                         "then unable to debit the stash "
                                         "record inside the smart contract "
                                         "itself.\n";
                        }
                    } else {
                        otErr << "OTSmartContract::StashFunds: FAILED in:  "
                                 "pStashAccount->Debit(lAbsoluteAmount);\n";
                    }
                }
            } else  // Debit party, Credit Stash
            {
                if (pPartyAssetAcct->GetBalance() >= lAbsoluteAmount) {
                    // Debit the party account.
                    bMoveParty = pPartyAssetAcct->Debit(
                        lAbsoluteAmount);  // <====== DEBIT FUNDS

                    // IF success, credit the Stash.
                    if (bMoveParty) {
                        bMoveStash = pStashAccount->Credit(
                            lAbsoluteAmount);  // <=== CREDIT FUNDS

                        // Okay, we already took it from the party account.
                        // But if we FAIL to credit the Stash, then we need to
                        // PUT IT BACK in the party acct.
                        // (EVEN THOUGH we'll just "NOT SAVE" after any failure,
                        // so it's really superfluous.)
                        //
                        if (!bMoveStash) {
                            bool bErr = pPartyAssetAcct->Credit(
                                lAbsoluteAmount);  // put the money back

                            otErr << "OTSmartContract::StashFunds: While "
                                     "succeeded debiting the asset account, "
                                     "FAILED in: "
                                     "pStashAccount->Credit(lAbsoluteAmount); "
                                     "\n"
                                     "Also, tried to credit asset account back "
                                     "again. Result: "
                                  << (bErr ? "success" : "failure") << ".\n";
                        } else {  // SUCCESS!
                            //
                            bool bStashSuccess = pStashItem->CreditStash(
                                lAbsoluteAmount);  // we already verified above
                                                   // that this stash item has
                                                   // enough funds to
                                                   // successfully debit.

                            if (bStashSuccess)
                                bSuccess = true;
                            else
                                otErr << "OTSmartContract::StashFunds: ERROR: "
                                         "Debited party account and credited "
                                         "stash account, but "
                                         "then unable to credit the stash "
                                         "record inside the smart contract "
                                         "itself.\n";
                        }
                    } else {
                        otErr << "OTSmartContract::StashFunds: FAILED in:  "
                                 "pPartyAssetAcct->Debit(lAbsoluteAmount);\n";
                    }
                }
            }

            // If ANY of these failed, then roll them all back and break.
            // (In fact we could just be checking bSuccess here, I wager.
            // Might as well be thorough.)
            //
            if (!bMoveParty || !bMoveStash) {
                // No need to roll back pStashItem here, since it is never
                // changed in the
                // first place unless BOTH of the above bools were successful.

                otErr << "OTSmartContract::StashFunds: Very strange! Funds "
                         "were available but "
                         "debit "
                      << ((bUnstashing) ? "stash" : "party") << " or credit "
                      << ((bUnstashing) ? "party" : "stash")
                      << " failed while performing move.\n";
                // We won't save the files anyway, if this failed.
                bSuccess = false;
            }

            // DO NOT SAVE ACCOUNTS if bSuccess is false.
            // We only save these accounts if bSuccess == true.
            // (But we do save the inboxes either way, since payment failures
            // always merit an inbox notice.)
            //
            if (true == bSuccess)  // The payment succeeded.
            {
                // The party needs to get a receipt in his inbox.
                //
                pItemParty->SetStatus(
                    Item::acknowledgement);  // pPartyAssetAcct

                const std::int64_t lReceiptAmount = (lAmount * (-1));

                //                pItemParty->SetAmount(lAmount);    // lAmount
                // is already negative or positive by the time it's passed into
                // this function.
                pItemParty->SetAmount(lReceiptAmount);  // However, if we are
                                                        // stashing 100, that
                                                        // means my account is
                                                        // -100. Therefore
                                                        // multiply by (-1)
                                                        // EITHER WAY.
                //                pItemParty->SetAmount(lAbsoluteAmount*(-1));
                // // "paymentReceipt" is otherwise ambigious about whether you
                // are paying or being paid.
                // This is also like market receipts, which use negative and
                // positive amounts.

                otOut << "OTSmartContract::StashFunds: Move performed.\n";

                // (I do NOT save m_pCron here, since that already occurs after
                // this function is called.)
            } else  // bSuccess = false.  The payment failed.
            {
                pItemParty->SetStatus(Item::rejection);  // pPartyAssetAcct
                                                         // // These are
                                                         // already initialized
                                                         // to false.
                pItemParty->SetAmount(
                    0);  // No money changed hands. Just being explicit.

                otOut << "OTSmartContract::StashFunds: Move failed.\n";
            }

            // Everytime a payment processes, a receipt is put in the user's
            // inbox, containing a
            // CURRENT copy of the cron item (which took just money from the
            // user's acct, or not,
            // and either way thus updated its status -- so its internal state
            // has changed.)
            //
            // It will also contain a copy of the user's ORIGINAL signed cron
            // item, where the data
            // has NOT changed, (so the user's original signature is still
            // good.) Although in the case of
            // smart contracts, each party stores their own signed copy anyway,
            // so it doesn't matter as
            // much.
            //
            // In order for it to export the RIGHT VERSION of the CURRENT smart
            // contract, which has just
            // changed (above), then I need to re-sign it and save it first.
            // (The original version I'll
            // load from a separate file using
            // OTSmartContract::LoadCronReceipt(lTransactionNum).
            //
            // I should be able to call a method on the original cronitem, where
            // I ask it to verify a certain
            // nym as being acceptable to that cron item as an agent, based on
            // the signature of the original
            // authorizing agent for that party. UPDATE: I believe the function
            // described in this paragraph
            // is now done.
            //

            ReleaseSignatures();
            SignContract(*pServerNym);
            SaveContract();

            //
            // EVERYTHING BELOW is just about notifying the party, by dropping
            // the receipt in his
            // inbox. The rest is done.  The two accounts and the inbox will all
            // be saved at the same time.
            //
            // The Smart Contract is entirely updated and saved by this point,
            // and Cron will
            // also be saved in the calling function once we return (no matter
            // what.)
            //

            // Basically I load up the INBOX, which is actually a LEDGER, and
            // then I create
            // a new transaction, with a new transaction item, for that ledger.
            // (That's where the receipt information goes.)
            //

            // The TRANSACTION will be sent with "In Reference To" information
            // containing the
            // ORIGINAL SIGNED SMART CONTRACT. (With both of the users' original
            // signatures on it.)
            //
            // Whereas the TRANSACTION ITEM will include an "attachment"
            // containing the UPDATED
            // SMART CONTRACT (this time with the SERVER's signature on it.)

            // (Lucky I just signed and saved the updated smart contract
            // (above), or it would still have
            // have the old data in it.)

            // I also already loaded the original smart contact. Remember this
            // from above,
            // near the top of the function:
            //  OTSmartContract * pOrigCronItem    = nullptr;
            //     OTString strOrigPlan(*pOrigCronItem); // <====== Farther down
            // in the code, I attach this string to the receipts.
            //  ... then lower down...
            // pTransParty->SetReferenceString(strOrigPlan);
            //
            // So the original plan is already loaded and copied to the
            // Transaction as the "In Reference To"
            // Field. Now let's add the UPDATED plan as an ATTACHMENT on the
            // Transaction ITEM:
            //

            //

            String strUpdatedCronItem(*this);

            // Set the updated cron item as the attachment on the transaction
            // item.
            // (With the SERVER's signature on it!)
            // (As a receipt for the party, so they can see their smartcontract
            // updating.)
            //
            pItemParty->SetAttachment(strUpdatedCronItem);

            // Success OR failure, either way I want a receipt in the inbox.
            // ===> But if FAILURE, I do NOT want to save the Accounts, JUST the
            // inbox!!
            //
            // (So the inbox happens either way, but the accounts are saved only
            // on success.)

            // sign the item
            pItemParty->SignContract(*pServerNym);
            pItemParty->SaveContract();

            // the Transaction "owns" the item now and will handle cleaning it
            // up.
            pTransParty->AddItem(*pItemParty);
            pTransParty->SignContract(*pServerNym);
            pTransParty->SaveContract();

            // Here, the transaction we just created is actually added to the
            // inbox ledger.
            // This happens either way, success or fail.

            thePartyInbox.AddTransaction(*pTransParty);

            // Release any signatures that were there before (They won't
            // verify anymore anyway, since the content has changed.)
            //
            thePartyInbox.ReleaseSignatures();
            thePartyInbox.SignContract(*pServerNym);
            thePartyInbox.SaveContract();

            pPartyAssetAcct->SaveInbox(thePartyInbox);

            // This corresponds to the AddTransaction() call just above.
            // These are stored in a separate file now.
            //
            pTransParty->SaveBoxReceipt(thePartyInbox);

            // If success, save the accounts with new balance. (Save inboxes
            // with receipts either way,
            // and the receipts will contain a rejection or acknowledgment
            // stamped by the Server Nym.)
            //
            if (true == bSuccess) {
                // SAVE THE ACCOUNTS.

                // Release any signatures that were there before (They won't
                // verify anymore anyway, since the content has changed.)
                //
                pPartyAssetAcct->ReleaseSignatures();
                pStashAccount->ReleaseSignatures();

                // Sign both of them.
                pPartyAssetAcct->SignContract(*pServerNym);
                pStashAccount->SignContract(*pServerNym);

                // Save both of them internally
                pPartyAssetAcct->SaveContract();
                pStashAccount->SaveContract();

                // TODO: Better rollback capabilities in case of failures here:

                // Save both accounts to storage.
                pPartyAssetAcct->SaveAccount();
                pStashAccount->SaveAccount();
                // NO NEED TO LOG HERE, since success / failure is already
                // logged above.
            }
        }  // the inbox was successfully loaded or generated.
    }  // By the time we enter this block, accounts and nyms are already loaded.
       // As we begin, inboxes are instantiated.

    // Either way, Cron should save, since it just updated.
    // The above function call WILL change this smart contract
    // and re-sign it and save it, no matter what. So I just
    // call this here to keep it simple:

    pCron->SaveCron();  // TODO No need to call this here if I can make sure
                        // it's being called higher up somewhere
    // (Imagine a script that has 10 account moves in it -- maybe don't need to
    // save cron until
    // after all 10 are done. Or maybe DO need to do in between. Todo research
    // this. Optimization.)
    return bSuccess;
}

// Higher level. Can be executed from inside scripts.
//
// Returns success if funds were moved.
// This function does not run any scripts, but it CAN be executed from within
// the scripts.
// Any movement of funds to-or-from any account will automatically try to
// load/use the
// appropriate authorizing agent for that account (or use him, if he's already
// loaded on
// this smart contract.)
//

bool OTSmartContract::MoveAcctFundsStr(
    std::string from_acct_name,
    std::string to_acct_name,
    std::string str_Amount)
{
    OTCron* pCron = GetCron();
    OT_ASSERT(nullptr != pCron);

    Nym* pServerNym = pCron->GetServerNym();
    OT_ASSERT(nullptr != pServerNym);

    // Below this point, these are all good:
    //
    //        pServerNym, pCron.

    if (str_Amount.size() < 1) {
        otOut << "OTSmartContract::MoveAcctFunds: Error: empty amount.\n";
        return false;
    }

    const std::int64_t lAmount = String::StringToLong(str_Amount.c_str());

    if (lAmount <= 0) {
        otOut << "OTSmartContract::MoveAcctFunds: Error: lAmount cannot be 0 "
                 "or <0. (Value passed in was "
              << lAmount << ".)\n";
        return false;
    }

    if (from_acct_name.size() <= 0) {
        otErr << "OTSmartContract::MoveAcctFunds: error: from_acct_name is "
                 "non-existent.\n";
        return false;
    }
    if (to_acct_name.size() <= 0) {
        otErr << "OTSmartContract::MoveAcctFunds: error: to_acct_name is "
                 "non-existent.\n";
        return false;
    }

    // Below this point, these are all good:
    //
    //        from_acct_name,
    //        to_acct_name,
    //        pServerNym, pCron.

    OTPartyAccount* pFromAcct = GetPartyAccount(from_acct_name);
    OTPartyAccount* pToAcct = GetPartyAccount(to_acct_name);

    if (nullptr == pFromAcct) {
        otOut << "OTSmartContract::MoveAcctFunds: error: from_acct ("
              << from_acct_name << ") not found on any party.\n";
        otOut << "FULL CONTRACT:  \n" << m_xmlUnsigned << " \n\n";
        return false;
    }
    if (nullptr == pToAcct) {
        otOut << "OTSmartContract::MoveAcctFunds: error: to_acct ("
              << to_acct_name << ") not found on any party.\n";
        otOut << "FULL CONTRACT:  \n" << m_xmlUnsigned << " \n\n";

        return false;
    }

    // Below this point, these are all good:
    //
    //        pFromAcct,    from_acct_name,
    //        pToAcct,    to_acct_name,
    //        pServerNym, pCron.

    OTAgent* pFromAgent =
        pFromAcct->GetAuthorizedAgent();  // This searches the account's party
                                          // for the account's authorized agent.
    OTAgent* pToAgent =
        pToAcct->GetAuthorizedAgent();  // (That way it's impossible to get an
                                        // agent for any other party.)

    if (nullptr == pFromAgent) {
        otOut << "OTSmartContract::MoveAcctFunds: error: authorized agent ("
              << pFromAcct->GetAgentName() << ") not found for from_acct ("
              << from_acct_name << ") on acct's party.\n";
        return false;
    }
    if (nullptr == pToAgent) {
        otOut << "OTSmartContract::MoveAcctFunds: error: authorized agent ("
              << pToAcct->GetAgentName() << ") not found for to_acct ("
              << to_acct_name << ") on acct's party.\n";
        return false;
    }

    if (!pFromAgent->IsAnIndividual()) {
        otOut << "OTSmartContract::MoveAcctFunds: error: authorized agent ("
              << pFromAcct->GetAgentName() << ") for from_acct ("
              << from_acct_name << ") is not an active agent.\n";
        return false;
    }
    if (!pToAgent->IsAnIndividual()) {
        otOut << "OTSmartContract::MoveAcctFunds: error: authorized agent ("
              << pToAcct->GetAgentName() << ") for to_acct (" << to_acct_name
              << ") is not an active agent.\n";
        return false;
    }
    //
    // Below this point, these are all good:
    //
    //        pFromAcct,    from_acct_name,    pFromAgent,
    //        pToAcct,    to_acct_name,    pToAgent,
    //        pServerNym, pCron.

    OTParty* pFromParty = pFromAgent->GetParty();
    OTParty* pToParty = pToAgent->GetParty();

    if (nullptr == pFromParty) {
        otErr << "OTSmartContract::MoveAcctFunds: error: Party pointer nullptr "
                 "on "
                 "authorized agent ("
              << pFromAcct->GetAgentName() << ") for from_acct ("
              << from_acct_name << ").\n";
        return false;
    }
    if (nullptr == pToParty) {
        otErr << "OTSmartContract::MoveAcctFunds: error: Party pointer nullptr "
                 "on "
                 "authorized agent ("
              << pToAcct->GetAgentName() << ") for to_acct (" << to_acct_name
              << ").\n";
        return false;
    }
    //
    // Below this point, these are all good:
    //
    //        pFromAcct,    from_acct_name,    pFromAgent,    pFromParty,
    //        pToAcct,    to_acct_name,    pToAgent,    pToParty,
    //        pServerNym, pCron.

    // Done: I can see that THIS VERIFICATION CODE WILL GET CALLED EVERY SINGLE
    // TIME THE SCRIPT
    // CALLS MOVE FUNDS.  Maybe that's good, but since technically this only
    // needs to be verified before the
    // first call, and not for EVERY call during any of a script's runs, I
    // should probably move this verification
    // higher, such as each time the OTCronItem triggers, plus each time a party
    // triggers a clause directly
    // through the API (server message). As long as those are covered, I will be
    // able to remove it from here
    // which should be a significant improvement for performance.
    // It will be at the bottom of those same functions that
    // "ClearTemporaryPointers()" should finally be called.
    //
    // FINAL DECISION: Redundant.  See comment in
    // OTSmartContract::StashAcctFunds()
    //
    // A party might have many agents who are only voting groups, and cannot
    // actually sign for things
    // the way that nyms can. But at least ONE of those agents IS a Nym --
    // because there must have been
    // an authorizing agent who initially signed to accept the agreement, and
    // who fronted the opening
    // transaction number that activated it.
    //
    // Similarly, the authorized agent for any given party's account (each
    // account has its own authorized
    // agent) MUST be an active agent (an active agent is one with a
    // Nym--whether that Nym is representing
    // himself as the party, or whether representing some entity as an employee
    // in a role). Why MUST the
    // authorized agent be an active agent? Because when funds are moved, that
    // Nym must be loaded since
    // the account must show that Nym as a legal owner/agent. The MoveFunds will
    // cause a paymentReceipt to
    // drop into the Inbox for the relevant asset accounts, and that
    // paymentReceipt can ONLY be accepted
    // by that same Nym, who must use a transaction # that he signed for
    // previously and received through
    // his nymbox. There is actually no justification at all to take funds from
    // that account, since the
    // new balance has not yet been signed, UNLESS THE PAYMENTRECEIPT CONTAINS A
    // VALID, SIGNED AUTHORIZATION
    // FROM THE ACCOUNT HOLDER. *That* is why the authorizing agent must either
    // be the Party's Owner himself
    // (representing himself as an agent, which most will do) in which case he
    // will appear as the valid
    // owner of the account, OR he MUST be a Nym working in a Valid Role for an
    // Entity, where said Entity is
    // the valid owner on the account in question. Either OT, it will be
    // possible in OT for him to sign for
    // the paymentReceipts when they come in, and impossible for him to escape
    // liability for them.
    // (That's the idea anyway.)
    //
    // Since we know that the Authorized Agent for an account must be an ACTIVE
    // agent (one way or the other)
    // then we can error out here if he's not.  We can then pass in his Nym ID
    //

    auto theFromAgentID = Identifier::Factory(),
         theToAgentID = Identifier::Factory();
    const bool bFromAgentID = pFromAgent->GetSignerID(theFromAgentID);
    const bool bToAgentID = pToAgent->GetSignerID(theToAgentID);

    if (!bFromAgentID) {
        otErr << "OTSmartContract::MoveAcctFunds: Failed to find FromAgent's "
                 "Signer ID: "
              << pFromAgent->GetName() << " \n";
        return false;
    }
    if (!bToAgentID) {
        otErr << "OTSmartContract::MoveAcctFunds: Failed to find ToAgent's "
                 "Signer ID: "
              << pToAgent->GetName() << " \n";
        return false;
    }

    if (!pFromAcct->GetAcctID().Exists()) {
        otErr << "OTSmartContract::MoveAcctFunds: Error: FromAcct has empty "
                 "AcctID: "
              << from_acct_name << " \n";
        return false;
    }
    if (!pToAcct->GetAcctID().Exists()) {
        otErr << "OTSmartContract::MoveAcctFunds: Error: ToAcct has empty "
                 "AcctID: "
              << to_acct_name << " \n";
        return false;
    }

    const auto theFromAcctID = Identifier::Factory(pFromAcct->GetAcctID()),
               theToAcctID = Identifier::Factory(pToAcct->GetAcctID());
    //
    // BELOW THIS POINT, theFromAcctID, theFromAgentID, theToAcctID, and
    // theToAgentID are all available.

    // WE SET THESE HERE SO THE RECEIPT SHOWS, SUCCESS OR FAIL,
    // WHO THE INTENDED SENDER / RECIPIENT ARE FOR THAT RECEIPT.
    //
    ReleaseLastSenderRecipientIDs();

    theFromAgentID->GetString(m_strLastSenderUser);  // This is the last Nym ID
    // of a party who SENT money.
    theFromAcctID->GetString(
        m_strLastSenderAcct);  // This is the last Acct ID of
                               // a party who SENT money.
    theToAgentID->GetString(m_strLastRecipientUser);  // This is the last Nym ID
                                                      // of a party who RECEIVED
                                                      // money.
    theToAcctID->GetString(m_strLastRecipientAcct);  // This is the last Acct ID
                                                     // of a party who RECEIVED
                                                     // money.

    mapOfConstNyms map_Nyms_Already_Loaded;
    RetrieveNymPointers(map_Nyms_Already_Loaded);

    bool bMoved = MoveFunds(
        map_Nyms_Already_Loaded,
        lAmount,
        theFromAcctID,
        theFromAgentID,
        theToAcctID,
        theToAgentID);
    if (!bMoved) {
        otOut << "OTSmartContract::MoveAcctFunds: Failed in call to MoveFunds. "
                 "from_acct: "
              << from_acct_name << "   to_acct: " << to_acct_name << "\n";
        return false;
    }

    return true;
}

// This is called by OTCronItem::HookRemovalFromCron
//
// (After calling this method, HookRemovalFromCron then calls
// onRemovalFromCron.)
void OTSmartContract::onFinalReceipt(
    OTCronItem& theOrigCronItem,
    const std::int64_t& lNewTransactionNumber,
    Nym& theOriginator,
    Nym* pActingNym)  // AKA "pRemover" in any other onFinalReceipt. Could be
                      // nullptr.
{
    OTCron* pCron = GetCron();

    OT_ASSERT(nullptr != pCron);

    Nym* pServerNym = pCron->GetServerNym();

    OT_ASSERT(nullptr != pServerNym);

    const String strNotaryID(GetNotaryID());

    // The finalReceipt Item's ATTACHMENT contains the UPDATED Cron Item.
    // (With the SERVER's signature on it!)
    String strUpdatedCronItem(*this);
    String* pstrAttachment = &strUpdatedCronItem;
    const String strOrigCronItem(theOrigCronItem);

    // IF server is originator and/or remover then swap it in for it/them so I
    // don't load it twice. (already handled before this function is called.)

    // THIS FUNCTION:
    //
    // LOOP through all parties.
    // For each party:
    // If party is server or originator or ActingNym, etc then set pointer
    // appropriately for that party. Find opening and closing numbers for that
    // party. Drop finalReceipt to Inboxes for each asset account, using closing
    // numbers. Drop finalReceipt to Nymbox for that party, using opening
    // number.
    //
    // A similar process should happen whenever ANY contract action occurs. (Not
    // just finalReceipt) We loop through all the parties and give them a
    // receipt in the relevant accounts. And perhaps all notices should be
    // numbered (similar to request number) so that people can prove which
    // notices they have received. Receipts are given based on? The asset
    // accounts that are CHANGED should definitely get an agreementReceipt for
    // the balance change.  + All Nymboxes should receive a notice at that time.
    // They should receive additional notice for any change in any variable as
    // well. Maybe let parties register for various notices. What about if a
    // clause processes, but no asset accounts are changed, (no inbox notice)
    // and no other variables are changed (no nymbox notices at all...) In that
    // case, no other receipts are dropped, right? There will be some standard
    // by which DIRTY flags are set onto the various parties and asset accounts,
    // and then notices will be sent based upon those.
    //
    // For those, instead of:
    // "theOriginator" (GetSenderNymID()) and "pRemover" and pRecipient,
    //
    // We would have:
    // "theOriginator" (GetSenderNymID()) and "pActingNym" and pParty /
    // pPartyNym (for Party[0..n])
    //
    // Just like here:

    for (auto& it : m_mapParties) {
        OTParty* pParty = it.second;

        OT_ASSERT_MSG(
            nullptr != pParty, "Unexpected nullptr pointer in party map.");

        // The Nym who is actively requesting to remove a cron item will be
        // passed in as pActingNym. However, sometimes there is no Nym...
        // perhaps it just expired and pActingNym is nullptr. The originating
        // Nym (if different than pActingNym) is loaded up. Otherwise
        // theOriginator just points to *pActingNym also.
        Nym* pPartyNym = nullptr;
        std::unique_ptr<Nym> thePartyNymAngel;

        // See if the serverNym is an agent on this party.

        // This should set the temp nym ptr inside the agent also, so I don't
        // have to search twice.
        if (pParty->HasAuthorizingAgent(*pServerNym)) {
            // Just in case the party's agent's Nym is also the server Nym.
            pPartyNym = pServerNym;
        }
        // If pActingNym is NOT nullptr, and HE is an agent on this party...
        // then set the pointer accordingly.
        else if (
            (nullptr != pActingNym) &&
            pParty->HasAuthorizingAgent(*pActingNym))  // There is only one
                                                       // authorizing agent
                                                       // per party.
        {
            // now both pointers are set (to same Nym). DONE!
            pPartyNym = pActingNym;
        }

        // Still not found?
        if (nullptr == pPartyNym) {
            // Of all of a party's Agents, the "authorizing agent" is the one
            // who originally activated the agreement for this party (and
            // fronted the opening trans#.) If we're ending the agreement, Then
            // we need to free that number from him. (Even if he was since fired
            // from the role!)
            //
            // Perhaps need to figure out if the Role itself stores the opening
            // number, and if so, treat the Nym's signature as the role's, even
            // though the Nym himself doesn't actually store the #. Anyway, I'll
            // deal with that when I get to entities and roles.
            // TODO.
            //
            pPartyNym = pParty->LoadAuthorizingAgentNym(*pServerNym);
            thePartyNymAngel.reset(pPartyNym);
        }

        // Every party SHOULD have an authorizing agent (otherwise how did that
        // party sign on in the first place??) So this should never fail.

        OT_ASSERT(nullptr != pPartyNym);

        auto context = OT::App().Wallet().mutable_ClientContext(
            GetNotaryID(), pPartyNym->ID());
        const auto opening = pParty->GetOpeningTransNo();
        const bool haveOpening = pParty->GetOpeningTransNo() > 0;
        const bool issuedOpening = context.It().VerifyIssuedNumber(opening);
        const bool validOpening = haveOpening && issuedOpening;

        // TODO: once entities and roles are added, Parties should have their
        // OWN "verify" function (Instead of me having to directly find the Nym
        // and verify it myself.)
        if (validOpening) {
            // The Nym (server side) stores a list of all opening and closing
            // cron #s. So when the number is released from the Nym, we also
            // take it off that list.
            context.It().CloseCronItem(opening);

            // the RemoveIssued call means the original transaction# (to find
            // this cron item on cron) is now CLOSED. But the Transaction itself
            // is still OPEN. How? Because the CLOSING number is still signed
            // out. The closing number is also USED, since the smart contract
            // was initially activated, but it remains ISSUED, until the final
            // receipt itself is accepted during a process inbox.
            context.It().ConsumeIssued(opening);
            pPartyNym->SaveSignedNymfile(*pServerNym);
        } else {
            otErr
                << "OTSmartContract::" << __FUNCTION__
                << ": Failed verifying pParty->GetOpeningTransNo() > 0 && "
                << "pPartyNym->VerifyIssuedNum(pParty->GetOpeningTransNo())\n";
        }

        // NOTIFY ALL AGENTS for this party, with a copy of the finalReceipt in
        // their Nymbox.
        //
        // TODO: if the above block fails, should I still go dropping these
        // receipts?
        if ((!pParty->DropFinalReceiptToNymboxes(
                lNewTransactionNumber,  // new, owned by the server. For notices
                strOrigCronItem,
                nullptr,
                pstrAttachment,
                pPartyNym))) {
            otErr << "OTSmartContract::" << __FUNCTION__
                  << ": Failure dropping final receipt into nymbox for even a "
                     "single agent.\n";
        }

        // So the same Nym doesn't get loaded twice on accident. (We pass in
        // pointers to nyms that are already loaded, so the called function can
        // use them instead of loading, if it came to that.)
        mapOfNyms nym_map;

        // pServerNym
        {
            const auto theServerNymID = Identifier::Factory(*pServerNym);
            const String strServerNymID(theServerNymID);

            if (nym_map.end() == nym_map.find(strServerNymID.Get())) {
                nym_map.insert(std::pair<std::string, Nym*>(
                    strServerNymID.Get(), pServerNym));
            }
        }

        // theOriginator
        {
            const auto theOriginatorNymID = Identifier::Factory(theOriginator);
            const String strOriginatorNymID(theOriginatorNymID);

            if (nym_map.end() == nym_map.find(strOriginatorNymID.Get())) {
                nym_map.insert(std::pair<std::string, Nym*>(
                    strOriginatorNymID.Get(), &theOriginator));
            }
        }

        if (nullptr != pActingNym) {
            const auto theActingNymID = Identifier::Factory(*pActingNym);
            const String strActingNymID(theActingNymID);

            if (nym_map.end() == nym_map.find(strActingNymID.Get())) {
                nym_map.insert(std::pair<std::string, Nym*>(
                    strActingNymID.Get(), pActingNym));
            }
        }

        if (nullptr != pPartyNym) {
            const auto thePartyNymID = Identifier::Factory(*pPartyNym);
            const String strPartyNymID(thePartyNymID);

            if (nym_map.end() == nym_map.find(strPartyNymID.Get())) {
                nym_map.insert(std::pair<std::string, Nym*>(
                    strPartyNymID.Get(), pPartyNym));
            }
        }

        // NOTIFY the agent for EACH ACCOUNT listed by this party, with a copy
        // of the finalReceipt in the Inbox for each asset acct.
        //
        // Also for each, if he has a Nym (HE SHOULD), and if
        // (CLOSING_NUMBER_HERE > 0), then call:
        //
        // pNym->VerifyIssuedNum(strNotaryID, lClosingNumber)
        // (This happens in OTAgent::DropFinalReceipt, FYI.)
        if (!pParty->DropFinalReceiptToInboxes(
                &nym_map,  // contains any Nyms who might already be
                           // loaded, mapped by ID.
                strNotaryID,
                *pServerNym,
                lNewTransactionNumber,
                strOrigCronItem,
                nullptr,
                pstrAttachment)) {
            otErr << "OTSmartContract::onFinalReceipt: Failure dropping final "
                     "receipt into all inboxes. (Missed at least one.)\n";
        }

        pParty->ClearTemporaryPointers();
    }
}

// OTCron calls this regularly, which is my chance to expire, etc.
// Return True if I should stay on the Cron list for more processing.
// Return False if I should be removed and deleted.
bool OTSmartContract::ProcessCron()
{
    OT_ASSERT(nullptr != GetCron());

    // Right now Cron is called 10 times per second.
    // I'm going to slow down all trades so they are once every
    // GetProcessInterval()
    // Todo: Create separate lists in Cron.  10*/sec list, 1/second list, 1 min
    // list, 1 hour, 1 day, 1 month.
    // That way I'm not looping through ALL cron items 10*/second, but only the
    // ones who are paying for those
    // kinds of resources. (Different lists will cost different server fees.)
    //
    if (GetLastProcessDate() > OT_TIME_ZERO) {
        // Default ProcessInternal is 1 second, but Trades will use 10 seconds,
        // and Payment
        // Plans will use an hour or day. Smart contracts are currently 30
        // seconds. (For testing.)
        //
        if (OTTimeGetTimeInterval(
                OTTimeGetCurrentTime(), GetLastProcessDate()) <=
            GetProcessInterval())
            return true;
    }
    // Keep a record of the last time this was processed.
    // (NOT saved to storage, only used while the software is running.)
    // (Thus no need to release signatures, sign contract, save contract, etc.)
    SetLastProcessDate(OTTimeGetCurrentTime());

    // END DATE --------------------------------
    // First call the parent's version (which this overrides) so it has
    // a chance to check its stuff.
    // Currently it calls IsExpired().
    //
    if (!ot_super::ProcessCron()) {
        otLog3 << "Cron job has expired.\n";
        return false;  // It's expired or flagged for removal--remove it from
                       // Cron.
    }

    // START DATE --------------------------------
    // Okay, so it's not expired. But might not have reached START DATE yet...
    if (!VerifyCurrentDate())
        return true;  // The Payment Plan is not yet valid, so we return. BUT,
                      // we also
    // return TRUE, so it will STAY on Cron until it BECOMES valid.

    // Make sure there are transaction numbers available in Cron.
    // (Can't do anything without those....)
    //
    if (GetCron()->GetTransactionCount() < 1) {
        otOut << "Failed to process smart contract: Cron is out of transaction "
                 "numbers!\n";
        return true;
    }

    // Make sure, if the script set a timer, that we don't process Cron until
    // that timer
    // is reached. (If the timer's not set, then we go ahead and process every
    // time.)
    //
    const time64_t& tNextProcessDate = GetNextProcessDate();

    if (tNextProcessDate > OT_TIME_ZERO)  // IF there is a timer set (as to when
                                          // the next "onProcess" should
                                          // occur)...
    {
        if (OTTimeGetCurrentTime() <=
            tNextProcessDate)  // AND if the current time has NOT YET reached
                               // that date (the date in the timer)...
        {
            // ...Then RETURN (since the timer hasn't popped yet)
            // But return TRUE, so that this cron item stays active for now.
            //
            return true;
        } else  // else it HAS now reached the official timer date...
        {
            SetNextProcessDate(OT_TIME_ZERO);  // Therefore timer has triggered,
            // so we will continue processing.
        }  // We also reset timer to 0 again since it has now "binged".
    }      // Continuing on....

    // Execute the scripts (clauses) that have registered for this hook.

    const std::string str_HookName(SMARTCONTRACT_HOOK_ON_PROCESS);
    mapOfClauses theMatchingClauses;

    if (GetHooks(str_HookName, theMatchingClauses)) {
        otOut << "Cron: Processing smart contract clauses for hook: "
              << SMARTCONTRACT_HOOK_ON_PROCESS << " \n";

        ExecuteClauses(
            theMatchingClauses);  // <============================================
    }

    if (IsFlaggedForRemoval()) {
        otLog3 << "OTSmartContract::ProcessCron: Removing smart contract from "
                  "cron processing...\n";
        return false;  // false means "remove this cron item from cron"
    }

    return true;
}

// virtual
void OTSmartContract::SetDisplayLabel(const std::string* pstrLabel)
{
    m_strLabel.Format(
        "smartcontract trans# %" PRId64 ", clause: %s",
        GetTransactionNum(),
        (nullptr != pstrLabel) ? pstrLabel->c_str() : "");
}

void OTSmartContract::ExecuteClauses(
    mapOfClauses& theClauses,
    String* pParam)  // someday
                     // pParam could
                     // be a
                     // stringMap
                     // instead of a
                     // single
                     // param.
{
    // Loop through the clauses passed in, and execute them all.
    for (auto& it_clauses : theClauses) {
        const std::string str_clause_name = it_clauses.first;
        OTClause* pClause = it_clauses.second;
        OT_ASSERT((nullptr != pClause) && (str_clause_name.size() > 0));
        OTBylaw* pBylaw = pClause->GetBylaw();
        OT_ASSERT(nullptr != pBylaw);

        // By this point, we have the clause we are executing as pClause,
        // and we have the Bylaw it belongs to, as pBylaw.

        const std::string str_code =
            pClause->GetCode();  // source code for the script.
        const std::string str_language =
            pBylaw->GetLanguage();  // language it's in. (Default is "chai")

        std::shared_ptr<OTScript> pScript =
            OTScriptFactory(str_language, str_code);

        std::unique_ptr<OTVariable> theVarAngel;

        //
        // SET UP THE NATIVE CALLS, REGISTER THE PARTIES, REGISTER THE
        // VARIABLES, AND EXECUTE THE SCRIPT.
        //
        if (pScript) {
            // Register the special server-side native OT calls we make
            // available to all scripts.
            //
            RegisterOTNativeCallsWithScript(*pScript);

            // Register all the parties with the script.
            //
            for (auto& it : m_mapParties) {
                const std::string str_party_name = it.first;
                OTParty* pParty = it.second;
                OT_ASSERT((nullptr != pParty) && (str_party_name.size() > 0));

                pScript->AddParty(str_party_name, *pParty);  // This also
                                                             // registers all of
                                                             // Party's accounts
                                                             // with pScript.
            }

            // Also need to loop through the Variables on pBylaw and register
            // those as well.
            //
            pBylaw->RegisterVariablesForExecution(
                *pScript);  // This also sets all the variables as CLEAN so we
                            // can check for dirtiness after execution.

            // A parameter might also be passed in, so we add that to the script
            // as well.
            // (Like if a client is sending a triggerClause message to a server,
            // and passing
            // a string parameter to that clause as input.)
            //
            const std::string str_Name("param_string");
            std::string str_Value("");

            // See if param_string variable is already found on the bylaw...
            //
            if (nullptr !=
                pBylaw->GetVariable(str_Name))  // disallow duplicate names.
            {
                otErr << "OTSmartContract::ExecuteClauses: While preparing to "
                         "run smartcontract trans# "
                      << GetTransactionNum() << ", clause: " << str_clause_name
                      << ".  Error: Parameter variable named " << str_Name
                      << " already exists. (Skipping the parameter actually "
                         "passed in.)\n";
            } else  // The param_string variable isn't already there. (So we add
                    // it as blank, if a value wasn't passed in.)
            {
                if (nullptr != pParam)  // if a param was passed in...
                    str_Value = pParam->Get();
                // else (it's already "")

                OTVariable* pVar = new OTVariable(
                    str_Name, str_Value, OTVariable::Var_Constant);
                OT_ASSERT(nullptr != pVar);
                theVarAngel.reset(pVar);

                // This causes pVar to keep a pointer to the script so it can
                // remove itself from the script upon destruction.
                pVar->RegisterForExecution(*pScript);
            }

            // TEMP FOR TESTING (HARDCODED CLAUSE NAME HERE...)
            //            OTVariable theReturnVal("return_val", false); //
            // initial value is: false.

            SetDisplayLabel(&str_clause_name);

            pScript->SetDisplayFilename(m_strLabel.Get());

            if (!pScript->ExecuteScript())  // If I passed theReturnVal
                                            // in here, then it'd be
                                            // assumed a bool is expected
                                            // to be returned inside it.
                                            //            if (false ==
            // pScript->ExecuteScript((str_clause_name.compare("process_clause")
            // == 0) ? &theReturnVal : nullptr))
            {
                otErr << "OTSmartContract::ExecuteClauses: Error while running "
                         "smartcontract trans# "
                      << GetTransactionNum() << ", clause: " << str_clause_name
                      << " \n\n";
            } else
                otOut << "OTSmartContract::ExecuteClauses: Success executing "
                         "smartcontract trans# "
                      << GetTransactionNum() << ", clause: " << str_clause_name
                      << " \n\n";

            //            For now, I've decided to allow ALL clauses to trigger
            // on the hook. The flag only matters after
            //            they are done, and not between scripts. Otherwise
            // problems could arise, such as order of execution.
            //            Remember, there is nothing stopping people from using
            // their own variables and ending all behavior
            //          after that flag is set.  Todo security: revisit this
            // just in case.
            //
        } else {
            otErr << "OTSmartContract::ExecuteClauses: Error instantiating "
                     "script!\n";
        }
    }

    // "Important" variables.
    // (If any of them have changed, then I need to notice the parties.)
    //
    // TODO: Fix IsDirtyImportant() so that it checks for changed STASHES
    // as well. (Or have another function to do it, which is also called here.)
    //
    // I'd like to get to where I can just call IsDirty() here, and then SAVE
    // CRON HERE,
    // so I'm not having to save it after EACH change, which is currently
    // occuring in the
    // StashAcctFunds / MoveAcctFunds functions. Todo.
    //
    if (IsDirtyImportant())  // This tells us if any "Important" variables
                             // have changed since executing the scripts.
    {
        OTCron* pCron = GetCron();
        OT_ASSERT(nullptr != pCron);

        Nym* pServerNym = pCron->GetServerNym();
        OT_ASSERT(nullptr != pServerNym);

        const std::int64_t lNewTransactionNumber =
            pCron->GetNextTransactionNumber();

        //        OT_ASSERT(lNewTransactionNumber > 0); // this can be my
        // reminder.
        if (0 == lNewTransactionNumber) {
            otErr << "OTSmartContract::ExecuteClauses: ** ERROR: Notice not "
                     "sent to parties, since no "
                     "transaction numbers were available!\n";
        } else {
            ReleaseSignatures();
            SignContract(*pServerNym);
            SaveContract();

            const String strReference(*this);
            bool bDroppedNotice = SendNoticeToAllParties(
                true,  // bSuccessMsg=true
                *pServerNym,
                GetNotaryID(),
                lNewTransactionNumber,
                strReference);  // pstrNote and pstrAttachment aren't used in
                                // this case.

            otOut << __FUNCTION__
                  << ": FYI, 'Important' variables were changed during the "
                     "execution of this script.\n"
                  << (bDroppedNotice ? "Success" : "Failure")
                  << " dropping notifications into all parties' nymboxes.\n";
        }
    }
}

// The server calls this when it wants to know if a certain party is allowed to
// cancel
// the entire contract (remove it from Cron).
// This function tries to answer that question by checking for a callback script
// called:
//                                callback_party_may_cancel_contract
// If the callback script exists, then it calls that for the answer. Otherwise
// the default
// return value is: true  (as long as he's a legitimate party.)
// Script coders may also call "party_may_cancel_contract()" from within a
// script, which
// will call this function, which will trigger the script
// callback_party_may_cancel_contract(),
// etc.
//
bool OTSmartContract::CanCancelContract(std::string str_party_name)
{
    OTCron* pCron = GetCron();
    OT_ASSERT(nullptr != pCron);

    Nym* pServerNym = pCron->GetServerNym();
    OT_ASSERT(nullptr != pServerNym);

    OTParty* pParty = GetParty(str_party_name);

    if (nullptr == pParty) {
        otOut
            << "OTSmartContract::CanCancelContract: Unable to find this party: "
            << str_party_name << "\n";
        return false;
    }
    // Below this point, pParty is good.

    // ...This WILL check to see if pParty has its Opening number verified as
    // issued.
    // (If the opening number is > 0 then VerifyPartyAuthorization() is smart
    // enough to verify it.)
    //
    // To KNOW that a party has the right to even ASK the script to cancel a
    // contract, MEANS that
    // (1) The party is listed as a party on the contract. (2) The party's copy
    // of that contract
    // is signed by the authorizing agent for that party. and (3) The opening
    // transaction number for
    // that party is verified as issued for authorizing agent. (2 and 3 are both
    // performed at the same
    // time, in VerifyPartyAuthorization(), since the agent may need to be
    // loaded in order to verify
    // them.) 1 is already done by this point, as it's performed above.
    //
    // Done: notice this code appears in CanCancelContract() (this function) as
    // well as
    // OTScriptable::CanExecuteClause.
    // Therefore I can see that THIS VERIFICATION CODE WILL GET CALLED EVERY
    // SINGLE TIME THE SCRIPT
    // CALLS ANY CLAUSE OR OT NATIVE FUNCTION.  Since technically this only
    // needs to be verified before the
    // first call, and not for EVERY call during any of a script's runs, I
    // should probably move this verification
    // higher, such as each time the OTCronItem triggers, plus each time a party
    // triggers a clause directly
    // through the API (server message). As long as those are covered, I will be
    // able to remove it from here
    // which should be a significant improvement for performance.
    // It will be at the bottom of those same functions that
    // "ClearTemporaryPointers()" should finally be called.
    //
    // Also todo:  Need to implement MOVE CONSTRUCTORS and MOVE COPY
    // CONSTRUCTORS all over the place,
    // once I'm sure C++0x build environments are available for all of the
    // various OT platforms. That should
    // be another great performance boost!
    //
    //    FINAL DECISION: Redundant. See comment in
    // OTSmartContract::StashAcctFunds()
    //

    // IF NO CALLBACK IS PROVIDED, The default answer to this function is:
    //     YES, this party MAY cancel this contract! (Assuming he's a real
    // party,
    //     which we have verified by this point.)
    //
    // But... first we check to see if this OTScriptable has a clause named:
    //          "callback_party_may_cancel_contract"
    // ...and if so, we ask the CALLBACK to make the decision instead. This way,
    // people can define
    // in their own scripts any rules they want about which parties may cancel
    // the contract.

    //
    const std::string str_CallbackName(SMARTCONTRACT_CALLBACK_PARTY_MAY_CANCEL);

    OTClause* pCallbackClause =
        GetCallback(str_CallbackName);  // See if there is a script clause
                                        // registered for this callback.

    if (nullptr !=
        pCallbackClause)  // Found it! There's a clause registered for
                          // this callback. Let's call it...
    {
        otOut << "OTSmartContract::CanCancelContract: Found script for: "
              << SMARTCONTRACT_CALLBACK_PARTY_MAY_CANCEL << ". Asking...\n";

        // The function we're IN defaults to TRUE, if there's no script
        // available.
        // However, if the script IS available, then our default return value
        // starts as FALSE.
        // (The script itself will then have to set it to true, if that's what
        // it wants.)
        //
        OTVariable theReturnVal("return_val", false);

        OTVariable param1(
            "param_party_name",
            str_party_name,
            OTVariable::Var_Constant);  // script can reference param_party_name

        mapOfVariables theParameters;
        theParameters.insert(
            std::pair<std::string, OTVariable*>("param_party_name", &param1));

        if (false ==
            ExecuteCallback(
                *pCallbackClause,
                theParameters,
                theReturnVal))  // <============================================
        {
            otErr << "OTSmartContract::CanCancelContract: Error while running "
                     "callback script "
                  << SMARTCONTRACT_CALLBACK_PARTY_MAY_CANCEL << ", clause "
                  << pCallbackClause->GetName() << " \n";
            return false;
        } else {
            otOut << "OTSmartContract::CanCancelContract: Success executing "
                     "callback script "
                  << SMARTCONTRACT_CALLBACK_PARTY_MAY_CANCEL
                  << ", clause: " << pCallbackClause->GetName() << ".\n\n";

            return theReturnVal.CopyValueBool();
        }

    } else {
        otOut
            << "OTSmartContract::CanCancelContract: Unable to find script for: "
            << SMARTCONTRACT_CALLBACK_PARTY_MAY_CANCEL
            << ". Therefore, default return value is: TRUE.\n";
    }

    return true;
}

/// See if theNym has rights to remove this item from Cron.
///
bool OTSmartContract::CanRemoveItemFromCron(const ClientContext& context)
{
    // You don't just go willy-nilly and remove a cron item from a market unless
    // you check first and make sure the Nym who requested it actually has said
    // number (or a related closing number) signed out to him on his last
    // receipt...
    //
    // Note: overrode parent method and NOT calling it. We do it our own way
    // here, and call a script if it's available.

    // IT'S ASSUMED that the opening and closing numbers WILL be verified in
    // order to insure they are CURRENTLY ISSUED.
    //
    // theNym.VerifyIssuedNum(strNotaryID, GetOpeningNum();
    // theNym.VerifyIssuedNum(strNotaryID, GetClosingNum();
    //
    // The default version OTCronItem does this for theNym, and the PaymentPlan
    // version has to be a little smarter: it has to figure out whether theNym
    // is the Sender or Recipient, so that it knows where to verify the numbers
    // from, before allowing theNym
    // to do the removal.
    //
    //
    // ===> THIS version (OTSmartContract) will look up pParty using theNym via:
    // OTParty * OTScriptable::FindPartyBasedOnNymAsAgent(const OTPseudonym&
    // theNym, OTAgent ** ppAgent=nullptr);
    //
    // ...Then it WILL check to see if pParty has its Opening number verified as
    // issued.
    // ...It COULD ALSO loop the partyaccounts and see if pAgent is authorized
    // agent for any of them.
    //    (If so, pAcct->VerifyClosingNumber() or pAgent->VerifyClosingNumber()
    // as well.)
    //
    //
    OTAgent* pAgent = nullptr;
    // This sets a pointer to theNym inside pAgent, so pParty can use it later.
    OTParty* pParty = FindPartyBasedOnNymAsAgent(*context.Nym(), &pAgent);

    if (nullptr == pParty) {
        otOut << "OTSmartContract::CanRemoveItemFromCron: Warning: theNym is "
                 "not an agent "
                 "for any party to this contract, yet tried to remove it.\n";
        return false;
    }

    OT_ASSERT(nullptr != pAgent);  // With one comes the other.

    // Below this point, pAgent is not only good, but it contains a secret
    // hidden pointer now to theNym. That way, when the SCRIPT asks the party to
    // verify issued number, without even having a reference to theNym, the
    // party will internally still be able to handle it. This always works in
    // cases where it's needed because we used theNym to look up pParty, and the
    // lookup function is what sets that pointer. That's why I clean the pointer
    // again after I'm done. (AT THE BOTTOM OF THIS FUNCTION.)
    //
    // NOTE: You can see OTCronItem looks up the relevant numbers by trying to
    // figure out if theNym is sender or receiver, and then calling these
    // methods:
    // if (GetCountClosingNumbers() < 1)
    // if (GetRecipientCountClosingNumbers() < 2)
    // Etc.
    //
    // But OTSmartContract doesn't use those functions, except where it has to
    // in order to work within the existing OTCronItem system. (That is, the
    // ORIGINATOR who actually activates a smart contract must still provide at
    // least an opening number, which is stored in the old system and used by
    // it.) Instead, OTSmartContract keeps its own records (via its parent class
    // OTScriptable) of all the parties to the contract, and all of their
    // opening transaction #s, as well as the accounts that are party to the
    // contract, and the closing transaction #s for each of those.
    //
    // ===> Therefore, when it comes to verifying whether the Nym has CERTAIN
    // RIGHTS regarding the contract, OTSmartContract doesn't actually use the
    // old system for that, but instead queries its own, superior system.
    //
    // In order to prevent infinite recursion I think I will be adding THAT code
    // into:
    //      OTSmartContract::CanCancelContract(party_name)

    bool bReturnValue = false;
    bool bPartyHasName = false;
    const std::string str_party_name = pParty->GetPartyName(&bPartyHasName);

    if (bPartyHasName &&
        CanCancelContract(str_party_name))  // Here is where it calls the
                                            // script, inside this call.
    {
        otOut << "OTSmartContract::CanRemoveItemFromCron: Looks like theNym "
                 "represents a party ("
              << str_party_name
              << ") and IS allowed by this contract to cancel it whenever he "
                 "chooses.\n";
        bReturnValue = true;
    }

    pParty->ClearTemporaryPointers();  // FindPartyBasedOnNymAsAgent() set the
                                       // party's agent's nym pointer to theNym.
                                       // This clears it.

    return bReturnValue;
}

// Server-side, need to verify ALL parties upon activation.
// Client-side, need to verify CURRENT list of parties before "the next party"
// signs on.
// May not be able to tell the difference, in code. I can verify that the
// present ones are good,
// but I can't guarantee that others aren't referenced in the script code,
// without some kind of
// "trial run" of the script, or parsing of it.
//
// Therefore no point verifying originator here -- let's verify it here for ANY
// of the parties, and
// let the server explicitly verify the trans#s instead of relying on this
// function to do it. That way
// I can use this function even in cases where I'm not able to verify the
// trans#s (such as on the
// client side.) SERVER WILL STILL NEED TO VERIFY those numbers....
//
// Server will also want to verify that originator IS a party (this function
// won't do it.)
//
// bool OTSmartContract::VerifySmartContract(OTPseudonym& theNym)
//{
// Need to verify:
//
// 1) That the opening/closing trans# on this CronItem match ONE of the parties.
// (Maybe not verifier.)
//    (Parties to trades and payments each have their own opening numbers.
// Therefore you can with scripts. But only one can activate.)
//    With trades, each Nym has their own cron item and #. With payment plans,
// there is only one cron item, and the sender is the
//    activator. Since he is the one paying, the number used is his. The other
// guy still gets receipts, but the code is smart
//    enough to create his receipts using HIS opening number, which he still has
// to provide up front. (Hmm.. in implementation that's not true...)
// Anyway, continuing: But those receipts contain
//    COPIES of the original cron item that was ACTIVATED by the sender, and has
// his trans# on it.
//    Still: the cron item is saved to storage under a specific number, right?
// Scripts must be smart enough to drop a receipt for
//    each party where the Opening Number comes from THAT PARTY, and where a
// closing number comes from one of his accts.
//
// 2) Verify this details against ALL parties copies.
//
// 3) If an optional list of Nyms is passed in, then verify all their signatures
// as well. hmm.
//
// May not even need this function. Could be the new ones above are enough to
// cover it all.
//

// ABOVE Makes sure that all parties' signed copies are equivalent to my own
// (*this)
//
// Once this is established, then BELOW verifies all the data on (*this) against
// the actual Nyms passed in.
//
// Finally, the signature for each is verified against his own copy.
//

// For smart contract:
//
// 1) Assume verifier could be any one of the parties, not necessarily the
// originator

// TO WORK WITHIN THE EXISTING CRON SYSTEM, the originator's Opening and Closing
// #s must be set
// and available onto THIS CRON ITEM, so that GetTransactionNum() is the opening
// and GetCountClosingNUmbers()
// contains at least 1 number. Thus: if originator does not have an asset
// account in his party, we fail to activate!!
// Originator MUST have an asset account...
// Why? Who cares about the closing numbers except for custom code for specific
// things like trades? We only REALLY
// care about the closing number when we need to put it into an asset account's
// inbox as a finalReceipt. AHHH But
// the server DOES drop copies of all finalReceipts into your NYMBOX as well, as
// a NOTICE, so you can get the news faster.
// And the SAME NUMBER is put onto that receipt, which you receive in your
// Nymbox even if you HAVE NO asset account.
// Perhaps you should provide one for your Nym AND for all your accounts.  That
// way your Nym can get a copy of all those
// notices, but even without any asset accounts, he STILL gets a notice
// onFinalReceipt with his own special number on it.
//
// THE ONLY DIFFERENCE IS:  With inbox finalReceipts, the closing number stays
// open until you process your inbox to accept it.
// You receive a notice of that same receipt in your Nymbox already, so that you
// know to stop using your OPENING number. This is
// because it's possible for it to appear out of the blue, and your transactions
// for all accts would stop working unless you knew
// which inbox to examine for the finalReceipt that must have appeared. So it
// goes into Nymbox so you only have to check one place.
// WHEREAS WITH SMART CONTRACT NYMBOX NOTICES, the Nymbox notice can also
// contain a CLOSING # for that NYM ITSELF. So it sees
// when the script has ended. HMM.  Why not have the Nym's Opening # be verified
// to start, and then that SAME # go into the Nym's
// Nymbox on the finalReceipt? The Nym can then close it out on his side (thanks
// to that notice) the same as it does now with other cron items.
// The "closing #" that would also be on the nymbox notice is unneeded in this
// case, since there are no asset accounts, and the Nym just needed
// notice that his opening number was free again.
//
// Therefore, to work within existing cron system, it should be easy to simply
// add the Opening Number for originator, AND
// to set the Closing Number on the Cron Item to be the originator's opening
// number, in cases where he has no asset accounts.
// I'm curious where that may ever even be needed -- I bet it's only there to
// provide a common location, since the other cron item
// types all happen to use it. I will endeavor to work within a paradigm where
// closing numbers are only needed for asset accounts
// and where Cron Items are still functional without them, for Nyms using
// contracts without asset accounts.
//
// UPDATE: in actual implementation, I resolved this with the simple requirement
// that the Nym who ACTIVATES a
// smart contract, must be the authorized agent for at least ONE account for his
// party, in that contract!  This
// simple requirement, which would probably be true anyway, in practice, insures
// that there are legitimate opening
// and closing transaction numbers available from the Nym who actually activates
// the contract.
// (That Nym, however, is still subject to the rules of the contract.)

// Here are my notes of what is needed here:
//
//    return true; // Success!
//}
//

// Old thoughts
// Note: agents will have restrictable permissions. Should be overridable in the
// role,
// in the agent itself (in the party), etc. Like a registered agent -- he can
// ONLY activate
// things (and sign for them...) After that, the passive mechanism of the group
// voting takes
// over for all future meetings/decisions.
// But someone must sign at first.  This can be a "registered agent" if you
// want, with limited
// authority, only allowed to activate.

// theNym is trying to activate the smart contract, and has
// supplied transaction numbers and a user/acct ID. ==> theNym definitely IS the
// owner of the account... that is
// verified in Server::NotarizeTransaction(), before it even knows what KIND
// of transaction it is processing!
// (For all transactions.) So by the time Server::NotarizeSmartContract() is
// called, we know that much.
//
// But for all other parties, we do not know this, so we still need to loop them
// all, etc to verify this crap,
// at least once. (And then maybe I can lessen some of the double-checking, for
// optimization purposes, once
// we've run this gamut.)
//
// One thing we still do not know, until VerifySmartContract is called, is
// whether theNym really IS a valid
// agent for this contract, and whether all the other agents are valid, and
// whether the accounts are validly
// owned by the agents they list, and whether the authorizing agent for each
// party has signed their own copy,
// and whether the authorizing agent for each party provided a valid opening
// number--which must be recorded
// as consumed--and whether the authorized agent for each account provided a
// valid closing number, which likewise
// must be recorded. (Set bBurnTransNo to true if you want to enforce the stuff
// about the opening and closing #s)
//
bool OTSmartContract::VerifySmartContract(
    Nym& theNym,
    Account& theAcct,
    Nym& theServerNym,
    bool bBurnTransNo)
{
    OTAgent* pAuthAgent = nullptr;
    OTParty* pAuthParty = FindPartyBasedOnNymAsAuthAgent(theNym, &pAuthAgent);

    if (nullptr == pAuthParty) {
        String strNymID;
        theNym.GetIdentifier(strNymID);
        otOut << __FUNCTION__
              << ": Unable to find a party in this smart contract, based "
                 "on theNym ("
              << strNymID << ") as authorizing agent.\n";
        return false;
    }
    OT_ASSERT(
        nullptr != pAuthAgent);  // If it found the party, then it DEFINITELY
                                 // should have set the agent pointer.
    // BELOW THIS POINT, pAuthAgent and pAuthParty and both good pointers.
    // Furthermore, we know that theNym
    // really is the authorizing agent for one of the parties to the contract.

    const String strNotaryID(
        GetNotaryID());  // the notaryID has already been verified by this time,
                         // in Server::NotarizeSmartContract()

    mapOfConstNyms map_Nyms_Already_Loaded;  // The list of Nyms that were
                                             // already instantiated before this
                                             // function was called.
    RetrieveNymPointers(map_Nyms_Already_Loaded);  // now theNym is on this
                                                   // map. (His party
                                                   // already has a pointer
                                                   // to him since he is
                                                   // the activator.)

    mapOfConstNyms map_Nyms_Loaded_In_This_Function;  // The total list of Nyms
                                                      // that were instantiated
                                                      // inside this function
                                                      // (and must be deleted.)

    mapOfAccounts map_Accts_Already_Loaded;  // The list of Accounts that were
                                             // already instantiated before this
                                             // function was called.
    const String strAcctID(theAcct.GetRealAccountID());
    map_Accts_Already_Loaded.insert(std::pair<std::string, Account*>(
        strAcctID.Get(), &theAcct));  // now theAcct is on this map.

    mapOfAccounts map_Accts_Loaded_In_This_Function;  // The total list of Accts
                                                      // that were instantiated
                                                      // inside this function
                                                      // (and must be deleted.)

    bool bAreAnyInvalidParties = false;
    std::set<OTParty*> theFailedParties;  // A set of pointers to parties who
                                          // failed verification.

    // LOOP THROUGH ALL PARTIES AND VERIFY THEM.
    for (auto& it_party : m_mapParties) {
        const std::string str_party_name = it_party.first;
        OTParty* pParty = it_party.second;
        OT_ASSERT_MSG(
            nullptr != pParty,
            "OTSmartContract::VerifySmartContract: "
            "Unexpected nullptr pointer in party "
            "map.\n");

        /*
         -- Load up the authorizing agent's Nym, if not already loaded. (Why? To
         verifySignature. Also, just to have
            it loaded so I don't have to load it twice in case it's needed for
         verifying one/some of the accts.) So really:
         -- Verify each party, that the authorizing agent and signature are all
         good. (I think I have this already...)
         -- Definitely during this, need to make sure that the contents of the
         signed version match the contents of the main version, for each signer.
         -- Verify that the authorizing agent actually has the opening
         transaction # for the party issued to him. (Do I have this?....)

         -- REMOVE the Opening transaction # for each agent.
         (leaving it as issued, but no longer as "available to be used on
         another transaction".)

         THE ABOVE is all accomplished via VerifyPartyAuthorization()..

         Next:

         -- Loop through all the asset accounts
         -- For each, get a pointer to the authorized agent and verify the
         CLOSING number for that asset acct. (I have something like this?...)

         -- Since we're looping through all the agents, and looping through all
         the asset accounts, and checking the agent for each asset account,
         then we might as well make sure that each agent is a legit agent for
         the party, and that each account has a legit agent lording over it.
         (Don't I do something like that already?)
         */

        bool bToBurnOrNotToBurn = bBurnTransNo;

        // If we're burning a number, but THIS party has the same opening # as
        // the smart contract itself, then don't bother verifying / burning this
        // specific number (since it's been done already, higher-up.)
        //
        if (bBurnTransNo &&  // If this party's opening number is the SMART
                             // CONTRACT's opening number, then this party
            (GetTransactionNum() ==
             pParty->GetOpeningTransNo()))  // MUST be the ACTIVATOR. (No need
                                            // to mark his trans# as IN USE
                                            // since already done earlier.)
        {
            // In cases where we're supposed to burn the transaction number, we
            // do that
            // for ALL parties EXCEPT the one who has the same Opening# as the
            // SmartContract
            // has for its GetTransactionNum().  Why? Because that one was
            // already burned, when
            // the activating party (pParty) activated the smart contract. At
            // that time, the normal
            // transaction system inside Server burned the # as part of its
            // process before even
            // calling NotarizeSmartContract().  Therefore, by this point, we
            // ASSUME that party's
            // opening num has already JUST been verified, and we skip it,
            // (continuing to verify
            // all the others.)

            bToBurnOrNotToBurn = false;
        }

        mapOfConstNyms map_Nyms_NewlyLoaded;
        mapOfConstNyms map_Nyms_Already_Loaded_AS_OF_NOW;

        map_Nyms_Already_Loaded_AS_OF_NOW.insert(
            map_Nyms_Already_Loaded.begin(), map_Nyms_Already_Loaded.end());
        map_Nyms_Already_Loaded_AS_OF_NOW.insert(
            map_Nyms_Loaded_In_This_Function.begin(),
            map_Nyms_Loaded_In_This_Function.end());

        const bool bIsPartyAuthorized = VerifyPartyAuthorization(
            *pParty,       // The party that supposedly is authorized for this
                           // supposedly executed agreement.
            theServerNym,  // For verifying signature on the authorizing Nym,
                           // when loading it
            strNotaryID,   // For verifying issued num, need the notaryID the #
                           // goes with.
            &map_Nyms_Already_Loaded_AS_OF_NOW,
            &map_Nyms_NewlyLoaded,
            bToBurnOrNotToBurn);  // bBurnTransNo = true  (default is false)

        map_Nyms_Loaded_In_This_Function.insert(
            map_Nyms_NewlyLoaded.begin(), map_Nyms_NewlyLoaded.end());

        // By this point, we've verified that pParty->GetOpeningTransNo() really
        // is ISSUED to pParty.
        // After all, you can Verify a Party's Authorization even after the
        // smart contract has been activated.
        // But in THIS function we also want to verify TRANSACTION num (not just
        // VerifyIssuedNum()) because
        // this is where that number is actually being BURNED for each one.
        // Since VerifyPartyAuthorization() ALREADY has the Nym loaded up for
        // verification, I'm going
        // to pass in a boolean arg to verify the Transaction Num as well, and
        // burn it. (for this very purpose.)
        //
        // Due to this, We don't want to stop this loop just because one of the
        // parties failed. We want to go ahead
        // and burn ALL the opening numbers for the remainder of the parties, so
        // that they have a consistent rule (the
        // opening number is considered burned and gone after a failed
        // activation attempt, though the closing numbers
        // are salvageable.)  Otherwise they would never know, upon receiving a
        // server failure reply, whether their
        // opening number was to still be considered valid -- or not.
        //
        if (!bIsPartyAuthorized) {
            otOut << __FUNCTION__ << ": Party " << str_party_name
                  << " does NOT verify as authorized! \n";
            // We let them all go through, but we still take notice that at
            // least one failed.
            bAreAnyInvalidParties = true;

            theFailedParties.insert(
                pParty);  // (So we can skip them in the loop
                          // below. Meaning THEIR closing #s
                          // also don't get marked as "used",
                          // which is another reason for
                          // clients to just harvest the
                          // number in that case and consider
                          // it as clean, since the server
                          // is.)
            //            return false; // see above comment. We still allow all
            // parties to burn their opening #s, to keep things consistent for
            // the client GUI code.
        }
    }

    // NEXT: THE ACCOUNTS
    //
    // We loop through the parties again, now knowing they are all authorized.
    // For each party, we call pParty->LoadAndVerifyAgentNyms(). This way, they
    // will
    // all be loaded up already for when we verify the accounts. Similarly we
    // call
    // pParty->LoadAndVerifyAssetAccounts(), so that all the accounts are loaded
    // up
    // as well. (We at least need to check their signatures...) At that point,
    // all
    // of the agent nyms AND accounts have been loaded! (And verified internally
    // against
    // themselves, such as their signature, etc.)
    //
    // From there, I need to verify the Party's Ownership over the account, as
    // well as
    // verify that the authorized agent listed for each account actually has
    // signer rights
    // over that account, and verify the closing transaction #s for each account
    // against its
    // authorized agent.
    //
    bool bAreAnyInvalidAccounts = false;

    /*
     NOTE on syncronicity...

     Above, we burned all the opening numbers for ALL the various nyms, even if
     there was some failure halfway through
     the verification process. This in order to make it easy to tell whether,
     for you, that number is still good or not.

     Similarly, what about the closing numbers (below) ? They are not instantly
     BURNED just for the ATTEMPT (as the
     opening numbers were) but they are still marked below as "USED BUT STILL
     ISSUED."

     if (bAreAnyInvalidParties) just above insures that the below code will not
     run if there are any invalid parties.
     That means that if "the message succeeded but transaction failed" due to
     invalid parties, then the closing numbers
     will never be seen by the server and thus never marked as "USED BUT STILL
     ISSUED." Instead, they would still be
     considered "ISSUED AND AVAILABLE."

     Based on that:
     If some FAILURE occurs below, as above, it still completes the loop in
     order to have consistent results for the
     closing transaction numbers, for all parties. So we know that success or
     fail, if the below code runs, the closing
     number is DEFINITELY marked as "USED BUT STILL ISSUED" for ALL parties,
     whereas if the below code does not run, the
     closing number for all parties is definitely still marked as "ISSUED AND
     AVAILABLE" (as far as this message is
     concerned.)

     Thus, there is still an inconsistency. When a client is later trying to
     interpret which transaction numbers to "claw
     back", he will have to harvest back the closing numbers SOMETIMES, based on
     WHY the transaction failed verification,
     and thus he cannot rely on a "transaction failed" state to give him a
     reliable answer to that question.

     However, we must keep in mind that these CLOSING numbers have NOT been
     burned -- they ARE still on the client's local
     list for his nym as "USED BUT STILL ISSUED." If the client harvests them
     back each time consistently, what will happen?
     The client will look at each closing number, see if it's on his local
     "issued" list (which it is) and then will re-add
     it to his "available" list as well. So is it really available on the server
     side? The answer is: sometimes.

     SOLUTION?
     1. Above, cache a list of all the parties who failed verification.
     2. Allow the code below to run, even if some parties have failed.
     3. Change the code below to skip failed parties while verifying accounts.
     (Using afore-mentioned list to make this possible.)
     4. Move the "if failed parties, return false" block BELOW this loop, in
     conjunction with the invalid accounts block.

     This will guarantee that closing numbers are consistently marked as "USED
     BUT STILL ISSUED" on the server side, no matter
     HOW the transaction has failed. (At least it will then be consistent.)

     However, if the transaction HAS failed, then all those closing numbers, if
     the clients are to claw them back, must be
     marked as available once again on the server side, as well! And if the
     clients are NOT to claw them back for future use,
     then how will they ever close them out? The server already won't allow the
     numbers to be used again, since they were
     marked as "used but still issued". But the transaction is ALREADY closed
     (it failed.)

     SOLUTIONS?
     -- Server could burn the closing numbers for failed transactions, vs
     marking them as "used but issued" when the transaction
        is successful. That way any client getting notice of the failure would
     DEFINITELY know that those numbers were burned, and
        could sync himself accordingly. (We definitely need to notice all
     parties as well, in this case, since they will need
        to discard the burned numbers after that point, in order to avoid going
     out-of-sync.)

     -- Server could also, in the event of transaction failure, mark the closing
     numbers as "available again, like new!"
        This way clients could clearly determine whether to burn their numbers
     or mark them as available again, purely based
        on the transaction's failure state. (State meaning the message itself is
     successful, and transaction could be success or fail.)
        Clients will not go out of sync in this case, if they fail to adjust to
     match the server. Well, they would be out of
        sync, but not the kind that prevents you from doing new transactions.
     (Rather, it's the kind where you have certain numbers
        signed out forever and you can never get rid of them.) Therefore, a
     notice is ALSO needed for this case.

     -- I know notices are already being sent upon activation, but I need to
     research that more closely now and determine the
        exact circumstances of these notices. Clearly a notice is REQUIRED, no
     matter which above option I choose!

     Thought: If notice is REQUIRED either way, perhaps the clients can just
     assume X unless a notice is received? For example,
     a client could just ASSUME the closing number is "used but still issued"
     unless notice is received to the contrary. This IS
     how the state would be if the transaction were successful, yet if it had
     failed, or hadn't been activated yet, the client
     also wouldn't experience any syncing issues by making this assumption.

     Clearly the server HAS to, in the event of transaction failure, mark the
     closing numbers either as "available again, like new!"
     or it has to mark them as BURNED, but either way, it's an additional piece
     the server has to do (up until now it has only
     marked them as "used but still issued.") What I can see is the server HAS
     to do something about those numbers, and it HAS
     to notify all the parties whether that transaction succeeded or failed, and
     then those parties HAVE to fix their client-side
     transaction count BASED on whether that transaciton succeeded or failed.

     Therefore:

     1. Implement the above 4-step solution.
     2. Do something about all the closing numbers, if activation fails.
        (Keep the nyms loaded until that is done, so as not to have to load them
     twice.)
     3. Make sure the parties are consistently noticed either way.
     4. Make sure the parties are syncing their numbers properly based on these
     notices.


     WOW, eh?  But I believe that smart contracts are the most complicated case,
     so this has got to be the worst of it, now handled :-)

     Another note: If we assume the server burns the closing numbers, then the
     clients will suddenly be out-of-sync and unable
     to process transactions until they receive and process the notice of this
     failure. But if we instead assume that the server
     "makes new" those closing numbers, then the clients will remain in sync
     regardless of notice, and they only need to harvest
     the numbers upon notice, just so they can use them again, and they will not
     experience any "out of sync failures" along the
     way. Thus, it is preferable for the server to "make those numbers new
     again!" in the event of activation failure, AND to notice
     the parties of this so that they can do likewise.
     One way to notice them is to drop the "message succeeded but transaction
     failed" server reply into ALL of their Nymboxes,
     and not just the activator's. But not sure if that's safe... hm.

     */

    for (auto& it_party : m_mapParties) {
        const std::string str_party_name = it_party.first;
        OTParty* pParty = it_party.second;
        OT_ASSERT_MSG(
            nullptr != pParty, "Unexpected nullptr pointer in party map.");

        // SKIP FAILED PARTIES...
        //
        auto it_failed = theFailedParties.find(pParty);

        if (theFailedParties.end() != it_failed)  // this means pParty was found
                                                  // on the FAILED list. (So we
                                                  // can skip it here.)
        {
            otOut << __FUNCTION__ << ": FYI, at least one party ("
                  << str_party_name
                  << ") has failed, and right now I'm skipping verification of "
                     "his asset accounts.\n";
            continue;
        }

        mapOfConstNyms map_Nyms_NewlyLoaded;
        mapOfConstNyms map_Nyms_Already_Loaded_AS_OF_NOW;

        map_Nyms_Already_Loaded_AS_OF_NOW.insert(
            map_Nyms_Already_Loaded.begin(), map_Nyms_Already_Loaded.end());
        map_Nyms_Already_Loaded_AS_OF_NOW.insert(
            map_Nyms_Loaded_In_This_Function.begin(),
            map_Nyms_Loaded_In_This_Function.end());

        // After calling this, map_Nyms_NewlyLoaded will contain pointers to
        // Nyms that MUST BE CLEANED UP.
        // LoadAndVerifyAgentNyms will not bother loading any Nyms which appear
        // on map_Nyms_Already_Loaded.
        //
        const bool bAgentsLoaded = pParty->LoadAndVerifyAgentNyms(
            theServerNym,
            map_Nyms_Already_Loaded_AS_OF_NOW,  // Nyms it won't
                                                // bother loading
                                                // 'cause they are
                                                // loaded already.
            map_Nyms_NewlyLoaded);  // Nyms it had to load itself, and thus that
                                    // YOU must clean up afterwards.
        map_Nyms_Loaded_In_This_Function.insert(
            map_Nyms_NewlyLoaded.begin(), map_Nyms_NewlyLoaded.end());
        if (!bAgentsLoaded) {
            otOut << __FUNCTION__
                  << ": Failed trying to Load and Verify Agent Nyms for party: "
                  << str_party_name << "\n";
            // We let them all go through, so there is consistent output, but we
            // still take notice that at least one failed.
            bAreAnyInvalidAccounts = true;
        }

        mapOfAccounts map_Accts_NewlyLoaded, map_Accts_Already_Loaded_AS_OF_NOW;

        map_Accts_Already_Loaded_AS_OF_NOW.insert(
            map_Accts_Already_Loaded.begin(), map_Accts_Already_Loaded.end());
        map_Accts_Already_Loaded_AS_OF_NOW.insert(
            map_Accts_Loaded_In_This_Function.begin(),
            map_Accts_Loaded_In_This_Function.end());

        // After calling this, map_Accts_NewlyLoaded will contain pointers to
        // Accts that MUST BE CLEANED UP.
        // LoadAndVerifyAssetAccounts will not bother loading any Accts which
        // appear on map_Accts_Already_Loaded.
        //
        const bool bAcctsLoaded = pParty->LoadAndVerifyAssetAccounts(
            theServerNym,
            strNotaryID,
            map_Accts_Already_Loaded_AS_OF_NOW,  // Accts it won't bother
                                                 // loading 'cause they are
                                                 // loaded already.
            map_Accts_NewlyLoaded);  // Accts it had to load itself, and thus
                                     // that YOU must clean up afterwards.

        map_Accts_Loaded_In_This_Function.insert(
            map_Accts_NewlyLoaded.begin(), map_Accts_NewlyLoaded.end());
        if (!bAcctsLoaded) {
            otOut
                << __FUNCTION__
                << ": Failed trying to Load and Verify Asset Accts for party: "
                << str_party_name << "\n";
            // We let them all go through, so there is consistent output, but we
            // still take notice that at least one failed.
            bAreAnyInvalidAccounts = true;
        }

        // BY THIS POINT, FOR THIS PARTY, we have successfully loaded and
        // verified ALL of the Nyms,
        // for ALL of the party's agents, and ALL of the asset accounts, for
        // this party. We know the
        // Party has pointers internally to all of those objects as well.
        // Therefore if we erase any of those objects, we must also clear the
        // pointers!
        //
        const bool bAreAcctsVerified = pParty->VerifyAccountsWithTheirAgents(
            strNotaryID,
            bBurnTransNo);  // bBurnTransNo=false by default.

        if (!bAreAcctsVerified) {
            otOut << __FUNCTION__
                  << ": Failed trying to Verify Asset Accts with their Agents, "
                     "for party: "
                  << str_party_name << "\n";
            // We let them all go through, so there is consistent output, but we
            // still take notice that at least one failed.
            bAreAnyInvalidAccounts = true;
        }

        // Now we don't delete these until AFTER the loop, until after we know
        // if it was a success.
        // If it failed, then we can fix their closing transaction numbers from
        // "used but still issued" back to
        // "issued and available for use." (If it was a success, then we don't
        // do anything, since the numbers are
        // already marked appropriately. And we still clean up all the nyms /
        // accounts--just AFTER this loop.)
        //
        //        pParty->ClearTemporaryPointers(); // Don't want any bad
        // pointers floating around after cleanup. (Pointers must be cleared in
        // same scope as whatever they point to...)
        //      OTSmartContract::CleanupNyms (map_Nyms_NewlyLoaded);  // HAVE to
        // do this, or we'll leak. Even if something returned
        //      OTSmartContract::CleanupAccts(map_Accts_NewlyLoaded); // false,
        // some objects may have been loaded before it failed.
    }

    const bool bSuccess =
        (!bAreAnyInvalidParties &&
         !bAreAnyInvalidAccounts);  // <=== THE RETURN VALUE

    if (bAreAnyInvalidParties)
        otOut << __FUNCTION__
              << ": Failure: There are invalid party(s) on "
                 "this smart contract.\n";

    if (bAreAnyInvalidAccounts)
        otOut << __FUNCTION__
              << ": Failure: there are invalid account(s) or "
                 "authorized agent(s) on this smart "
                 "contract.\n";

    // IF we marked the numbers as IN USE (bBurnTransNo) but then FAILURE
    // occurred,
    // then we need to CLOSE the opening numbers (RemoveIssuedNum) meaning they
    // are done
    // and over, and can never be used again, and we also need to HARVEST the
    // CLOSING
    // numbers, meaning they are available again for use in the future.
    // (If failure occurred in a case where we did NOT burn the numbers, then we
    // wouldn't
    // be worried about putting them back, now would we?)
    //
    //
    if (!bSuccess &&   // If this function was not a success, overall, and
        bBurnTransNo)  // if we DID burn (mark as 'in use') the numbers during
                       // this function...
    {
        // CloseoutOpeningNumbers...
        // This closes the opening numbers for all parties except the activator
        // Nym. Why not him? Because his is already closed out in
        // NotarizeTransaction, if the transaction has failed.
        //
        // Also, where that happens, his set of open cron items is also updated
        // to remove the number from that list. (As the below function also does
        // for the rest of the nyms involved.)
        CloseoutOpeningNumbers();

        // Then harvest those closing numbers back again (for ALL Nyms.) (Not
        // the opening numbers, which are already burned for good by this
        // point.)
        HarvestClosingNumbers(
            &theServerNym,  // theServerNym is the signer, here on the server
                            // side.
            &theFailedParties);  // Since we skipped marking the closing numbers
                                 // for these failed parties, then we skip
                                 // adding those same numbers back again, too.
    }

    // Now that all potentially-needed harvesting is done, we can clean up.
    //
    ClearTemporaryPointers();  // Don't want any bad pointers floating
                               // around after cleanup.

    OTSmartContract::CleanupNyms(
        map_Nyms_Loaded_In_This_Function);  // HAVE to do this, or we'll leak.
                                            // Even if something returned
    OTSmartContract::CleanupAccts(
        map_Accts_Loaded_In_This_Function);  // false, some objects may have
                                             // been loaded before it failed.

    // DONE: if the above loop fails halfway through, then we should really PUT
    // BACK the closing
    // transaction #s that we removed. After all, we have a list of them.
    // Otherwise the only way
    // to know which parties have their numbers still, and which ones don't,
    // would be to stick a notice
    // in their nymbox, like we do for finalReceipt.  Perhaps such a notice
    // should ALWAYS go into the
    // Nymbox in these cases... *shrug*

    return bSuccess;
}

// Used for closing-out the opening transaction numbers
// (RemoveIssuedNum(lNymOpeningNumber)), for
// all the Nyms, after the smart contract, for whatever reason, has failed to
// activate. ASSUMES
// the Nyms are already loaded, since this is used in a place where they are
// already still loaded.
// (Used above, in VerifySmartContract.)
//
// Also: Saves any Nyms that it does this for.
// NOTE: Skips the activating Nym, since his opening number is ALREADY closed
// out on failure,
// in NotarizeTransaction.
//
// (Server-side.)
//
void OTSmartContract::CloseoutOpeningNumbers()
{
    const String strNotaryID(GetNotaryID());

    for (auto& it : m_mapParties) {
        OTParty* pParty = it.second;
        OT_ASSERT_MSG(
            nullptr != pParty,
            "OTSmartContract::CloseoutOpeningNumbers:"
            " Unexpected nullptr pointer in party map.");

        // Closeout the opening transaction numbers:
        if (GetTransactionNum() !=
            // We skip the activating Nym. (His is already closed-out in
            // NotarizeTransaction.)
            pParty->GetOpeningTransNo())
            pParty->CloseoutOpeningNumber(strNotaryID);
    }
}

// Used for adding the closing transaction numbers BACK to all the Nyms, after
// the smart contract,
// for whatever reason, has failed to activate. ASSUMES the Nyms are already
// loaded, since this is
// used in a place where they are already still loaded.  (Used above, in
// VerifySmartContract.)
//
// Also: Saves any Nyms that it harvests for.
//
// (Server-side.)
//
void OTSmartContract::HarvestClosingNumbers(
    Nym* pSignerNym,
    std::set<OTParty*>* pFailedParties)
{
    const String strNotaryID(GetNotaryID());

    for (auto& it : m_mapParties) {
        const std::string str_party_name = it.first;
        OTParty* pParty = it.second;
        OT_ASSERT_MSG(
            nullptr != pParty,
            "OTSmartContract::HarvestClosingNumbers: "
            "Unexpected nullptr pointer in party map.");

        // If certain parties failed verification, then
        // OTSmartContract::VerifySmartContract() is smart enough
        // not to bother verifying the accounts of those parties. Thus, the
        // closing numbers for those accounts
        // could NOT have been marked as "used" (since they were skipped). So we
        // passed that same set of failed
        // parties into this function here, so that we can skip them here as
        // well, when harvesting the closing
        // numbers back again.
        //
        if (nullptr != pFailedParties)  // There are failed parties.
        {

            // Skip failed parties...
            //
            auto it_failed = pFailedParties->find(pParty);

            if (pFailedParties->end() != it_failed)  // this means pParty was
            // found on the FAILED list.
            // (So we can skip it here.)
            {
                otOut << __FUNCTION__ << ": FYI, at least one party ("
                      << str_party_name
                      << ") has failed verification, so right now I'm skipping "
                         "harvesting of his "
                         "closing transaction numbers. (Since he failed, we "
                         "never verified his accounts, so we never grabbed "
                         "those closing "
                         "numbers in the first place, so there's no need to "
                         "grab them back now.)\n";
                continue;
            }
        }

        // For all non-failed parties, now we harvest the closing transaction
        // numbers:
        pParty->HarvestClosingNumbers(strNotaryID);
    }
}

// Used for adding transaction numbers back to a Nym, after deciding not to use
// this smart contract, or failing in trying to use it.
void OTSmartContract::HarvestClosingNumbers(ServerContext& context)
{
    // We do NOT call the parent version.
    // OTCronItem::HarvestClosingNumbers(theNym);

    // For payment plan, the parent (OTCronItem) grabs the sender's #s, and then
    // the subclass's override (OTAgreement::HarvestClosingNumbers) grabs the
    // recipient's #s. But with SMART CONTRACTS, there are only "the parties"
    // and they ALL burned an opening #, plus they can ALL harvest their closing
    // #s if activation failed. In fact, done: might as well send them all a
    // notification if it fails, so they can all AUTOMATICALLY remove said
    // numbers from their future balance agreements.
    for (auto& it : m_mapParties) {
        OTParty* pParty = it.second;
        OT_ASSERT_MSG(
            nullptr != pParty, "Unexpected nullptr pointer in party map.");

        pParty->HarvestClosingNumbers(context);
    }
}

// You usually wouldn't want to use this, since if the transaction failed, the
// opening number is already burned and gone. But there might be cases where
// it's not, and you want to retrieve it. So I added this function for those
// cases. (In most cases, you will prefer HarvestClosingNumbers().)
void OTSmartContract::HarvestOpeningNumber(ServerContext& context)
{
    // We do NOT call the parent version.
    // OTCronItem::HarvestOpeningNumber(theNym);

    // For payment plan, the parent (OTCronItem) grabs the sender's #s, and then
    // the subclass's override (OTAgreement::HarvestClosingNumbers) grabs the
    // recipient's #s. But with SMART CONTRACTS, there are only "the parties"
    // and they ALL burned an opening #, plus they can ALL harvest their closing
    // #s if activation failed. In fact, todo: might as well send them all a
    // notification if it fails, so they can all AUTOMATICALLY remove said
    // numbers from their future balance agreements.

    for (auto& it : m_mapParties) {
        OTParty* pParty = it.second;
        OT_ASSERT_MSG(
            nullptr != pParty, "Unexpected nullptr pointer in party map.");

        pParty->HarvestOpeningNumber(context);
    }
}

// static
void OTSmartContract::CleanupNyms(mapOfConstNyms& theMap)
{

    while (!theMap.empty()) {
        const Nym* pNym = theMap.begin()->second;
        OT_ASSERT(nullptr != pNym);

        delete pNym;
        pNym = nullptr;

        theMap.erase(theMap.begin());
    }
}

// static
void OTSmartContract::CleanupAccts(mapOfAccounts& theMap)
{

    while (!theMap.empty()) {
        Account* pAcct = theMap.begin()->second;
        OT_ASSERT(nullptr != pAcct);

        delete pAcct;
        pAcct = nullptr;

        theMap.erase(theMap.begin());
    }
}

// AddParty()
// For adding a theoretical party to a smart contract, as part of the contract's
// design, so the
// contract can be circulated BLANK and many different instances of it might be
// used.
//
// (The party, at this stage, has a name, and accounts with instrument
// definitions, but no
// actual Nym IDs
// or account IDs.)
// This way any Nym or Entity could later sign on as the "trustee" or as the
// "employee" etc. And
// until they do, the contract still shows the "trustee" or "employee", allowing
// the reader to see
// how those entities are manipulated in the script code of the smartcontract.
//
bool OTSmartContract::AddParty(OTParty& theParty)
{
    if (!theParty.HasActiveAgent()) {
        otOut << "OTSmartContract::AddParty: Party doesn't have an active "
                 "agent -- who will sign for this smart contract?\n";
        return false;
    }

    // MIGHT move this below (1).. OR this might turn out to be important going
    // first...
    //
    if (!OTScriptable::AddParty(theParty)) {
        otOut << "OTSmartContract::AddParty: Failed adding party.\n";
        return false;
    }

    return true;
}

// Done:
// Similar to AddParty(). Used afterwards.
// ConfirmParty() looks up an existing party on the smart contract, then makes
// sure that it matches
// the one passed in, and then REPLACES the existing one with the new one that
// was passed in. Unlike
// AddParty (above) this version DOES expect account IDs, NymIDs, and
// transaction numbers, and it DOES
// save a signed copy internally as the ultimate confirmation. This version also
// needs to validate
// the signatures that are already there.
// Client-side.
//
//
// Note: AFTER A SUCCESSFUL CALL, the transaction numbers HAVE been set aside,
// and must be retrieved
// in the event of any failure.
//
bool OTSmartContract::ConfirmParty(OTParty& theParty, ServerContext& context)
{
    if (!theParty.HasActiveAgent()) {
        otOut << "OTSmartContract::ConfirmParty: Party doesn't have an active "
                 "agent -- who will sign for this smart contract?\n";
        return false;
    }

    // Let's RESERVE however many transaction numbers we need to confirm this
    // smartcontract...
    //
    // ReserveTransNumsForConfirm() sets aside the Opening # for the party,
    // as well as the Closing #s for all the asset accounts for that party.
    //
    // This MUST be done before calling OTScriptable::ConfirmParty, because
    // *this will get SIGNED in there, and so must have its final data in
    // place already. If the confirmation fails, we will harvest the numbers
    // back again.
    //
    if (!theParty.ReserveTransNumsForConfirm(context)) {
        otOut << "OTSmartContract::ConfirmParty: Failure trying to reserve "
                 "transaction numbers for "
                 "the smart contract. (Nym needs more numbers than he has.)\n";
        return false;
    }
    // Note: BELOW THIS POINT, the transaction numbers have been set aside, and
    // must be retrieved,
    // below this point, in the event of any failure, using this call:
    // theParty.HarvestAllTransactionNumbers(context);

    // Since EVERY party keeps his own signed copy, then we reset the creation
    // date
    // before EACH signature. That way, we have the date of signing for EVERY
    // signer.
    // (The final date will be set upon activation.)
    //
    const time64_t &CURRENT_TIME = OTTimeGetCurrentTime(),
                   OLD_TIME = GetCreationDate();

    // Set the Creation Date.
    SetCreationDate(CURRENT_TIME);

    // THIS IS where the SIGNED COPY is SAVED, so all final changes must occur
    // ABOVE this point.
    //
    if (!ot_super::ConfirmParty(theParty, context)) {
        otOut << "OTSmartContract::ConfirmParty: Failed confirming party.\n";
        SetCreationDate(OLD_TIME);  // Might as well set this back.
        // If it failed, grab BACK the numbers that we reserved above.
        theParty.HarvestAllTransactionNumbers(context);

        return false;
    }

    // SUCCESS!!
    //
    return true;

    // Are we good? The contract is compared against the other parties' signed
    // contracts; my own party and
    // transaction #s are added, and a signed copy of everything is saved in my
    // party. Then the entire contract
    // is re-signed (saving its updated contents) and then sent on to the next
    // party, who is free to release that
    // signature since I already have a signed copy in my party.
    //
    // Assuming all parties have signed AND provided valid transaction #s, then
    // the server is free to get started
    // immediately upon activation, and furthermore to cancel whenever it wants
    // (and probably just according to
    // the terms.)  There should be a standard call for seeing if a person can
    // cancel the agreement, and if it's
    // not overridden in the contract, then it defaults to return true. (Note:
    // that is done now.)
    //
    // Technically ANY party's authorizing agent could become the originator by
    // activating the contract, but
    // only if all parties have validly signed.  (Server needs to verify.)
}

// ALWAYS succeeds. (It will OT_ASSERT() otherwise.)
//
OTStash* OTSmartContract::GetStash(std::string str_stash_name)
{
    auto it = m_mapStashes.find(str_stash_name);

    if (m_mapStashes.end() == it)  // It's not there. Create it.
    {
        OTStash* pStash = new OTStash(str_stash_name);
        OT_ASSERT(nullptr != pStash);

        m_mapStashes.insert(
            std::pair<std::string, OTStash*>(str_stash_name, pStash));
        return pStash;
    }

    OTStash* pStash = it->second;
    OT_ASSERT(nullptr != pStash);

    return pStash;
}

OTSmartContract::OTSmartContract()
    : ot_super()
    , m_StashAccts(Account::stash)
    , m_tNextProcessDate(OT_TIME_ZERO)
{
    InitSmartContract();
}

OTSmartContract::OTSmartContract(const Identifier& NOTARY_ID)
    : ot_super()
    , m_StashAccts(Account::stash)
    , m_tNextProcessDate(OT_TIME_ZERO)
{
    Instrument::SetNotaryID(NOTARY_ID);
    InitSmartContract();
}

OTSmartContract::~OTSmartContract() { Release_SmartContract(); }

void OTSmartContract::InitSmartContract()
{
    m_strContractType = "SMARTCONTRACT";

    SetProcessInterval(
        SMART_CONTRACT_PROCESS_INTERVAL);  // Smart contracts current default is
                                           // 30 seconds. Actual default will
                                           // probably be configurable in config
                                           // file, and most contracts will also
                                           // probably override this.
}

void OTSmartContract::ReleaseStashes()
{

    while (!m_mapStashes.empty()) {
        OTStash* pStash = m_mapStashes.begin()->second;
        OT_ASSERT(nullptr != pStash);

        delete pStash;
        pStash = nullptr;

        m_mapStashes.erase(m_mapStashes.begin());
    }

    m_StashAccts.Release();
}

void OTSmartContract::Release_SmartContract() { ReleaseStashes(); }

void OTSmartContract::Release()
{
    Release_SmartContract();

    ot_super::Release();  // since I've overridden the base class, I call it
                          // now...

    // Then I call this to re-initialize everything
    InitSmartContract();
}

std::int32_t OTSmartContract::GetCountStashes() const
{
    return static_cast<std::int32_t>(m_mapStashes.size());
}

std::int32_t OTSmartContract::GetCountStashAccts() const
{
    return m_StashAccts.GetCountAccountIDs();
}

// Done.
//
// Before we can make sure that ALL parties have signed equivalent versions,
// we must be able to compare TWO versions.  The below function does that.
//
bool OTSmartContract::Compare(OTScriptable& rhs) const
{
    if (!OTScriptable::Compare(rhs)) return false;

    if (GetCountStashes() > 0) {
        otErr << "OTSmartContract::Compare: Error: How is this function EVER "
                 "being called when there are stashes present? Only the server "
                 "can create stashes.\n";
        return false;
    }

    if (GetCountStashAccts() > 0) {
        otErr << "OTSmartContract::Compare: Error: How is this function EVER "
                 "being called when there are "
                 "stash accounts present? Only the server can create stash "
                 "accounts.\n";
        return false;
    }

    // Compare OTSmartContract specific info here.
    const OTSmartContract* pSmartContract =
        dynamic_cast<const OTSmartContract*>(&rhs);

    if (nullptr != pSmartContract) {
        if (pSmartContract->GetCountStashes() > 0) {
            otErr << "OTSmartContract::Compare: Error: How is this function "
                     "EVER being called when there are stashes present on rhs? "
                     "Only the server can create stashes.\n";
            return false;
        }

        if (pSmartContract->GetCountStashAccts() > 0) {
            otErr << "OTSmartContract::Compare: Error: How is this function "
                     "EVER being called when there are stash accounts present "
                     "on rhs? Only the server can create stash accounts.\n";
            return false;
        }

        if ((GetNotaryID() == pSmartContract->GetNotaryID()) &&
            (GetValidFrom() == pSmartContract->GetValidFrom()) &&
            (GetValidTo() == pSmartContract->GetValidTo()))
            return true;
    }

    return false;
}

void OTSmartContract::UpdateContents()
{
    // I release this because I'm about to repopulate it.
    m_xmlUnsigned.Release();

    const String NOTARY_ID(GetNotaryID()), ACTIVATOR_NYM_ID(GetSenderNymID()),
        ACTIVATOR_ACCT_ID(GetSenderAcctID());

    OT_ASSERT(m_pCancelerNymID->empty());

    String strCanceler;

    if (m_bCanceled) m_pCancelerNymID->GetString(strCanceler);

    std::string tCreation =
        formatTimestamp(m_bCalculatingID ? OT_TIME_ZERO : GetCreationDate());
    std::string tValidFrom =
        formatTimestamp(m_bCalculatingID ? OT_TIME_ZERO : GetValidFrom());
    std::string tValidTo =
        formatTimestamp(m_bCalculatingID ? OT_TIME_ZERO : GetValidTo());
    std::string tNextProcess =
        formatTimestamp(m_bCalculatingID ? OT_TIME_ZERO : GetNextProcessDate());

    // OTSmartContract
    Tag tag("smartContract");

    tag.add_attribute("version", m_strVersion.Get());
    tag.add_attribute("notaryID", m_bCalculatingID ? "" : NOTARY_ID.Get());
    tag.add_attribute(
        "activatorNymID", m_bCalculatingID ? "" : ACTIVATOR_NYM_ID.Get());
    tag.add_attribute(
        "activatorAcctID", m_bCalculatingID ? "" : ACTIVATOR_ACCT_ID.Get());
    tag.add_attribute(
        "lastSenderNymID", m_bCalculatingID ? "" : m_strLastSenderUser.Get());
    tag.add_attribute(
        "lastSenderAcctID", m_bCalculatingID ? "" : m_strLastSenderAcct.Get());
    tag.add_attribute(
        "lastRecipientNymID",
        m_bCalculatingID ? "" : m_strLastRecipientUser.Get());
    tag.add_attribute(
        "lastRecipientAcctID",
        m_bCalculatingID ? "" : m_strLastRecipientAcct.Get());
    tag.add_attribute("canceled", formatBool(m_bCanceled));
    tag.add_attribute("cancelerNymID", m_bCanceled ? strCanceler.Get() : "");
    tag.add_attribute(
        "transactionNum", formatLong(m_bCalculatingID ? 0 : m_lTransactionNum));
    tag.add_attribute("creationDate", tCreation);
    tag.add_attribute("validFrom", tValidFrom);
    tag.add_attribute("validTo", tValidTo);
    tag.add_attribute("nextProcessDate", tNextProcess);

    // OTCronItem
    if (!m_bCalculatingID) {
        for (std::int32_t i = 0; i < GetCountClosingNumbers(); i++) {
            std::int64_t lClosingNumber = GetClosingTransactionNoAt(i);
            OT_ASSERT(lClosingNumber > 0);

            TagPtr tagClosingNo(new Tag("closingTransactionNumber"));

            tagClosingNo->add_attribute("value", formatLong(lClosingNumber));

            tag.add_tag(tagClosingNo);

            // For OTSmartContract, this should only contain a single number,
            // from the activator Nym.
            // I preserved the loop anyway. Call me crazy. But I'm still
            // displaying an error if there's more than one.
            if (i > 0)
                otErr << "OTSmartContract::" << __FUNCTION__
                      << ": ERROR: There's only ever "
                         "supposed to be a single closing number here (for "
                         "smart contracts.)\n";
        }
    }

    // *** OT SCRIPTABLE ***
    // FYI: this is: void
    UpdateContentsToTag(tag, m_bCalculatingID);

    if (!m_bCalculatingID) {

        // Save m_StashAccts.
        //
        // (This is an object that contains a map of OTAccountIDs, by
        // instrument_definition_id)
        //
        m_StashAccts.Serialize(tag);

        // This is a map of OTStash's, by stash_name.
        // EACH ONE contains a map of OTStashItems, by instrument_definition_id

        // These stashes are what the scripts interact with. They have names.
        // Whereas the stash accts (above) are the actual accountIDs
        for (auto& it : m_mapStashes) {
            // where the actual funds are stored for each instrument definition.
            OTStash* pStash = it.second;
            OT_ASSERT(nullptr != pStash);
            pStash->Serialize(tag);
        }
    }

    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned.Concatenate("%s", str_result.c_str());
}

// Used internally here.
void OTSmartContract::ReleaseLastSenderRecipientIDs()
{
    m_strLastSenderUser.Release();  // This is the last Nym ID of a party who
                                    // SENT money.
    m_strLastSenderAcct.Release();  // This is the last Acct ID of a party who
                                    // SENT money.
    m_strLastRecipientUser.Release();  // This is the last Nym ID of a party who
                                       // RECEIVED money.
    m_strLastRecipientAcct.Release();  // This is the last Acct ID of a party
                                       // who RECEIVED money.
}

// We call this just before activation (in OT_API::activateSmartContract) in
// order
// to make sure that certain IDs and transaction #s are set, so the smart
// contract
// will interoperate with the old Cron Item system of doing things.
//
void OTSmartContract::PrepareToActivate(
    const std::int64_t& lOpeningTransNo,
    const std::int64_t& lClosingTransNo,
    const Identifier& theNymID,
    const Identifier& theAcctID)
{
    SetTransactionNum(lOpeningTransNo);

    ClearClosingNumbers();  // Just in case. Should be unnecessary, but you
                            // never know how people might screw around.
    AddClosingTransactionNo(lClosingTransNo);

    SetSenderNymID(theNymID);  // This is the activator of the contract. (NOT
                               // the actual "sender" of any single payment, in
                               // the case of smart contracts anyway.)
    SetSenderAcctID(theAcctID);  // This is an account provided by the activator
                                 // so a closing number and final receipt are
                                 // guaranteed for this smart contract.

    ReleaseLastSenderRecipientIDs();  // These should be blank starting out
                                      // anyway.

    // You shouldn't have any of these anyway; the server only creates
    // them after a smart contract is activated.
    //
    ReleaseStashes();
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
std::int32_t OTSmartContract::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    const String strNodeName(xml->getNodeName());

    std::int32_t nReturnVal = 0;

    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // OTSmartContract::ProcessXMLNode calls OTCronItem::ProcessXMLNode,
    // which calls OTScriptable... Meaning:
    //
    // NO NEED to explicitly load OTScriptable stuff here!
    //
    nReturnVal = ot_super::ProcessXMLNode(xml);

    if (0 != (nReturnVal)) {
        return nReturnVal;
    }

    if (strNodeName.Compare("smartContract")) {
        m_strVersion = xml->getAttributeValue("version");

        const String strNotaryID(xml->getAttributeValue("notaryID"));
        const String strActivatorNymID(
            xml->getAttributeValue("activatorNymID"));
        const String strActivatorAcctID(
            xml->getAttributeValue("activatorAcctID"));
        const String strCanceled(xml->getAttributeValue("canceled"));
        const String strCancelerNymID(xml->getAttributeValue("cancelerNymID"));

        if (strNotaryID.Exists()) {
            const auto NOTARY_ID = Identifier::Factory(strNotaryID);
            SetNotaryID(NOTARY_ID);
        }
        if (strActivatorNymID.Exists()) {
            const auto ACTIVATOR_NYM_ID =
                Identifier::Factory(strActivatorNymID);
            SetSenderNymID(ACTIVATOR_NYM_ID);
        }
        if (strActivatorAcctID.Exists()) {
            const auto ACTIVATOR_ACCT_ID =
                Identifier::Factory(strActivatorAcctID);
            SetSenderAcctID(ACTIVATOR_ACCT_ID);
        }

        if (strCanceled.Exists() && strCanceled.Compare("true")) {
            m_bCanceled = true;

            if (strCancelerNymID.Exists())
                m_pCancelerNymID->SetString(strCancelerNymID);
            // else log
        } else {
            m_bCanceled = false;
            m_pCancelerNymID->Release();
        }

        const String strTransNum = xml->getAttributeValue("transactionNum");

        SetTransactionNum(strTransNum.Exists() ? strTransNum.ToLong() : 0);

        std::int64_t tValidFrom =
            parseTimestamp(xml->getAttributeValue("validFrom"));
        std::int64_t tValidTo =
            parseTimestamp(xml->getAttributeValue("validTo"));
        std::int64_t tCreation =
            parseTimestamp(xml->getAttributeValue("creationDate"));
        std::int64_t tNextProcess =
            parseTimestamp(xml->getAttributeValue("nextProcessDate"));

        SetValidFrom(OTTimeGetTimeFromSeconds(tValidFrom));
        SetValidTo(OTTimeGetTimeFromSeconds(tValidTo));
        SetCreationDate(OTTimeGetTimeFromSeconds(tCreation));
        SetNextProcessDate(OTTimeGetTimeFromSeconds(tNextProcess));

        // These are stored for RECEIPTS, so if there is an inbox receipt with
        // an amount,
        // we will know who was sending and receiving.  If sender or receiver is
        // blank, that
        // means the source/destination was a STASH instead of an account. FYI.
        //
        m_strLastSenderUser = xml->getAttributeValue(
            "lastSenderNymID");  // Last Nym ID of a party who SENT money.
        m_strLastSenderAcct = xml->getAttributeValue(
            "lastSenderAcctID");  // Last Acct ID of a party who SENT money.
        m_strLastRecipientUser =
            xml->getAttributeValue("lastRecipientNymID");  // Last Nym ID of a
                                                           // party who RECEIVED
                                                           // money.
        m_strLastRecipientAcct =
            xml->getAttributeValue("lastRecipientAcctID");  // Last Acct ID of a
        // party who RECEIVED
        // money.

        otWarn << "\n\n Smartcontract. Transaction Number: "
               << m_lTransactionNum << "\n";

        otInfo << " Creation Date: " << tCreation
               << "   Valid From: " << tValidFrom << "\n Valid To: " << tValidTo
               << "\n"
                  " NotaryID: "
               << strNotaryID
               << "\n"
                  " activatorNymID: "
               << strActivatorNymID << "\n ";

        nReturnVal = 1;
    } else if (strNodeName.Compare("accountList"))  // the stash reserve account
                                                    // IDs.
    {
        const String strAcctType = xml->getAttributeValue("type");
        const String strAcctCount = xml->getAttributeValue("count");

        if ((-1) ==
            m_StashAccts.ReadFromXMLNode(xml, strAcctType, strAcctCount)) {
            otErr << "OTSmartContract::ProcessXMLNode: Error loading stash "
                     "accountList.\n";
            nReturnVal = (-1);
        } else
            nReturnVal = 1;
    } else if (strNodeName.Compare("stash"))  // the actual stashes.
    {
        const String strStashName = xml->getAttributeValue("name");
        const String strItemCount = xml->getAttributeValue("count");

        const std::string str_stash_name = strStashName.Get();
        OTStash* pStash = new OTStash(str_stash_name);
        OT_ASSERT(nullptr != pStash);

        if ((-1) == pStash->ReadFromXMLNode(xml, strStashName, strItemCount)) {
            otErr << "OTSmartContract::ProcessXMLNode: Error loading stash: "
                  << strStashName << "\n";
            delete pStash;
            nReturnVal = (-1);
        } else {
            // Success
            //
            m_mapStashes.insert(
                std::pair<std::string, OTStash*>(strStashName.Get(), pStash));

            nReturnVal = 1;
        }
    }

    return nReturnVal;
}

// DONE: Make a GENERIC VERSION of the BELOW function, that script coders can
// call
// whenever they need to move money between two parties!!!! The more I look at
// it,
// the more I realize I can probably use it NEARLY "as is" !
//
// true == success, false == failure.
//
bool OTSmartContract::MoveFunds(
    const mapOfConstNyms& map_NymsAlreadyLoaded,
    const std::int64_t& lAmount,
    const Identifier& SOURCE_ACCT_ID,     // GetSenderAcctID();
    const Identifier& SENDER_NYM_ID,      // GetSenderNymID();
    const Identifier& RECIPIENT_ACCT_ID,  // GetRecipientAcctID();
    const Identifier& RECIPIENT_NYM_ID)   // GetRecipientNymID();
{
    OTCron* pCron = GetCron();
    OT_ASSERT(nullptr != pCron);

    Nym* pServerNym = pCron->GetServerNym();
    OT_ASSERT(nullptr != pServerNym);

    if (lAmount <= 0) {
        otOut << " OTCronItem::MoveFunds: Error: lAmount cannot be 0 or <0. "
                 "(Value passed in was "
              << lAmount << ".)\n";
        return false;
    }

    bool bSuccess = false;  // The return value.

    const auto NOTARY_ID = Identifier::Factory(pCron->GetNotaryID());
    const auto NOTARY_NYM_ID = Identifier::Factory(*pServerNym);

    String strSenderNymID(SENDER_NYM_ID), strRecipientNymID(RECIPIENT_NYM_ID),
        strSourceAcctID(SOURCE_ACCT_ID), strRecipientAcctID(RECIPIENT_ACCT_ID),
        strServerNymID(NOTARY_NYM_ID);

    // Make sure they're not the same Account IDs ...
    // Otherwise we would have to take care not to load them twice, like with
    // the Nyms below.
    // (Instead I just disallow it entirely. After all, even if I DID allow the
    // account to transfer
    // to itself, there would be no difference in balance than disallowing it.)
    //
    if (SOURCE_ACCT_ID == RECIPIENT_ACCT_ID) {
        otOut << "OTCronItem::MoveFunds: Aborted move: both account IDs were "
                 "identical.\n";
        FlagForRemoval();  // Remove from Cron
        return false;  // TODO: should have a "Validate Scripts" function that
                       // weeds this crap out before we even get here. (There
                       // are other examples...)
    }
    // When the accounts are actually loaded up, then we should also compare
    // the instrument definitions to make sure they were what we expected them
    // to be.

    // Need to load up the ORIGINAL VERSION OF THIS SMART CONTRACT
    // Will need to verify the parties' signatures, as well as attach a copy of
    // it to the receipt.

    OTCronItem* pOrigCronItem = nullptr;

    // OTCronItem::LoadCronReceipt loads the original version with the user's
    // signature.
    // (Updated versions, as processing occurs, are signed by the server.)
    pOrigCronItem = OTCronItem::LoadCronReceipt(GetTransactionNum());

    OT_ASSERT(nullptr != pOrigCronItem);  // How am I processing it now if the
                                          // receipt wasn't saved in the first
                                          // place??
    // TODO: Decide global policy for handling situations where the hard drive
    // stops working, etc.

    // When theOrigPlanGuardian goes out of scope, pOrigCronItem gets deleted
    // automatically.
    std::unique_ptr<OTCronItem> theOrigPlanGuardian(pOrigCronItem);

    // strOrigPlan is a String copy (a PGP-signed XML file, in string form) of
    // the original Payment Plan request...
    String strOrigPlan(*pOrigCronItem);  // <====== Farther down in the code, I
                                         // attach this string to the receipts.

    // Make sure to clean these up.
    //    delete pOrigCronItem;        // theOrigPlanGuardian will handle this
    // now, whenever it goes out of scope.
    //    pOrigCronItem = nullptr;        // So I don't need to worry about
    // deleting this anymore. I can keep it around and
    // use it all I want, and return anytime, and it won't leak.

    // -------------- Make sure have both nyms loaded and checked out.
    // --------------------------------------------------
    // WARNING: 1 or both of the Nyms could be also the Server Nym. They could
    // also be the same Nym, but NOT the Server.
    // In all of those different cases, I don't want to load the same file twice
    // and overwrite it with itself, losing
    // half of all my changes. I have to check all three IDs carefully and set
    // the pointers accordingly, and then operate
    // using the pointers from there.

    Nym theSenderNym, theRecipientNym;  // We MIGHT use ONE, OR BOTH, of
                                        // these, or none. (But probably
                                        // both.)

    // Find out if either Nym is actually also the server.
    bool bSenderNymIsServerNym =
        ((SENDER_NYM_ID == NOTARY_NYM_ID) ? true : false);
    bool bRecipientNymIsServerNym =
        ((RECIPIENT_NYM_ID == NOTARY_NYM_ID) ? true : false);

    // We also see, after all that is done, whether both pointers go to the same
    // entity.
    // (We'll want to know that later.)
    bool bUsersAreSameNym =
        ((SENDER_NYM_ID == RECIPIENT_NYM_ID) ? true : false);

    const Nym* pSenderNym = nullptr;
    const Nym* pRecipientNym = nullptr;

    auto it_sender = map_NymsAlreadyLoaded.find(strSenderNymID.Get());
    auto it_recipient = map_NymsAlreadyLoaded.find(strRecipientNymID.Get());

    if (map_NymsAlreadyLoaded.end() !=
        it_sender)  // found the sender in list of Nyms that are already loaded.
    {
        pSenderNym = it_sender->second;
        OT_ASSERT(
            (nullptr != pSenderNym) && (pSenderNym->CompareID(SENDER_NYM_ID)));
    }
    if (map_NymsAlreadyLoaded.end() != it_recipient)  // found the recipient in
                                                      // list of Nyms that are
                                                      // already loaded.
    {
        pRecipientNym = it_recipient->second;
        OT_ASSERT(
            (nullptr != pRecipientNym) &&
            (pRecipientNym->CompareID(RECIPIENT_NYM_ID)));
    }

    // Figure out if Sender Nym is also Server Nym.
    if (bSenderNymIsServerNym) {
        // If the First Nym is the server, then just point to that.
        pSenderNym = pServerNym;
    } else if (nullptr == pSenderNym)  // Else load the First Nym from storage,
                                       // if still not found.
    {
        theSenderNym.SetIdentifier(
            SENDER_NYM_ID);  // theSenderNym is pSenderNym

        if (!theSenderNym.LoadPublicKey()) {
            String strNymID(SENDER_NYM_ID);
            otErr << "OTCronItem::MoveFunds: Failure loading Sender Nym public "
                     "key: "
                  << strNymID << "\n";
            FlagForRemoval();  // Remove it from future Cron processing, please.
            return false;
        }

        if (theSenderNym.VerifyPseudonym() &&
            theSenderNym.LoadSignedNymfile(*pServerNym))  // ServerNym here is
                                                          // not theSenderNym's
                                                          // identity, but
                                                          // merely the signer
                                                          // on this file.
        {
            otOut << "OTCronItem::MoveFunds: Loading sender Nym, since he **** "
                     "APPARENTLY **** wasn't already loaded.\n"
                     "(On a cron item processing, this is normal. But if you "
                     "triggered a clause directly, then your Nym SHOULD be "
                     "already loaded...)\n";
            pSenderNym = &theSenderNym;  //  <=====
        } else {
            String strNymID(SENDER_NYM_ID);
            otErr << "OTCronItem::MoveFunds: Failure loading or verifying "
                     "Sender Nym public key: "
                  << strNymID << "\n";
            FlagForRemoval();  // Remove it from future Cron processing, please.
            return false;
        }
    }

    // Next, we also find out if Recipient Nym is Server Nym...
    if (bRecipientNymIsServerNym) {
        // If the Recipient Nym is the server, then just point to that.
        pRecipientNym = pServerNym;
    } else if (bUsersAreSameNym)  // Else if the participants are the same Nym,
                                  // point to the one we already loaded.
    {
        pRecipientNym = pSenderNym;       // theSenderNym is pSenderNym
    } else if (nullptr == pRecipientNym)  // Otherwise load the Other Nym from
                                          // Disk and point to that, if still
                                          // not found.
    {
        theRecipientNym.SetIdentifier(RECIPIENT_NYM_ID);

        if (!theRecipientNym.LoadPublicKey()) {
            String strNymID(RECIPIENT_NYM_ID);
            otErr << "OTCronItem::MoveFunds: Failure loading Recipient Nym "
                     "public key: "
                  << strNymID << "\n";
            FlagForRemoval();  // Remove it from future Cron processing, please.
            return false;
        }

        if (theRecipientNym.VerifyPseudonym() &&
            theRecipientNym.LoadSignedNymfile(*pServerNym)) {
            otOut << "OTCronItem::MoveFunds: Loading recipient Nym, since he "
                     "**** APPARENTLY **** wasn't already loaded.\n"
                     "(On a cron item processing, this is normal. But if you "
                     "triggered a clause directly, then your Nym SHOULD be "
                     "already loaded...)\n";

            pRecipientNym = &theRecipientNym;  //  <=====
        } else {
            String strNymID(RECIPIENT_NYM_ID);
            otErr << "OTCronItem::MoveFunds: Failure loading or verifying "
                     "Recipient Nym public key: "
                  << strNymID << "\n";
            FlagForRemoval();  // Remove it from future Cron processing, please.
            return false;
        }
    }

    // Below this point, both Nyms are loaded and good-to-go.

    mapOfConstNyms map_ALREADY_LOADED;  // I know I passed in one of these, but
                                        // now I have processed the Nym pointers
                                        // (above) and have better data here
                                        // now.
    auto it_temp = map_ALREADY_LOADED.find(strServerNymID.Get());
    if (map_ALREADY_LOADED.end() == it_temp)
        map_ALREADY_LOADED.insert(std::pair<std::string, Nym*>(
            strServerNymID.Get(),
            pServerNym));  // Add Server Nym to list of Nyms already loaded.
    it_temp = map_ALREADY_LOADED.find(strSenderNymID.Get());
    if (map_ALREADY_LOADED.end() == it_temp)
        map_ALREADY_LOADED.insert(std::pair<std::string, const Nym*>(
            strSenderNymID.Get(),
            pSenderNym));  // Add Sender Nym to list of Nyms already loaded.
    it_temp = map_ALREADY_LOADED.find(strRecipientNymID.Get());
    if (map_ALREADY_LOADED.end() == it_temp)
        map_ALREADY_LOADED.insert(std::pair<std::string, const Nym*>(
            strRecipientNymID.Get(), pRecipientNym));  // Add Recipient Nym to
                                                       // list of Nyms already
                                                       // loaded.
    //
    //    I set up map_ALREADY_LOADED here so that when I call
    // VerifyAgentAsNym(), I can pass it along. VerifyAgentAsNym often
    //  just verifies ownership (for individual nym owners who act as their own
    // agents.) BUT... If we're in a smart contract,
    //  or other OTScriptable, there might be a party owned by an entity, and a
    // signature needs to be checked that ISN'T the
    //  same Nym! (Perhaps the entity signed the smartcontract via another
    // signer Nym.) This means I might potentially have to
    //  LOAD the other signer Nym, in order to verify his signature...BUT WHAT
    // IF HE'S ALREADY LOADED?
    //
    //  THAT... is exactly why I am passing in a list now of all the Nyms that
    // are already loaded... So if the authorizing Nym for
    //  any given party happens to already be loaded on that list, it can use
    // it, instead of loading it twice (and potentially
    //  overwriting data... Bad thing!)
    //
    // Now that I have the original cron item loaded, and all the Nyms ready to
    // go,
    // let's make sure that BOTH the nyms in question have SIGNED the original
    // request.
    // (Their signatures wouldn't be on the updated version in Cron--the server
    // signs
    // that one. Except smart contracts, which keep a signed copy automatically
    // for each party.)
    //
    // NOTE: originally I used to just verify here that both Nyms have signed
    // the original
    // cron item. But now I provide a virtual method to do that (which it still
    // does, by default.)
    // But in the case of smart contracts, it's more complicated. The Nym might
    // merely be an agent
    // for a party, (legitimately) but where a DIFFERENT agent is the one who
    // originally signed.
    // Thus, I have to allow SmartContract to override the method so it can add
    // that extra intelligence.
    //
    // NOTE: This function does NOT verify that the Nyms are authorized for the
    // ASSET ACCOUNTS--either the
    // party providing authorization OR the acct itself. Instead, this ONLY
    // verifies that they are actually
    // agents for legitimate parties to this agreement ACCORDING TO THOSE
    // PARTIES, (regardless of the asset
    // accounts) and that said agreement has also been signed by authorized
    // agents of those parties.
    // (Usually the agent who originally signed, and the agent signing now, and
    // the
    // party that agent represents, are all in reality the SAME NYM. But in the
    // case of
    // smart contracts, they can be different Nyms, which is why we now have an
    // overridable virtual method
    // for this, instead of simply verifying both signatures on the cron item
    // itself, as the default
    // OTTrade and OTAgreement versions of that virtual method will continue to
    // do.
    //
    // Repeat: this is not about verifying account ownershp or permissions. It's
    // ONLY about verifying
    // that these nyms are actually authorized to act as parties to the
    // agreement. Verifying their ownership
    // rights over the asset accounts is done separately below.
    //
    // Another thing: The original "VerifySignature()" proved that the NYM
    // HIMSELF had signed/authorized,
    // whereas now it's more like, prove which party owns the account, then
    // prove that party has authorized
    // Nym as his agent.
    //

    // VerifyNymAsAgent() replaces VerifySignature() here. It's polymorphic, so
    // VerifySignature() is still called
    // directly on theNym in certain versions (agreement, trade...) but
    // otherwise there is now more to it than that.
    // See OTScriptable for the complicated version. Either way, it makes sure
    // that the right person signed and that
    // theNym is authorized to act by that person. (As far as pOrigCronItem
    // cares -- the Account may still disagree.)
    // This verifies:
    // - that the Nym is found as an agent on one of the parties.
    // - that there is an "original party signed copy" of the contract attached
    // to the party.
    // - that there is an authorizing agent for that party whose SIGNATURE
    // VERIFIES on the party's signed copy.
    //
    if (!pOrigCronItem->VerifyNymAsAgent(
            *pSenderNym,
            *pServerNym,
            // In case it needs to load the AUTHORIZING agent, and that agent
            // is already loaded, it will have access here.
            &map_ALREADY_LOADED)) {
        otErr << "OTCronItem::MoveFunds: Failed authorization for sender Nym: "
              << strSenderNymID << "\n";
        FlagForRemoval();  // Remove it from Cron.
        return false;
    }

    if (!pOrigCronItem->VerifyNymAsAgent(
            *pRecipientNym,
            *pServerNym,
            // In case it needs to load the AUTHORIZING agent, and that agent
            // is already loaded, it will have access here.
            &map_ALREADY_LOADED)) {
        otErr
            << "OTCronItem::MoveFunds: Failed authorization for recipient Nym: "
            << strRecipientNymID << "\n";
        FlagForRemoval();  // Remove it from Cron.
        return false;
    }

    // (verifications)
    //
    // -- DONE Verify that both Nyms are actually authorized to act as agents
    // for the parties. (in the cron item.) This is like
    // pOrig->VerifySignature(theNym);
    // -- DONE Verify that both parties have properly signed/executed the
    // contract. This may mean loading a DIFFERENT Nym (the authorizing agent
    // who signed for the party originally)
    //    This is also like pOrig->VerifySignature(theNym); the extra
    // functionality is added via polymorphism.
    // -- DONE Verify that both Nyms are authorized agents FOR THE ASSET
    // ACCOUNTS IN QUESTION, according to their PARTIES. This is like
    // pParty->HasAgent(theNym);
    // -- DONE Verify that both ACCOUNTS are owned by the parties that the Nyms
    // represent.  According to the ACCOUNTS. This is like
    // pAcct->VerifyOwner(theNym)
    //    The above two items should ALSO be done polymorphically, just like the
    // first two.
    // -- If the Nyms are not the parties they represent, then verify that they
    // have the proper Role at their entities in order to claim such authority.
    //    Does this last one go into OTAccount itself? Or OTScriptable. And the
    // actual verification part will be probably in OTAgent, just like
    // VerifySignature was.
    //
    //    if (!pOrigCronItem->VerifySignature(*pSenderNym) ||
    // !pOrigCronItem->VerifySignature(*pRecipientNym))
    //
    //    {
    //        otErr << "OTCronItem::MoveFunds: Failed authorization.\n";
    //        FlagForRemoval(); // Remove it from Cron.
    //        return false;
    //    }

    // AT THIS POINT, I have pServerNym, pSenderNym, and pRecipientNym.
    // ALL are loaded from disk (where necessary.) AND...
    // ALL are valid pointers, (even if they sometimes point to the same
    // object,)
    // AND none are capable of overwriting the storage of the other (by
    // accidentally
    // loading the same storage twice.)
    // We also have boolean variables at this point to tell us exactly which are
    // which,
    // (in case some of those pointers do go to the same object.)
    // They are:
    // bSenderNymIsServerNym, bRecipientNymIsServerNym, and bUsersAreSameNym.
    //
    // I also have pOrigCronItem, which is a dynamically-allocated copy of the
    // original
    // Cron Receipt for this Cron Item. (And I don't need to worry about
    // deleting it, either.)
    // I know for a fact they are both authorized on pOrigCronItem...

    // LOAD THE ACCOUNTS
    //
    Account* pSourceAcct =
        Account::LoadExistingAccount(SOURCE_ACCT_ID, NOTARY_ID);

    if (nullptr == pSourceAcct) {
        otOut << "OTCronItem::MoveFunds: ERROR verifying existence of source "
                 "account.\n";
        FlagForRemoval();  // Remove it from future Cron processing, please.
        return false;
    }
    // Past this point we know pSourceAcct is good and will clean itself up.
    std::unique_ptr<Account> theSourceAcctSmrtPtr(pSourceAcct);

    Account* pRecipientAcct =
        Account::LoadExistingAccount(RECIPIENT_ACCT_ID, NOTARY_ID);

    if (nullptr == pRecipientAcct) {
        otOut << "OTCronItem::MoveFunds: ERROR verifying existence of "
                 "recipient account.\n";
        FlagForRemoval();  // Remove it from future Cron processing, please.
        return false;
    }
    // Past this point we know pRecipientAcct is good and will clean itself up.
    std::unique_ptr<Account> theRecipAcctSmrtPtr(pRecipientAcct);

    // BY THIS POINT, both accounts are successfully loaded, and I don't have to
    // worry about
    // cleaning either one of them up, either. But I can now use pSourceAcct and
    // pRecipientAcct...

    // A few verification if/elses...

    // Are both accounts of the same Asset Type?
    if (pSourceAcct->GetInstrumentDefinitionID() !=
        pRecipientAcct->GetInstrumentDefinitionID()) {  // We already know the
                                                        // SUPPOSED
        // Instrument Definition Ids of these accounts...
        // But only once
        // the accounts THEMSELVES have been loaded can we VERIFY this to be
        // true.
        otOut << "OTCronItem::MoveFunds: ERROR - attempted payment between "
                 "accounts of different "
                 "instrument definitions.\n";
        FlagForRemoval();  // Remove it from future Cron processing, please.
        return false;
    }

    // I call VerifySignature (WITH SERVER NYM) here since VerifyContractID was
    // already called in LoadExistingAccount().
    //
    else if (
        !pSourceAcct->VerifySignature(*pServerNym) ||
        !VerifyNymAsAgentForAccount(*pSenderNym, *pSourceAcct)) {
        otOut << "OTCronItem::MoveFunds: ERROR verifying signature or "
                 "ownership on source account.\n";
        FlagForRemoval();  // Remove it from future Cron processing, please.
        return false;
    } else if (
        !pRecipientAcct->VerifySignature(*pServerNym) ||
        !VerifyNymAsAgentForAccount(*pRecipientNym, *pRecipientAcct)) {
        otOut << "OTCronItem::MoveFunds: ERROR verifying signature or "
                 "ownership on recipient account.\n";
        FlagForRemoval();  // Remove it from future Cron processing, please.
        return false;
    }

    // By this point, I know I have both accounts loaded, and I know that they
    // have the right instrument definitions,
    // and I know they have the right owners and they were all signed by the
    // server.
    // I also know that their account IDs in their internal records matched the
    // account filename for each acct.
    // I also have pointers to the Nyms who own these accounts.
    else {
        // Okay then, everything checks out. Let's add a receipt to the sender's
        // inbox and the recipient's inbox.
        // IF they can be loaded up from file, or generated, that is.

        // Load the inboxes in case they already exist
        Ledger theSenderInbox(SENDER_NYM_ID, SOURCE_ACCT_ID, NOTARY_ID),
            theRecipientInbox(RECIPIENT_NYM_ID, RECIPIENT_ACCT_ID, NOTARY_ID);

        // ALL inboxes -- no outboxes. All will receive notification of
        // something ALREADY DONE.
        bool bSuccessLoadingSenderInbox = theSenderInbox.LoadInbox();
        bool bSuccessLoadingRecipientInbox = theRecipientInbox.LoadInbox();

        // ...or generate them otherwise...

        if (true == bSuccessLoadingSenderInbox)
            bSuccessLoadingSenderInbox =
                theSenderInbox.VerifyAccount(*pServerNym);
        else
            otErr << "OTCronItem::MoveFunds: ERROR loading sender inbox "
                     "ledger.\n";
        //        else
        //            bSuccessLoadingSenderInbox        =
        // theSenderInbox.GenerateLedger(SOURCE_ACCT_ID, NOTARY_ID,
        // OTLedger::inbox, true); // bGenerateFile=true

        if (true == bSuccessLoadingRecipientInbox)
            bSuccessLoadingRecipientInbox =
                theRecipientInbox.VerifyAccount(*pServerNym);
        else
            otErr << "OTCronItem::MoveFunds: ERROR loading recipient inbox "
                     "ledger.\n";
        //        else
        //            bSuccessLoadingRecipientInbox    =
        // theRecipientInbox.GenerateLedger(RECIPIENT_ACCT_ID, NOTARY_ID,
        // OTLedger::inbox, true); // bGenerateFile=true

        if ((false == bSuccessLoadingSenderInbox) ||
            (false == bSuccessLoadingRecipientInbox)) {
            otErr << "OTCronItem::MoveFunds: ERROR loading or generating one "
                     "(or both) of the inbox ledgers.\n";
        } else {
            // Generate new transaction numbers for these new transactions
            std::int64_t lNewTransactionNumber =
                GetCron()->GetNextTransactionNumber();

            //          OT_ASSERT(lNewTransactionNumber > 0); // this can be my
            //          reminder.
            if (0 == lNewTransactionNumber) {
                otOut << "OTCronItem::MoveFunds: Aborted move: There are no "
                         "more transaction numbers available.\n";
                // (Here I do NOT flag for removal.)
                return false;
            }

            OTTransaction* pTransSend = OTTransaction::GenerateTransaction(
                theSenderInbox,
                OTTransaction::paymentReceipt,
                originType::origin_smart_contract,
                lNewTransactionNumber);

            OTTransaction* pTransRecip = OTTransaction::GenerateTransaction(
                theRecipientInbox,
                OTTransaction::paymentReceipt,
                originType::origin_smart_contract,
                lNewTransactionNumber);

            // (No need to OT_ASSERT on the above transactions since it occurs
            // in GenerateTransaction().)

            // Both inboxes will get receipts with the same (new) transaction ID
            // on them.
            // They will have a "In reference to" field containing the original
            // payment plan
            // (with user's signature).

            // set up the transaction items (each transaction may have multiple
            // items... but not in this case.)
            Item* pItemSend = Item::CreateItemFromTransaction(
                *pTransSend, Item::paymentReceipt);
            Item* pItemRecip = Item::CreateItemFromTransaction(
                *pTransRecip, Item::paymentReceipt);

            // these may be unnecessary, I'll have to check
            // CreateItemFromTransaction. I'll leave em.
            OT_ASSERT(nullptr != pItemSend);
            OT_ASSERT(nullptr != pItemRecip);

            pItemSend->SetStatus(Item::rejection);   // the default.
            pItemRecip->SetStatus(Item::rejection);  // the default.

            const std::int64_t lTransSendRefNo =
                GetOpeningNumber(SENDER_NYM_ID);
            const std::int64_t lTransRecipRefNo =
                GetOpeningNumber(RECIPIENT_NYM_ID);

            // Here I make sure that each receipt (each inbox notice) references
            // the original
            // transaction number that was used to set the cron item into
            // place...
            // This number is used to track all cron items. (All Cron items
            // require a transaction
            // number from the user in order to add them to Cron in the first
            // place.)
            //
            // The number is also used to uniquely identify all other
            // transactions, as you
            // might guess from its name.
            //
            // UPDATE: Notice I'm now looking up different numbers based on the
            // NymIDs.
            // This is to support smart contracts, which have many parties,
            // agents, and accounts.
            //
            //            pItemSend->SetReferenceToNum(lTransSendRefNo);
            //            pItemRecip->SetReferenceToNum(lTransRecipRefNo);

            pTransSend->SetReferenceToNum(lTransSendRefNo);
            pTransRecip->SetReferenceToNum(lTransRecipRefNo);

            // The TRANSACTION (a receipt in my inbox) will be sent with "In
            // Reference To" information
            // containing the ORIGINAL SIGNED CRON ITEM. (With both parties'
            // original signatures on it.)
            //
            // Whereas the TRANSACTION ITEM will include an "attachment"
            // containing the UPDATED
            // CRON ITEM (this time with the SERVER's signature on it.)
            //
            // Here's the original one going onto the transaction:
            //
            pTransSend->SetReferenceString(strOrigPlan);
            pTransRecip->SetReferenceString(strOrigPlan);

            // MOVE THE DIGITAL ASSETS FROM ONE ACCOUNT TO ANOTHER...

            // Calculate the amount and debit/ credit the accounts
            // Make sure each Account can afford it, and roll back in case of
            // failure.

            // Make sure he can actually afford it...
            if (pSourceAcct->GetBalance() >= lAmount) {
                // Debit the source account.
                bool bMoveSender = pSourceAcct->Debit(lAmount);
                bool bMoveRecipient = false;

                // IF success, credit the recipient.
                if (bMoveSender) {
                    bMoveRecipient =
                        pRecipientAcct->Credit(lAmount);  // <=== CREDIT FUNDS

                    // Okay, we already took it from the source account.
                    // But if we FAIL to credit the recipient, then we need to
                    // PUT IT BACK in the source acct.
                    // (EVEN THOUGH we'll just "NOT SAVE" after any failure, so
                    // it's really superfluous.)
                    //
                    if (!bMoveRecipient)
                        pSourceAcct->Credit(lAmount);  // put the money back
                    else
                        bSuccess = true;
                }

                // If ANY of these failed, then roll them all back and break.
                if (!bMoveSender || !bMoveRecipient) {
                    otErr << "OTCronItem::MoveFunds: Very strange! Funds were "
                             "available but "
                             "debit or credit failed while performing move.\n";
                    // We won't save the files anyway, if this failed.
                    bSuccess = false;
                }
            }

            // DO NOT SAVE ACCOUNTS if bSuccess is false.
            // We only save these accounts if bSuccess == true.
            // (But we do save the inboxes either way, since payment failures
            // always merit an inbox notice.)

            if (true == bSuccess)  // The payment succeeded.
            {
                // Both accounts involved need to get a receipt of this trade in
                // their inboxes...
                pItemSend->SetStatus(Item::acknowledgement);   // pSourceAcct
                pItemRecip->SetStatus(Item::acknowledgement);  // pRecipientAcct

                pItemSend->SetAmount(lAmount * (-1));  // "paymentReceipt" is
                                                       // otherwise ambigious
                                                       // about whether you are
                                                       // paying or being paid.
                pItemRecip->SetAmount(
                    lAmount);  // So, I decided for payment and
                               // market receipts, to use
                               // negative and positive
                               // amounts.
                // I will probably do the same for cheques, since they can be
                // negative as well (invoices).

                otLog3 << "OTCronItem::MoveFunds: Move performed.\n";

                // (I do NOT save m_pCron here, since that already occurs after
                // this function is called.)
            } else  // bSuccess = false.  The payment failed.
            {
                pItemSend->SetStatus(Item::rejection);   // pSourceAcct
                                                         // // These are already
                                                         // initialized to
                                                         // false.
                pItemRecip->SetStatus(Item::rejection);  // pRecipientAcct
                                                         // // (But just making
                                                         // sure...)

                pItemSend->SetAmount(
                    0);  // No money changed hands. Just being explicit.
                pItemRecip->SetAmount(
                    0);  // No money changed hands. Just being explicit.

                otLog3 << "OTCronItem::MoveFunds: Move failed.\n";
            }

            // Everytime a payment processes, a receipt is put in the user's
            // inbox, containing a
            // CURRENT copy of the cron item (which took just money from the
            // user's acct, or not,
            // and either way thus updated its status -- so its internal data
            // has changed.)
            //
            // It will also contain a copy of the user's ORIGINAL signed cron
            // item, where the data
            // has NOT changed, (so the user's original signature is still
            // good.)
            //
            // In order for it to export the RIGHT VERSION of the CURRENT plan,
            // which has just changed
            // (above), then I need to re-sign it and save it first. (The
            // original version I'll load from
            // a separate file using
            // OTCronItem::LoadCronReceipt(lTransactionNum).
            //
            // I should be able to call a method on the original cronitem, where
            // I ask it to verify a certain
            // nym as being acceptable to that cron item as an agent, based on
            // the signature of the original
            // authorizing agent for that party.
            //

            ReleaseSignatures();
            SignContract(*pServerNym);
            SaveContract();

            // This is now at the bottom of this function.
            //
            // m_pCron->SaveCron(); // Cron is where I am serialized, so if
            // Cron's not saved, I'm not saved.

            //
            // EVERYTHING BELOW is just about notifying the users, by dropping
            // the receipt in their
            // inboxes. The rest is done.  The accounts and inboxes will all be
            // saved at the same time.
            //
            // The Payment Plan is entirely updated and saved by this point, and
            // Cron will
            // also be saved in the calling function once we return (no matter
            // what.)
            //

            // Basically I load up both INBOXES, which are actually LEDGERS, and
            // then I create
            // a new transaction, with a new transaction item, for each of the
            // ledgers.
            // (That's where the receipt information goes.)
            //

            // The TRANSACTION will be sent with "In Reference To" information
            // containing the
            // ORIGINAL SIGNED PLAN. (With both of the users' original
            // signatures on it.)
            //
            // Whereas the TRANSACTION ITEM will include an "attachment"
            // containing the UPDATED
            // PLAN (this time with the SERVER's signature on it.)

            // (Lucky I just signed and saved the updated plan (above), or it
            // would still have
            // have the old data in it.)

            // I also already loaded the original plan. Remember this from
            // above,
            // near the top of the function:
            //  OTCronItem * pOrigCronItem    = nullptr;
            //     OTString strOrigPlan(*pOrigCronItem); // <====== Farther down
            // in the code, I attach this string to the receipts.
            //  ... then lower down...
            // pTransSend->SetReferenceString(strOrigPlan);
            // pTransRecip->SetReferenceString(strOrigPlan);
            //
            // So the original plan is already loaded and copied to the
            // Transaction as the "In Reference To"
            // Field. Now let's add the UPDATED plan as an ATTACHMENT on the
            // Transaction ITEM:
            //
            String strUpdatedCronItem(*this);

            // Set the updated cron item as the attachment on the transaction
            // item.
            // (With the SERVER's signature on it!)
            // (As a receipt for each party, so they can see their smartcontract
            // updating.)
            //
            pItemSend->SetAttachment(strUpdatedCronItem);
            pItemRecip->SetAttachment(strUpdatedCronItem);

            // Success OR failure, either way I want a receipt in both inboxes.
            // But if FAILURE, I do NOT want to save the Accounts, JUST the
            // inboxes.
            // So the inboxes happen either way, but the accounts are saved only
            // on success.

            // sign the item
            pItemSend->SignContract(*pServerNym);
            pItemRecip->SignContract(*pServerNym);

            pItemSend->SaveContract();
            pItemRecip->SaveContract();

            // the Transaction "owns" the item now and will handle cleaning it
            // up.
            pTransSend->AddItem(*pItemSend);
            pTransRecip->AddItem(*pItemRecip);

            pTransSend->SignContract(*pServerNym);
            pTransRecip->SignContract(*pServerNym);

            pTransSend->SaveContract();
            pTransRecip->SaveContract();

            // Here, the transactions we just created are actually added to the
            // ledgers.
            // This happens either way, success or fail.

            theSenderInbox.AddTransaction(*pTransSend);
            theRecipientInbox.AddTransaction(*pTransRecip);

            // Release any signatures that were there before (They won't
            // verify anymore anyway, since the content has changed.)
            theSenderInbox.ReleaseSignatures();
            theRecipientInbox.ReleaseSignatures();

            // Sign both of them.
            theSenderInbox.SignContract(*pServerNym);
            theRecipientInbox.SignContract(*pServerNym);

            // Save both of them internally
            theSenderInbox.SaveContract();
            theRecipientInbox.SaveContract();

            // Save both inboxes to storage. (File, DB, wherever it goes.)
            pSourceAcct->SaveInbox(theSenderInbox);
            pRecipientAcct->SaveInbox(theRecipientInbox);

            // These correspond to the AddTransaction() calls, just above
            //
            pTransSend->SaveBoxReceipt(theSenderInbox);
            pTransRecip->SaveBoxReceipt(theRecipientInbox);

            // If success, save the accounts with new balance. (Save inboxes
            // with receipts either way,
            // and the receipts will contain a rejection or acknowledgment
            // stamped by the Server Nym.)
            if (true == bSuccess) {

                // Release any signatures that were there before (They won't
                // verify anymore anyway, since the content has changed.)
                pSourceAcct->ReleaseSignatures();
                pRecipientAcct->ReleaseSignatures();

                // Sign both of them.
                pSourceAcct->SignContract(*pServerNym);
                pRecipientAcct->SignContract(*pServerNym);

                // Save both of them internally
                pSourceAcct->SaveContract();
                pRecipientAcct->SaveContract();

                // TODO: Better rollback capabilities in case of failures here:

                // Save both accounts to storage.
                pSourceAcct->SaveAccount();
                pRecipientAcct->SaveAccount();

                // NO NEED TO LOG HERE, since success / failure is already
                // logged above.
            }
        }  // both inboxes were successfully loaded or generated.
    }  // By the time we enter this block, accounts and nyms are already loaded.
       // As we begin, inboxes are instantiated.

    // Todo: possibly notify ALL parties here (in Nymbox.)

    // Either way, Cron should save, since it just updated.
    // The above function WILL change this smart contract.
    // and re-sign it and save it, no matter what. So I just
    // call this here to keep it simple:

    GetCron()->SaveCron();

    return bSuccess;
}

}  // namespace opentxs
