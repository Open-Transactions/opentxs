# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(license)
add_subdirectory(log)
add_subdirectory(platform)
add_subdirectory(storage)

set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/util/Allocated.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/Allocator.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/BlockchainProfile.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/Bytes.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/ConnectionMode.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/Container.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/Iterator.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/Literals.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/Numbers.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/NymEditor.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/Options.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/PasswordCallback.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/PasswordCaller.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/PasswordPrompt.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/Time.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/Types.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/WorkType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/WriteBuffer.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/util/Writer.hpp"
)
target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/util/AsyncConst.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/BoostPMR.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/Bytes.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/Editor.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/Exclusive.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/Flag.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/Future.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/Lockable.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/Mutex.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/P0330.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/PasswordPrompt.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/Pimpl.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/Shared.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/SharedPimpl.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/Signals.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/Size.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/TSV.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/Thread.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/Time.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/Timer.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/Types.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/UniqueQueue.hpp"
    "Actor.hpp"
    "Allocator.cpp"
    "Allocator.hpp"
    "Backoff.hpp"
    "Blank.hpp"
    "Bytes.cpp"
    "Common.cpp"
    "Container.hpp"
    "Exclusive.tpp"
    "Flag.cpp"
    "Flag.hpp"
    "Gatekeeper.cpp"
    "Gatekeeper.hpp"
    "HDIndex.hpp"
    "JobCounter.cpp"
    "JobCounter.hpp"
    "Latest.hpp"
    "NullCallback.cpp"
    "NullCallback.hpp"
    "NymEditor.cpp"
    "Options.cpp"
    "Options.hpp"
    "PasswordCallback.cpp"
    "PasswordCaller.cpp"
    "PasswordPrompt.cpp"
    "PasswordPromptPrivate.cpp"
    "PasswordPromptPrivate.hpp"
    "Polarity.hpp"
    "Random.cpp"
    "Random.hpp"
    "ScopeGuard.cpp"
    "ScopeGuard.hpp"
    "Signals.cpp"
    "Size.cpp"
    "Sodium.cpp"
    "Sodium.hpp"
    "Thread.cpp"
    "Time.cpp"
    "Timer.cpp"
    "Timer.hpp"
    "Work.cpp"
    "Work.hpp"
    "WriteBuffer.cpp"
    "Writer.cpp"
    "WriterPrivate.cpp"
    "WriterPrivate.hpp"
)
opentxs_link_internal(unofficial-sodium::sodium)
opentxs_link_external(Boost::container)
opentxs_link_external(Boost::program_options)
opentxs_link_external(unofficial-sodium::sodium)
target_sources(opentxs-common PRIVATE ${cxx-install-headers})

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/util
    COMPONENT dev
  )
endif()
