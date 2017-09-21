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

#if OT_CRYPTO_USING_LIBBITCOIN

#include <string>
#include <boost/algorithm/hex.hpp>

#include <bitcoin/bitcoin/chain/script.hpp>
#include <bitcoin/bitcoin/chain/transaction.hpp>
#include <bitcoin/bitcoin/chain/output.hpp>
#include <bitcoin/bitcoin/machine/operation.hpp>
#include <bitcoin/bitcoin/math/elliptic_curve.hpp>

#include "opentxs/core/stdafx.hpp"

#include "opentxs/core/crypto/Bip47.hpp"

#include "opentxs/core/Data.hpp"

namespace libbitcoin
{

namespace machine
{

bool is_public_key_pattern(const operation::list& ops);
bool is_key_hash_pattern(const operation::list& ops);
bool is_multisig_script_hash_pattern(
    unsigned int& middle,
    const machine::operation::list& ops);

bool is_public_key_pattern(const operation::list& ops)
{
    return ops.size() == 3 && is_public_key(ops[1].data()) &&
           ops[2].code() == machine::opcode::checksig;
}

bool is_key_hash_pattern(const operation::list& ops)
{
    return ops.size() == 7 && is_public_key(ops[1].data()) &&
           ops[2].code() == opcode::dup && ops[3].code() == opcode::hash160 &&
           ops[4].data().size() == short_hash_size &&
           ops[5].code() == opcode::equalverify &&
           ops[6].code() == opcode::checksig;
}

bool is_multisig_script_hash_pattern(
    unsigned int& middle,
    const machine::operation::list& ops)
{
    static constexpr auto op_1 = static_cast<uint8_t>(opcode::push_positive_1);
    static constexpr auto op_16 =
        static_cast<uint8_t>(opcode::push_positive_16);

    const auto op_count = ops.size();

    if (op_count < 6 || ops[0].code() != opcode::push_size_0 ||
        ops[op_count - 1].code() != opcode::checkmultisig)
        return false;

    const auto op_n = static_cast<uint8_t>(ops[op_count - 2].code());

    int num_signatures = 0;
    middle = 1;
    while (true) {
        if (middle >= op_count) {
            return false;
        }

        // The first numeric op we encounter is the middle of the script.
        if (machine::operation::is_numeric(ops[middle].code())) {
            break;
        }

        // check for push data
        if (!machine::operation::is_push(ops[middle].code())) {
            return false;
        }

        num_signatures++;
        middle++;
    }

    const auto op_m = static_cast<uint8_t>(ops[middle].code());

    if (op_m < op_1 || op_m > op_n || op_n < op_1 || op_n > op_16) return false;

    const int number = op_n - op_1 + 1u;
    const int points = op_count - 3u - num_signatures - 1;

    if (number != points) return false;

    for (auto op = ops.begin() + num_signatures + 3; op != ops.end() - 2; ++op)
        if (!is_public_key(op->data())) return false;

    return true;
}

}  // machine

}  // libbitcoin

