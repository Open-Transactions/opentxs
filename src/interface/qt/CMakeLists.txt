# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_QT_EXPORT)
  add_subdirectory(displayscale)
  add_subdirectory(seedvalidator)
  add_subdirectory(identitymanager)
endif()

add_subdirectory(qml)

if(OT_QT_EXPORT)
  list(
    APPEND
    OPENTXS_QT_PUBLIC_HEADERS
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/AccountActivity.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/AccountList.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/AccountSummary.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/AccountTree.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/ActivitySummary.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/ActivityThread.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/AmountValidator.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/BlankModel.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/BlockchainAccountStatus.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/BlockchainSelection.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/BlockchainStatistics.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/Contact.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/ContactList.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/DestinationValidator.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/MessagableList.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/Model.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/NymList.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/PayableList.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/Profile.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/SeedTree.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/interface/qt/UnitList.hpp"
  )
  list(
    APPEND
    OPENTXS_QT_PRIVATE_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/DraftValidator.hpp"
  )
  target_sources(
    opentxs-common
    PRIVATE
      "${opentxs_SOURCE_DIR}/src/internal/interface/qt/Factory.hpp"
      "AccountActivity.cpp"
      "AccountList.cpp"
      "AccountSummary.cpp"
      "AccountTree.cpp"
      "ActivitySummary.cpp"
      "ActivityThread.cpp"
      "AmountValidator.cpp"
      "AmountValidator.hpp"
      "BlockchainAccountStatus.cpp"
      "BlockchainDestinationValidator.cpp"
      "BlockchainSelection.cpp"
      "BlockchainStatistics.cpp"
      "Contact.cpp"
      "ContactList.cpp"
      "CustodialDestinationValidator.cpp"
      "DestinationValidator.cpp"
      "DestinationValidator.hpp"
      "DraftValidator.cpp"
      "MessagableList.cpp"
      "Model.cpp"
      "NymList.cpp"
      "PayableList.cpp"
      "Profile.cpp"
      "SeedTree.cpp"
      "SendMonitor.cpp"
      "SendMonitor.hpp"
      "UnitList.cpp"
      ${MOC_SRCS}
  )

  if(OT_BLOCKCHAIN_EXPORT)
    target_sources(opentxs-common PRIVATE "Blockchain.cpp")
  endif()

  target_link_libraries(opentxs-moc PRIVATE Qt::Core Qt::Gui)
  opentxs_link_internal(Qt::Core)
  opentxs_link_internal(Qt::Gui)
  opentxs_link_external(Qt::Core)
  opentxs_link_external(Qt::Gui)
else()
  target_sources(opentxs-common PRIVATE "no-Qt.cpp")
endif()

set(OPENTXS_QT_PUBLIC_HEADERS
    ${OPENTXS_QT_PUBLIC_HEADERS}
    PARENT_SCOPE
)
set(OPENTXS_QT_PRIVATE_HEADERS
    ${OPENTXS_QT_PRIVATE_HEADERS}
    PARENT_SCOPE
)
