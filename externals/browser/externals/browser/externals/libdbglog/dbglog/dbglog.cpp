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

#include "detail/log_helpers.hpp"
#include "detail/logger.hpp"

namespace dbglog {

const std::string logger::empty_;

namespace detail {

#if defined(__GNUC__)
#    define INIT_PRIORITY(PRIORITY) \
    __attribute__ ((init_priority (PRIORITY)))
#else
#    define INIT_PRIORITY(PRIORITY)
#endif

#ifdef __EMSCRIPTEN__
thread_local std::unique_ptr<std::string> thread_id::holder_ INIT_PRIORITY(101);
std::uint32_t thread_id::generator_;
#else
boost::thread_specific_ptr<std::string> thread_id::holder_ INIT_PRIORITY(101);
std::atomic_uint_fast64_t thread_id::generator_ INIT_PRIORITY(101)(0);
#endif

logger deflog INIT_PRIORITY(101)(default_);

} // namespace detail

void thread_id(const std::string &id)
{
    detail::thread_id::set(id);
}

std::string thread_id()
{
    return detail::thread_id::get();
}

int process_id()
{
    return detail::processId();
}

} // namespace dbglog
