# Copyright (c) Monetas AG, 2014

set(cxx-sources
  commands/CmdAcceptAll.cpp
  commands/CmdAcceptInbox.cpp
  commands/CmdAcceptInvoices.cpp
  commands/CmdAcceptMoney.cpp
  commands/CmdAcceptPayments.cpp
  commands/CmdAcceptReceipts.cpp
  commands/CmdAcceptTransfers.cpp
  commands/CmdAddAsset.cpp
  commands/CmdAddServer.cpp
  commands/CmdAddSignature.cpp
  commands/CmdUsageCredits.cpp
  commands/CmdBase.cpp
  commands/CmdBaseAccept.cpp
  commands/CmdBaseInstrument.cpp
  commands/CmdCancel.cpp
  commands/CmdChangePw.cpp
  commands/CmdCheckNym.cpp
  commands/CmdClearExpired.cpp
  commands/CmdClearRecords.cpp
  commands/CmdConfirm.cpp
  commands/CmdDecode.cpp
  commands/CmdDecrypt.cpp
  commands/CmdDeleteInmail.cpp
  commands/CmdDeleteOutmail.cpp
  commands/CmdDeposit.cpp
  commands/CmdDiscard.cpp
  commands/CmdEditAccount.cpp
  commands/CmdEditAsset.cpp
  commands/CmdEditNym.cpp
  commands/CmdEditServer.cpp
  commands/CmdEncode.cpp
  commands/CmdEncrypt.cpp
  commands/CmdExchangeBasket.cpp
  commands/CmdExportCash.cpp
  commands/CmdExportNym.cpp
  commands/CmdGetContract.cpp
  commands/CmdGetMarkets.cpp
  commands/CmdGetMyOffers.cpp
  commands/CmdGetOffers.cpp
  commands/CmdGetReceipt.cpp
  commands/CmdImportCash.cpp
  commands/CmdImportNym.cpp
  commands/CmdInbox.cpp
  commands/CmdInmail.cpp
  commands/CmdInpayments.cpp
  commands/CmdIssueAsset.cpp
  commands/CmdKillOffer.cpp
  commands/CmdKillPlan.cpp
  commands/CmdNewAccount.cpp
  commands/CmdNewAsset.cpp
  commands/CmdNewBasket.cpp
  commands/CmdNewKey.cpp
  commands/CmdNewNym.cpp
  commands/CmdNewOffer.cpp
  commands/CmdNewServer.cpp
  commands/CmdOutbox.cpp
  commands/CmdOutmail.cpp
  commands/CmdOutpayment.cpp
  commands/CmdPasswordDecrypt.cpp
  commands/CmdPasswordEncrypt.cpp
  commands/CmdPayDividend.cpp
  commands/CmdPayInvoice.cpp
  commands/CmdProposePlan.cpp
  commands/CmdRefresh.cpp
  commands/CmdRefreshAccount.cpp
  commands/CmdRefreshNym.cpp
  commands/CmdRegisterNym.cpp
  commands/CmdSendCash.cpp
  commands/CmdSendCheque.cpp
  commands/CmdSendInvoice.cpp
  commands/CmdSendMessage.cpp
  commands/CmdSendVoucher.cpp
  commands/CmdShowAccount.cpp
  commands/CmdShowAccounts.cpp
  commands/CmdShowActive.cpp
  commands/CmdShowAssets.cpp
  commands/CmdShowBalance.cpp
  commands/CmdShowBasket.cpp
  commands/CmdShowCredential.cpp
  commands/CmdShowCredentials.cpp
  commands/CmdShowExpired.cpp
  commands/CmdShowMarkets.cpp
  commands/CmdShowMint.cpp
  commands/CmdShowMyOffers.cpp
  commands/CmdShowNym.cpp
  commands/CmdShowNyms.cpp
  commands/CmdShowOffers.cpp
  commands/CmdShowPayment.cpp
  commands/CmdShowPurse.cpp
  commands/CmdShowRecords.cpp
  commands/CmdShowServers.cpp
  commands/CmdShowWallet.cpp
  commands/CmdSignContract.cpp
  commands/CmdShowWallet.cpp
  commands/CmdTransfer.cpp
  commands/CmdTriggerClause.cpp
  commands/CmdVerifyReceipt.cpp
  commands/CmdVerifySignature.cpp
  commands/CmdWithdrawCash.cpp
  commands/CmdWithdrawVoucher.cpp
  commands/CmdWriteCheque.cpp
  commands/CmdWriteInvoice.cpp
  Helpers.cpp
  TransportCallback.cpp
  OTMessageBuffer.cpp
  OTMessageOutbuffer.cpp
  OpenTransactions.cpp
  ot_made_easy_ot.cpp
  OT_ME.cpp
  ot_otapi_ot.cpp
  ot_utility_ot.cpp
  OTAPI.cpp
  OTAPI_Exec.cpp
  OTClient.cpp
  OTMeCpp.cpp
  OTRecord.cpp
  OTRecordList.cpp
  OTServerConnection.cpp
  OTWallet.cpp
)

file(GLOB_RECURSE cxx-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/client/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

set(MODULE_NAME opentxs-client)
if(WIN32)
  # suppress "decorated name length exceeded, name was truncated" warnings 
  add_definitions("/wd4503")

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/windll.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
    @ONLY
  )

  # we have an extra Windows-specific file to compile
  add_library(${MODULE_NAME}
  ${cxx-sources}
  ${cxx-headers}
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
  )
else()
  add_library(${MODULE_NAME}
    ${cxx-sources}
    ${cxx-headers}
  )
endif()

target_link_libraries(opentxs-client opentxs-ext opentxs-cash opentxs-basket opentxs-core)
set_target_properties(opentxs-client PROPERTIES VERSION "${VERSION_STRING}" SOVERSION "${VERSION_MAJOR}")

if(WIN32)
  add_custom_command(TARGET opentxs-client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/opentxs-client.dll"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/opentxs-client.dll")

  add_custom_command(TARGET opentxs-client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/lib/$<CONFIGURATION>/opentxs-client.pdb"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/opentxs-client.pdb")
endif()

if(NOT WIN32)
  include(GNUInstallDirs)
  install(TARGETS opentxs-client
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          EXPORT opentxsTargets
          COMPONENT main)
endif()

