/**
 * Copyright (c) 2017 Melown Technologies SE
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
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <stdexcept>

#include "dbglog/dbglog.hpp"
#include "dbglog/mask.hpp"

void once() {
    LOGONCE(info4) << "once";
    LOGONCE(info4, dbglog::detail::deflog) << "once: another line";
}

BOOST_AUTO_TEST_CASE(dbglog_once)
{
    // TODO: test once
    once();
    once();
    once();
}

BOOST_AUTO_TEST_CASE(dbglog_mask)
{
    // TODO: test different masks
    std::string def("ALL");
    try {
        dbglog::mask mask(def);
        std::cout << def <<  " -> " << mask
                  << "(" << mask.get() << ")" << std::endl;
        dbglog::set_mask(mask);
    } catch (const std::runtime_error &e) {
        std::cout << def <<  " -> error: " << e.what() << std::endl;
    }
}
