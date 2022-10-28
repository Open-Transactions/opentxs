// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/program_options.hpp>
#include <opentxs/opentxs.hpp>
#include <cassert>
#include <iostream>
#include <memory>

namespace ot = opentxs;
namespace po = boost::program_options;

namespace ottest
{
class Options;
}

namespace ottest
{
class OPENTXS_EXPORT ArgumentParser
{
public:
    bool show_help_;

    auto help(const ot::Options& args) const noexcept
        -> const ot::UnallocatedCString&
    {
        static const auto text = [&] {
            auto out = std::stringstream{};
            out << "libopentxs unit test options:\n";
            out << options_;
            out << args.HelpText();

            return out.str();
        }();

        return text;
    }
    auto options() const noexcept -> const po::options_description&
    {
        return options_;
    }

    auto parse(int argc, char** argv) noexcept -> void
    {
        try {
            auto parsed = po::command_line_parser(argc, argv)
                              .options(options_)
                              .allow_unregistered()
                              .run();
            po::store(parsed, variables_);
            po::notify(variables_);
        } catch (po::error& e) {
            std::cerr << "ERROR: " << e.what() << "\n\n"
                      << options_ << std::endl;

            return;
        }

        for (const auto& [name, value] : variables_) {
            if (name == help_) { show_help_ = true; }
        }
    }

    ArgumentParser() noexcept
        : show_help_(false)
        , variables_p_(std::make_unique<po::variables_map>())
        , options_p_(std::make_unique<po::options_description>())
        , variables_(*variables_p_)
        , options_(*options_p_)
    {
        options_.add_options()(help_, "Display this message");
    }

private:
    static constexpr auto help_{"help"};

    std::unique_ptr<po::variables_map> variables_p_;
    std::unique_ptr<po::options_description> options_p_;
    po::variables_map& variables_;
    po::options_description& options_;
};
}  // namespace ottest
