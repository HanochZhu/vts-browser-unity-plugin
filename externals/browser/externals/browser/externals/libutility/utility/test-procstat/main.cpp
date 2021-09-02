/**
 * Copyright (c) 2020 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstdlib>
#include <iostream>

#include <boost/lexical_cast.hpp>

#include "utility/procstat.hpp"

void print(const utility::ProcStat &ps)
{
    std::cout << ps.pid << " [" << ps.ppid << "]: rss=" << ps.rss
              << ", virt=" << ps.virt
              << ", swap=" << ps.swap
              << ", shared=" << ps.shared
              << "\n";
}

int main(int argc, char *argv[])
{
    const auto &shift([&]() { --argc; ++argv; });

    if (argc < 2) {
        for (const auto &ps : utility::getProcStat({})) {
            print(ps);
        }
        return EXIT_SUCCESS;
    }

    shift();
    if (!strcmp(argv[0], "self")) {
        print(utility::getProcStat());
        return EXIT_SUCCESS;
    }

    if (!strcmp(argv[0], "uid")) {
        shift();

        if (!argc) {
            for (const auto &ps : utility::getUserProcStat(::getuid())) {
                print(ps);
            }
            return EXIT_SUCCESS;
        }

        utility::UidList uids;
        uids.reserve(argc);
        for (int i(0); i < argc; ++i) {
            uids.push_back(boost::lexical_cast<long>(argv[i]));
        }

        for (const auto &ps : utility::getUserProcStat(uids)) {
            print(ps);
        }

        return EXIT_SUCCESS;
    }

    utility::PidList pids;
    pids.reserve(argc);
    for (int i(0); i < argc; ++i) {
        pids.push_back(boost::lexical_cast<long>(argv[i]));
    }

    for (const auto &ps : utility::getProcStat(pids)) {
        print(ps);
    }

    return EXIT_SUCCESS;
}