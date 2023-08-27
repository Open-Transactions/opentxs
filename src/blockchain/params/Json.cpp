// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/params/Json.hpp"  // IWYU pragma: associated

namespace opentxs::blockchain::params
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#include <ADA.json.h>
#include <ALGO.json.h>
#include <AR.json.h>
#include <ATOM.json.h>
#include <AVAX.json.h>
#include <BCH.json.h>
#include <BNB.json.h>
#include <BSC.json.h>
#include <BSV.json.h>
#include <BTC.json.h>
#include <BTG.json.h>
#include <BTS.json.h>
#include <CELO.json.h>
#include <CHZ.json.h>
#include <CSPR.json.h>
#include <DASH.json.h>
#include <DCR.json.h>
#include <DOGE.json.h>
#include <DOT.json.h>
#include <EGLD.json.h>
#include <EOS.json.h>
#include <ETC.json.h>
#include <ETH.json.h>
#include <ETHgoerli.json.h>
#include <ETHholesovice.json.h>
#include <ETHkovan.json.h>
#include <ETHmorden.json.h>
#include <ETHolympic.json.h>
#include <ETHrinkeby.json.h>
#include <ETHropsten.json.h>
#include <ETHsepolia.json.h>
#include <EXP.json.h>
#include <FCT.json.h>
#include <FIL.json.h>
#include <FLOW.json.h>
#include <FTM.json.h>
#include <HBAR.json.h>
#include <HNT.json.h>
#include <HT.json.h>
#include <ICP.json.h>
#include <ICX.json.h>
#include <KDA.json.h>
#include <KLAY.json.h>
#include <KSM.json.h>
#include <LTC.json.h>
#include <LUNA.json.h>
#include <MAID.json.h>
#include <MATIC.json.h>
#include <MINA.json.h>
#include <MIOTA.json.h>
#include <NEAR.json.h>
#include <NEO.json.h>
#include <NXT.json.h>
#include <OKB.json.h>
#include <ONE.json.h>
#include <PKT.json.h>
#include <POA.json.h>
#include <QNT.json.h>
#include <QTUM.json.h>
#include <RUNE.json.h>
#include <RVN.json.h>
#include <SC.json.h>
#include <SCRT.json.h>
#include <SOL.json.h>
#include <STEEM.json.h>
#include <STX.json.h>
#include <TFUEL.json.h>
#include <THETA.json.h>
#include <TRX.json.h>
#include <UNITTEST.json.h>
#include <VET.json.h>
#include <WAVES.json.h>
#include <XDC.json.h>
#include <XEC.json.h>
#include <XEM.json.h>
#include <XLM.json.h>
#include <XMR.json.h>
#include <XRP.json.h>
#include <XTZ.json.h>
#include <ZEC.json.h>
#include <ZIL.json.h>
#include <tn4BCH.json.h>
#include <tnBCH.json.h>
#include <tnBSV.json.h>
#include <tnBTC.json.h>
#include <tnCSPR.json.h>
#include <tnDASH.json.h>
#include <tnDOGE.json.h>
#include <tnLTC.json.h>
#include <tnMAID.json.h>
#include <tnNEM.json.h>
#include <tnNXT.json.h>
#include <tnPKT.json.h>
#include <tnPOA.json.h>
#include <tnSC.json.h>
#include <tnSTEEM.json.h>
#include <tnWAVES.json.h>
#include <tnXEC.json.h>
#include <tnXMR.json.h>
#include <tnXRP.json.h>

#pragma GCC diagnostic pop
}  // namespace opentxs::blockchain::params