namespace opentxs
{

bool extract_designated_pubkey(
    libbitcoin::data_chunk& out,
    const libbitcoin::machine::operation::list& input,
    const libbitcoin::machine::operation::list& prevout_script);
Data* designated_pubkey(
    const libbitcoin::data_chunk& transaction,
    const std::vector<Data>& previous_transactions);

bool extract_designated_pubkey(
    libbitcoin::data_chunk& out,
    const libbitcoin::machine::operation::list& input,
    const libbitcoin::machine::operation::list& prevout_script)
{
    if (libbitcoin::chain::script::is_pay_public_key_pattern(prevout_script)) {
        if (libbitcoin::chain::script::is_sign_public_key_pattern(input)) {
            out = prevout_script[0].data();
            return true;
        }

        return false;
    }

    if (libbitcoin::chain::script::is_pay_key_hash_pattern(prevout_script)) {
        if (libbitcoin::chain::script::is_sign_key_hash_pattern(input)) {
            out = input[1].data();
            return true;
        }

        return false;
    }

    if (libbitcoin::chain::script::is_pay_multisig_pattern(prevout_script)) {
        if (libbitcoin::chain::script::is_sign_multisig_pattern(input)) {
            out = prevout_script[1].data();
            return true;
        }

        return false;
    }

    if (!libbitcoin::chain::script::is_pay_script_hash_pattern(
            prevout_script)) {
        return false;
    }

    if (is_public_key_pattern(input)) {
        out = input[1].data();
        return true;
    }

    if (is_key_hash_pattern(input)) {
        out = input[1].data();
        return true;
    }

    unsigned int middle;
    if (is_multisig_script_hash_pattern(middle, input)) {
        out = input[middle + 1].data();
        return true;
    }

    return false;
}

const libbitcoin::data_chunk bip47::ReadData(const Data& d)
{
    const uint8_t* p = reinterpret_cast<const uint8_t*>(d.GetPointer());
    return std::vector<std::uint8_t>(p, p + d.GetSize());
}

// The previous transactions are not necessarily given in order.
Data* designated_pubkey(
    const libbitcoin::data_chunk& transaction,
    const std::vector<Data>& previous_transactions)
{
    // Deserialize tx
    libbitcoin::chain::transaction tx =
        libbitcoin::chain::transaction::factory(transaction);
    if (!tx.is_valid()) {
        return nullptr;
    }

    auto inputs = tx.inputs();
    const unsigned int size = inputs.size();
    if (size < 1 || size != previous_transactions.size()) {
        return nullptr;
    }

    // A type used to iterate over the previous transactions, which might be out
    // of order.
    class matcher
    {
    public:
        const unsigned int size;
        const std::vector<Data>& previous_transactions;

        // Keep track of which txs have been matched.
        std::vector<bool> matched;

        // Keep track of which txs have been seen;
        std::vector<bool> seen;

        std::vector<libbitcoin::chain::transaction> txs;
        std::vector<libbitcoin::hash_digest> hashes;

        libbitcoin::chain::output get(
            libbitcoin::chain::output_point previous_output)
        {
            for (unsigned int i = 0; i < size; i++) {
                // skip txs that have already been matched.
                if (matched[i]) continue;

                // The previous tx corresponding to this index.
                libbitcoin::chain::transaction prev;

                // The hash of the previous tx corresponding to this index.
                libbitcoin::hash_digest id;

                if (seen[i]) {
                    prev = txs[i];
                    id = hashes[i];
                } else {
                    prev = libbitcoin::chain::transaction::factory(
                        bip47::ReadData(previous_transactions[i]));
                    id = prev.hash();

                    // libbitcoin::reverse_hash(id);
                }

                // If the hashes don't match, save these and try the next one.
                if (id != previous_output.hash()) {
                    txs[i] = prev;
                    hashes[i] = id;
                    seen[i] = true;

                    continue;
                }

                matched[i] = true;

                auto outputs = prev.outputs();
                if (previous_output.index() < outputs.size()) {
                    return prev.outputs()[previous_output.index()];
                }

                // Error case if the input doesn't correspond to an output.
                return libbitcoin::chain::output();
            }

            // Invalid output if we go through the whole list and don't find
            // what we're looking for.
            return libbitcoin::chain::output();
        }

        matcher(int z, const std::vector<Data>& p)
            : size(z)
            , previous_transactions(p)
            , matched(std::vector<bool>(size))
            , seen(std::vector<bool>(size))
            , txs(std::vector<libbitcoin::chain::transaction>(size))
            , hashes(std::vector<libbitcoin::hash_digest>(size))
        {
            for (int i = 0; i < z; i++) {
                matched[i] = false;
                seen[i] = false;
            }
        }
    };

    matcher m(size, previous_transactions);

    libbitcoin::data_chunk out;

    // Go through inputs of this transaction.
    for (std::vector<libbitcoin::chain::input>::iterator input = inputs.begin();
         input != inputs.end();
         ++input) {

        // Get the previous output corresponding to this input.
        libbitcoin::chain::output output = m.get(input->previous_output());

        // if the output is invalid, something went wrong.
        if (!output.is_valid()) {
            return nullptr;
        }

        if (extract_designated_pubkey(
                out,
                input->script().operations(),
                output.script().operations())) {
            return new Data(out);
        }
    }

    return nullptr;
}

std::unique_ptr<Data> DesignatedPubkey(
    const Data& transaction,
    const std::vector<Data>& previous_transactions)
{
    return std::unique_ptr<Data>(
        designated_pubkey(bip47::ReadData(transaction), previous_transactions));
}

}  // opentxs

#endif  // OT_CRYPTO_USING_LIBBITCOIN
