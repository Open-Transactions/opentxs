# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(PROTO_SOURCES
    APIArgument.proto
    AcceptPendingPayment.proto
    AccountData.proto
    AccountEvent.proto
    AddClaim.proto
    AddContact.proto
    AsymmetricKey.proto
    Authority.proto
    Bailment.proto
    BailmentReply.proto
    BasketItem.proto
    BasketParams.proto
    Bip47Channel.proto
    Bip47Direction.proto
    BitcoinBlockHeaderFields.proto
    BlindedSeriesList.proto
    BlockchainAccountData.proto
    BlockchainActivity.proto
    BlockchainAddress.proto
    BlockchainBlockHeader.proto
    BlockchainBlockLocalData.proto
    BlockchainDeterministicAccountData.proto
    BlockchainEnums.proto
    BlockchainExternalAddress.proto
    BlockchainFilterHeader.proto
    BlockchainHDAccountData.proto
    BlockchainInputWitness.proto
    BlockchainOutputMultisigDetails.proto
    BlockchainPeerAddress.proto
    BlockchainPreviousOutput.proto
    BlockchainTransaction.proto
    BlockchainTransactionInput.proto
    BlockchainTransactionOutput.proto
    BlockchainTransactionProposal.proto
    BlockchainTransactionProposedNotification.proto
    BlockchainTransactionProposedOutput.proto
    BlockchainWalletKey.proto
    CashEnums.proto
    ChildCredentialParameters.proto
    Ciphertext.proto
    Claim.proto
    ClientContext.proto
    ConnectionInfo.proto
    ConnectionInfoReply.proto
    ConsensusEnums.proto
    Contact.proto
    ContactData.proto
    ContactEnums.proto
    ContactEvent.proto
    ContactItem.proto
    ContactSection.proto
    Context.proto
    ContractEnums.proto
    CreateInstrumentDefinition.proto
    CreateNym.proto
    Credential.proto
    CurrencyParams.proto
    DisplayScale.proto
    Enums.proto
    Envelope.proto
    EquityParams.proto
    EthereumBlockHeaderFields.proto
    Faucet.proto
    GCS.proto
    GetWorkflow.proto
    HDAccount.proto
    HDPath.proto
    HDSeed.proto
    Identifier.proto
    InstrumentRevision.proto
    Issuer.proto
    KeyCredential.proto
    ListenAddress.proto
    LucreTokenData.proto
    MasterCredentialParameters.proto
    ModifyAccount.proto
    MoveFunds.proto
    NoticeAcknowledgement.proto
    Nym.proto
    NymIDSource.proto
    OTXEnums.proto
    OTXPush.proto
    OutBailment.proto
    OutBailmentReply.proto
    P2PBlockchainChainState.proto
    P2PBlockchainHello.proto
    P2PBlockchainSync.proto
    PairEvent.proto
    PaymentCode.proto
    PaymentEvent.proto
    PaymentWorkflow.proto
    PaymentWorkflowEnums.proto
    PeerEnums.proto
    PeerObject.proto
    PeerReply.proto
    PeerRequest.proto
    PeerRequestHistory.proto
    PeerRequestWorkflow.proto
    PendingBailment.proto
    PendingCommand.proto
    Purse.proto
    PurseExchange.proto
    RPCCommand.proto
    RPCEnums.proto
    RPCPush.proto
    RPCResponse.proto
    RPCStatus.proto
    RPCTask.proto
    ScaleRatio.proto
    Seed.proto
    SendMessage.proto
    SendPayment.proto
    ServerContext.proto
    ServerContract.proto
    ServerReply.proto
    ServerRequest.proto
    SessionData.proto
    Signature.proto
    SourceProof.proto
    SpentTokenList.proto
    StorageAccountIndex.proto
    StorageAccounts.proto
    StorageBip47AddressIndex.proto
    StorageBip47ChannelList.proto
    StorageBip47Contexts.proto
    StorageBip47NymAddressIndex.proto
    StorageBlockchainAccountList.proto
    StorageBlockchainTransactions.proto
    StorageContactAddressIndex.proto
    StorageContactNymIndex.proto
    StorageContacts.proto
    StorageCredentials.proto
    StorageEnums.proto
    StorageIDList.proto
    StorageIssuers.proto
    StorageItemHash.proto
    StorageItems.proto
    StorageNotary.proto
    StorageNym.proto
    StorageNymList.proto
    StoragePaymentWorkflows.proto
    StoragePurse.proto
    StorageRoot.proto
    StorageSeeds.proto
    StorageServers.proto
    StorageThread.proto
    StorageThreadItem.proto
    StorageUnits.proto
    StorageWorkflowIndex.proto
    StorageWorkflowType.proto
    StoreSecret.proto
    SymmetricKey.proto
    TaggedKey.proto
    TaskComplete.proto
    Token.proto
    TransactionData.proto
    UnitAccountMap.proto
    UnitDefinition.proto
    Verification.proto
    VerificationGroup.proto
    VerificationIdentity.proto
    VerificationOffer.proto
    VerificationSet.proto
    VerifyClaim.proto
    ZMQEnums.proto
)

protobuf_generate_cpp(
  PROTO_SRC
  PROTO_HEADER
  ${PROTO_SOURCES}
)

add_library(messages OBJECT ${PROTO_SRC} ${PROTO_HEADER})
target_link_libraries(messages PUBLIC protobuf::libprotobuf-lite)
target_include_directories(messages SYSTEM PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")

if(MSVC)
  target_compile_options(messages PRIVATE "/wd4244" "/wd4267")
else()
  target_compile_options(
    messages
    PRIVATE
      -Wno-c++20-compat
      -Wno-effc++
      -Wno-extra-semi
      -Wno-missing-declarations
      -Wno-reserved-identifier
      -Wno-shorten-64-to-32
      -Wno-suggest-destructor-override
      -Wno-switch-default
      -Wno-switch-enum
      -Wno-undef
      -Wno-unsafe-buffer-usage
      -Wno-unused-macros
  )
endif()

if(WIN32)
  set_target_properties(messages PROPERTIES UNITY_BUILD OFF)
endif()

set_target_properties(
  messages
  PROPERTIES
    MSVC_RUNTIME_LIBRARY "${MSVC_RUNTIME_DEFAULT}"
    C_INCLUDE_WHAT_YOU_USE ""
    CXX_INCLUDE_WHAT_YOU_USE ""
    C_CLANG_TIDY ""
    CXX_CLANG_TIDY ""
)

opentxs_link_internal(messages)
opentxs_link_external(protobuf::libprotobuf-lite)
opentxs_link_external($<BUILD_INTERFACE:messages>)
opentxs_add_sources($<TARGET_OBJECTS:messages>)
add_dependencies(generated_code messages)
