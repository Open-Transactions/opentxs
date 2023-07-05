# Copyright (c) 2010-2022 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(alloc)
add_subdirectory(license)
add_subdirectory(log)
add_subdirectory(platform)
add_subdirectory(storage)

libopentxs_add_public_header("util" "Allocated.hpp")
libopentxs_add_public_header("util" "Allocator.hpp")
libopentxs_add_public_header("util" "BlockchainProfile.hpp")
libopentxs_add_public_header("util" "Bytes.hpp")
libopentxs_add_public_header("util" "ConnectionMode.hpp")
libopentxs_add_public_header("util" "Container.hpp")
libopentxs_add_public_header("util" "Iterator.hpp")
libopentxs_add_public_header("util" "Literals.hpp")
libopentxs_add_public_header("util" "Multiple.hpp")
libopentxs_add_public_header("util" "Numbers.hpp")
libopentxs_add_public_header("util" "NymEditor.hpp")
libopentxs_add_public_header("util" "Options.hpp")
libopentxs_add_public_header("util" "PasswordCallback.hpp")
libopentxs_add_public_header("util" "PasswordCaller.hpp")
libopentxs_add_public_header("util" "PasswordPrompt.hpp")
libopentxs_add_public_header("util" "Time.hpp")
libopentxs_add_public_header("util" "Types.hpp")
libopentxs_add_public_header("util" "WorkType.hpp")
libopentxs_add_public_header("util" "WriteBuffer.hpp")
libopentxs_add_public_header("util" "Writer.hpp")

target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_SOURCE_DIR}/src/internal/util/Bytes.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/util/DeferredConstruction.hpp"
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
    "Shutdown.cpp"
    "Shutdown.hpp"
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
libopentxs_link_internal(unofficial-sodium::sodium)
libopentxs_link_external(Boost::container)
libopentxs_link_external(Boost::program_options)
libopentxs_link_external(unofficial-sodium::sodium)