namespace opentxs::blockchain::params
{
auto ada_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__ADA_json), __ADA_json_len};
}
auto algo_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__ALGO_json), __ALGO_json_len};
}
auto ar_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__AR_json), __AR_json_len};
}
auto atom_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__ATOM_json), __ATOM_json_len};
}
auto avax_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__AVAX_json), __AVAX_json_len};
}
auto bch_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__BCH_json), __BCH_json_len};
}
auto bnb_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__BNB_json), __BNB_json_len};
}
auto bsc_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__BSC_json), __BSC_json_len};
}
auto bsv_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__BSV_json), __BSV_json_len};
}
auto btc_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__BTC_json), __BTC_json_len};
}
auto btg_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__BTG_json), __BTG_json_len};
}
auto bts_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__BTS_json), __BTS_json_len};
}
auto celo_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__CELO_json), __CELO_json_len};
}
auto chz_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__CHZ_json), __CHZ_json_len};
}
auto cspr_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__CSPR_json), __CSPR_json_len};
}
auto dash_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__DASH_json), __DASH_json_len};
}
auto dcr_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__DCR_json), __DCR_json_len};
}
auto doge_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__DOGE_json), __DOGE_json_len};
}
auto dot_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__DOT_json), __DOT_json_len};
}
auto egld_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__EGLD_json), __EGLD_json_len};
}
auto eos_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__EOS_json), __EOS_json_len};
}
auto etc_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__ETC_json), __ETC_json_len};
}
auto ethgoerli_json() noexcept -> std::string_view
{
    return {
        reinterpret_cast<const char*>(__ETHgoerli_json), __ETHgoerli_json_len};
}
auto ethholesovice_json() noexcept -> std::string_view
{
    return {
        reinterpret_cast<const char*>(__ETHholesovice_json),
        __ETHholesovice_json_len};
}
auto eth_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__ETH_json), __ETH_json_len};
}
auto ethkovan_json() noexcept -> std::string_view
{
    return {
        reinterpret_cast<const char*>(__ETHkovan_json), __ETHkovan_json_len};
}
auto ethmorden_json() noexcept -> std::string_view
{
    return {
        reinterpret_cast<const char*>(__ETHmorden_json), __ETHmorden_json_len};
}
auto etholympic_json() noexcept -> std::string_view
{
    return {
        reinterpret_cast<const char*>(__ETHolympic_json),
        __ETHolympic_json_len};
}
auto ethrinkeby_json() noexcept -> std::string_view
{
    return {
        reinterpret_cast<const char*>(__ETHrinkeby_json),
        __ETHrinkeby_json_len};
}
auto ethropsten_json() noexcept -> std::string_view
{
    return {
        reinterpret_cast<const char*>(__ETHropsten_json),
        __ETHropsten_json_len};
}
auto ethsepolia_json() noexcept -> std::string_view
{
    return {
        reinterpret_cast<const char*>(__ETHsepolia_json),
        __ETHsepolia_json_len};
}
auto exp_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__EXP_json), __EXP_json_len};
}
auto fct_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__FCT_json), __FCT_json_len};
}
auto fil_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__FIL_json), __FIL_json_len};
}
auto flow_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__FLOW_json), __FLOW_json_len};
}
auto ftm_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__FTM_json), __FTM_json_len};
}
auto hbar_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__HBAR_json), __HBAR_json_len};
}
auto hnt_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__HNT_json), __HNT_json_len};
}
auto ht_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__HT_json), __HT_json_len};
}
auto icp_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__ICP_json), __ICP_json_len};
}
auto icx_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__ICX_json), __ICX_json_len};
}
auto kda_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__KDA_json), __KDA_json_len};
}
auto klay_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__KLAY_json), __KLAY_json_len};
}
auto ksm_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__KSM_json), __KSM_json_len};
}
auto ltc_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__LTC_json), __LTC_json_len};
}
auto luna_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__LUNA_json), __LUNA_json_len};
}
auto maid_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__MAID_json), __MAID_json_len};
}
auto matic_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__MATIC_json), __MATIC_json_len};
}
auto mina_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__MINA_json), __MINA_json_len};
}
auto miota_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__MIOTA_json), __MIOTA_json_len};
}
auto near_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__NEAR_json), __NEAR_json_len};
}
auto neo_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__NEO_json), __NEO_json_len};
}
auto nxt_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__NXT_json), __NXT_json_len};
}
auto okb_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__OKB_json), __OKB_json_len};
}
auto one_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__ONE_json), __ONE_json_len};
}
auto pkt_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__PKT_json), __PKT_json_len};
}
auto poa_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__POA_json), __POA_json_len};
}
auto qnt_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__QNT_json), __QNT_json_len};
}
auto qtum_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__QTUM_json), __QTUM_json_len};
}
auto rune_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__RUNE_json), __RUNE_json_len};
}
auto rvn_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__RVN_json), __RVN_json_len};
}
auto sc_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__SC_json), __SC_json_len};
}
auto scrt_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__SCRT_json), __SCRT_json_len};
}
auto sol_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__SOL_json), __SOL_json_len};
}
auto steem_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__STEEM_json), __STEEM_json_len};
}
auto stx_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__STX_json), __STX_json_len};
}
auto tfuel_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__TFUEL_json), __TFUEL_json_len};
}
auto theta_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__THETA_json), __THETA_json_len};
}
auto tn4bch_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tn4BCH_json), __tn4BCH_json_len};
}
auto tnbch_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnBCH_json), __tnBCH_json_len};
}
auto tnbsv_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnBSV_json), __tnBSV_json_len};
}
auto tnbtc_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnBTC_json), __tnBTC_json_len};
}
auto tncspr_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnCSPR_json), __tnCSPR_json_len};
}
auto tndash_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnDASH_json), __tnDASH_json_len};
}
auto tndoge_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnDOGE_json), __tnDOGE_json_len};
}
auto tnltc_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnLTC_json), __tnLTC_json_len};
}
auto tnmaid_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnMAID_json), __tnMAID_json_len};
}
auto tnnem_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnNEM_json), __tnNEM_json_len};
}
auto tnnxt_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnNXT_json), __tnNXT_json_len};
}
auto tnpkt_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnPKT_json), __tnPKT_json_len};
}
auto tnpoa_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnPOA_json), __tnPOA_json_len};
}
auto tnsc_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnSC_json), __tnSC_json_len};
}
auto tnsteem_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnSTEEM_json), __tnSTEEM_json_len};
}
auto tnwaves_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnWAVES_json), __tnWAVES_json_len};
}
auto tnxec_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnXEC_json), __tnXEC_json_len};
}
auto tnxmr_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnXMR_json), __tnXMR_json_len};
}
auto tnxrp_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnXRP_json), __tnXRP_json_len};
}
auto trx_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__TRX_json), __TRX_json_len};
}
auto unittest_json() noexcept -> std::string_view
{
    return {
        reinterpret_cast<const char*>(__UNITTEST_json), __UNITTEST_json_len};
}
auto vet_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__VET_json), __VET_json_len};
}
auto waves_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__WAVES_json), __WAVES_json_len};
}
auto xdc_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__XDC_json), __XDC_json_len};
}
auto xec_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__XEC_json), __XEC_json_len};
}
auto xem_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__XEM_json), __XEM_json_len};
}
auto xlm_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__XLM_json), __XLM_json_len};
}
auto xmr_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__XMR_json), __XMR_json_len};
}
auto xrp_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__XRP_json), __XRP_json_len};
}
auto xtz_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__XTZ_json), __XTZ_json_len};
}
auto zec_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__ZEC_json), __ZEC_json_len};
}
auto zil_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__ZIL_json), __ZIL_json_len};
}
}  // namespace opentxs::blockchain::params
