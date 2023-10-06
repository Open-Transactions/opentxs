// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/params/ChainDataPrivate.hpp"  // IWYU pragma: associated

#include <boost/container/vector.hpp>
#include <boost/json.hpp>
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <memory>

#include "blockchain/params/Json.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/Factory.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/cfilter/FilterType.hpp"  // IWYU pragma: keep
#include "opentxs/util/Log.hpp"
#include "util/Container.hpp"

namespace opentxs::blockchain::params
{
ChainDataPrivate::ChainDataPrivate(
    const boost::json::object& json,
    const Data& data,
    blockchain::Type chain) noexcept
    : chain_(chain)
    , data_(data)
    , associated_mainnet_(data_.associated_mainnet_)
    , category_(data_.category_)
    , forked_from_(data_.forked_from_)
    , supported_(data_.supported_)
    , testnet_(data_.testnet_)
    , segwit_(data_.segwit_)
    , cashtoken_(data_.cashtoken_)
    , spend_unconfirmed_(data_.spend_unconfirmed_)
    , segwit_scale_factor_(data_.segwit_scale_factor_)
    , currency_type_(data_.itemtype_)
    , bip44_(data_.bip44_)
    , difficulty_(data_.n_bits_)
    , genesis_hash_(block::Hash{IsHex, data.genesis_hash_hex_})
    , serialized_genesis_block_(IsHex, data_.genesis_block_hex_)
    , genesis_cfheader_([&] {
        auto out = GenesisCfheader{};
        const auto& map = data.genesis_bip158_;
        std::ranges::transform(
            map, std::inserter(out, out.end()), [](const auto& value) {
                const auto& [type, bytes] = value;
                const auto& [cfheader, _] = bytes;

                return std::make_pair(type, cfilter::Header{IsHex, cfheader});
            });

        return out;
    }())
    , known_cfilter_types_([&] {
        auto out = Set<cfilter::Type>{};
        std::ranges::transform(
            genesis_cfheader_,
            std::inserter(out, out.end()),
            [](const auto& value) { return value.first; });

        return out;
    }())
    , checkpoint_(
          json.at("checkpoint").at("position").at("height").as_int64(),
          block::Hash{
              IsHex,
              json.at("checkpoint")
                  .at("position")
                  .at("hash")
                  .as_string()
                  .c_str()})
    , checkpoint_prior_(
          json.at("checkpoint").at("previous").at("height").as_int64(),
          block::Hash{
              IsHex,
              json.at("checkpoint")
                  .at("previous")
                  .at("hash")
                  .as_string()
                  .c_str()})
    , checkpoint_cfheader_(cfilter::Header{
          IsHex,
          json.at("checkpoint").at("cfheader").as_string().c_str()})
    , default_filter_type_(data.default_filter_type_)
    , p2p_protocol_(data.p2p_protocol_)
    , p2p_protocol_version_(data.p2p_protocol_version_)
    , p2p_magic_bits_(data.p2p_magic_bits_)
    , default_port_(data.default_port_)
    , dns_seeds_([&] {
        auto out = Vector<std::string_view>{};
        const auto& in = data.dns_seeds_;
        std::ranges::copy(in, std::back_inserter(out));

        return out;
    }())
    , default_fee_rate_(data.default_fee_rate_)
    , block_download_batch_(data.block_download_batch_)
    , maturation_interval_(data.maturation_interval_)
    , cfilter_element_count_estimate_(data.cfilter_element_count_estimate_)
    , scripts_(data.scripts_)
    , default_script_(data.default_script_)
    , services_(data.services_)
    , services_reverse_(reverse_arbitrary_map<
                        Data::ServiceMap::key_type,
                        Data::ServiceMap::mapped_type,
                        Data::ServiceMapReverse,
                        Data::ServiceMap>(services_))
    , bip158_(data.bip158_)
    , bip158_reverse_(reverse_arbitrary_map<
                      Data::Bip158::key_type,
                      Data::Bip158::mapped_type,
                      Data::Bip158Reverse,
                      Data::Bip158>(bip158_))
    , zmq_(std::make_pair(data.parent_bip44_, data.subchain_))
    , max_notifications_(data.max_notifications_)
    , cfheaders_([&] {
        auto out = CfheaderCheckpoints{};

        if (const auto* i = json.find("predefined"); json.end() != i) {
            for (const auto& cp : i->value().as_array()) {
                const auto& checkpoint = cp.as_object();
                auto& [block, map] = out[static_cast<block::Height>(
                    checkpoint.at("height").as_int64())];
                const auto rc =
                    block.DecodeHex(checkpoint.at("block").as_string().c_str());

                assert_true(rc);

                using enum cfilter::Type;
                static const auto types = {Basic_BIP158, Basic_BCHVariant, ES};

                for (const auto& type : types) {
                    const auto key =
                        std::to_string(static_cast<std::uint32_t>(type));

                    if (const auto* j = checkpoint.find(key);
                        checkpoint.end() != j) {
                        const auto& cfheader = j->value().as_string();
                        map.try_emplace(type, IsHex, cfheader.c_str());
                    }
                }
            }
        }

        if (false == out.contains(0)) {
            auto& [block, map] = out[0];
            const auto rc = block.DecodeHex(data.genesis_hash_hex_);

            assert_true(rc);

            for (const auto& [type, genesis] : data.genesis_bip158_) {
                const auto& [cfheader, cfilter] = genesis;
                map.try_emplace(type, IsHex, cfheader);
            }
        }

        return out;
    }())
    , genesis_block_()
    , genesis_cfilters_()
{
}

ChainDataPrivate::ChainDataPrivate(blockchain::Type chain) noexcept
    : ChainDataPrivate(
          [&]() -> const auto& {
              const auto id = std::to_string(static_cast<std::uint32_t>(chain));

              return json().at(id).as_object();
          }(),
          Chains().at(chain),
          chain)
{
}

auto ChainDataPrivate::add_to_json(
    std::string_view in,
    boost::json::object& out) noexcept -> void
{
    auto parser = boost::json::stream_parser{};
    parser.write(in.data(), in.size());
    const auto json = parser.release();

    for (const auto& [key, value] : json.as_object()) {
        out[key].emplace_object() = value.as_object();
    }
}

auto ChainDataPrivate::GenesisBlock(const api::Crypto& crypto) const noexcept
    -> const block::Block&
{
    auto handle = genesis_block_.lock();
    auto& block = *handle;

    if (false == block.IsValid()) {
        block = factory::BlockchainBlock(
            crypto, chain_, serialized_genesis_block_.Bytes(), {});

        assert_true(block.IsValid());
        assert_true(0 == block.Header().Position().height_);
    }

    return block;
}

auto ChainDataPrivate::GenesisCfilter(
    const api::Session& api,
    cfilter::Type type) const noexcept -> const cfilter::GCS&
{
    static const auto blank = cfilter::GCS{};
    auto handle = genesis_cfilters_.lock();
    auto& map = *handle;

    if (auto i = map.find(type); map.end() != i) {

        return i->second;
    } else {
        const auto& raw = data_.genesis_bip158_;

        if (const auto r = raw.find(type); raw.end() != r) {
            const auto [j, inserted] = map.try_emplace(
                type,
                factory::GCS(
                    api,
                    type,
                    blockchain::internal::BlockHashToFilterKey(
                        genesis_hash_.Bytes()),
                    ByteArray{IsHex, r->second.second}.Bytes(),
                    {}));

            if (inserted) {

                return j->second;
            } else {

                return blank;
            }
        } else {

            return blank;
        }
    }
}

auto ChainDataPrivate::json() noexcept -> const boost::json::object&
{
    static const auto data = [] {
        auto out = boost::json::object{};
        add_to_json(ada_json(), out);
        add_to_json(algo_json(), out);
        add_to_json(ar_json(), out);
        add_to_json(atom_json(), out);
        add_to_json(avax_json(), out);
        add_to_json(bch_json(), out);
        add_to_json(bnb_json(), out);
        add_to_json(bsc_json(), out);
        add_to_json(bsv_json(), out);
        add_to_json(btc_json(), out);
        add_to_json(btg_json(), out);
        add_to_json(bts_json(), out);
        add_to_json(celo_json(), out);
        add_to_json(chz_json(), out);
        add_to_json(cspr_json(), out);
        add_to_json(dash_json(), out);
        add_to_json(dcr_json(), out);
        add_to_json(doge_json(), out);
        add_to_json(dot_json(), out);
        add_to_json(egld_json(), out);
        add_to_json(eos_json(), out);
        add_to_json(etc_json(), out);
        add_to_json(ethgoerli_json(), out);
        add_to_json(ethholesovice_json(), out);
        add_to_json(eth_json(), out);
        add_to_json(ethkovan_json(), out);
        add_to_json(ethmorden_json(), out);
        add_to_json(etholympic_json(), out);
        add_to_json(ethrinkeby_json(), out);
        add_to_json(ethropsten_json(), out);
        add_to_json(ethsepolia_json(), out);
        add_to_json(exp_json(), out);
        add_to_json(fct_json(), out);
        add_to_json(fil_json(), out);
        add_to_json(flow_json(), out);
        add_to_json(ftm_json(), out);
        add_to_json(hbar_json(), out);
        add_to_json(hnt_json(), out);
        add_to_json(ht_json(), out);
        add_to_json(icp_json(), out);
        add_to_json(icx_json(), out);
        add_to_json(kda_json(), out);
        add_to_json(klay_json(), out);
        add_to_json(ksm_json(), out);
        add_to_json(ltc_json(), out);
        add_to_json(luna_json(), out);
        add_to_json(maid_json(), out);
        add_to_json(matic_json(), out);
        add_to_json(mina_json(), out);
        add_to_json(miota_json(), out);
        add_to_json(near_json(), out);
        add_to_json(neo_json(), out);
        add_to_json(nxt_json(), out);
        add_to_json(okb_json(), out);
        add_to_json(one_json(), out);
        add_to_json(pkt_json(), out);
        add_to_json(poa_json(), out);
        add_to_json(qnt_json(), out);
        add_to_json(qtum_json(), out);
        add_to_json(rune_json(), out);
        add_to_json(rvn_json(), out);
        add_to_json(sc_json(), out);
        add_to_json(scrt_json(), out);
        add_to_json(sol_json(), out);
        add_to_json(steem_json(), out);
        add_to_json(stx_json(), out);
        add_to_json(tfuel_json(), out);
        add_to_json(theta_json(), out);
        add_to_json(tn4bch_json(), out);
        add_to_json(tnbch_json(), out);
        add_to_json(tnbsv_json(), out);
        add_to_json(tnbtc_json(), out);
        add_to_json(tncspr_json(), out);
        add_to_json(tndash_json(), out);
        add_to_json(tndoge_json(), out);
        add_to_json(tnltc_json(), out);
        add_to_json(tnmaid_json(), out);
        add_to_json(tnnem_json(), out);
        add_to_json(tnnxt_json(), out);
        add_to_json(tnpkt_json(), out);
        add_to_json(tnpoa_json(), out);
        add_to_json(tnsc_json(), out);
        add_to_json(tnsteem_json(), out);
        add_to_json(tnwaves_json(), out);
        add_to_json(tnxec_json(), out);
        add_to_json(tnxmr_json(), out);
        add_to_json(tnxrp_json(), out);
        add_to_json(trx_json(), out);
        add_to_json(unittest_json(), out);
        add_to_json(vet_json(), out);
        add_to_json(waves_json(), out);
        add_to_json(xdc_json(), out);
        add_to_json(xec_json(), out);
        add_to_json(xem_json(), out);
        add_to_json(xlm_json(), out);
        add_to_json(xmr_json(), out);
        add_to_json(xrp_json(), out);
        add_to_json(xtz_json(), out);
        add_to_json(zec_json(), out);
        add_to_json(zil_json(), out);

        return out;
    }();

    return data;
}
}  // namespace opentxs::blockchain::params
