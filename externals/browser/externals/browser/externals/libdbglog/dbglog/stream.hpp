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
#ifndef shared_dbglog_stream_hpp_included_
#define shared_dbglog_stream_hpp_included_

#include <utility>
#include <cstddef>

#include <boost/noncopyable.hpp>
#include <boost/format.hpp>

#include <sstream>
#include <stdexcept>

#include "dbglog/logger.hpp"
#include "dbglog/level.hpp"
#include "dbglog/detail/log_helpers.hpp"
#include "dbglog/detail/logger.hpp"

namespace dbglog {

namespace detail {

inline void formatMessage(boost::format&) {}

template <typename T, typename ...Args>
inline void formatMessage(boost::format &fmt, T &&arg, Args &&...rest)
{
    fmt % arg;
    return formatMessage(fmt, std::forward<Args>(rest)...);
}

} // namespace detail

template <typename SinkType>
class stream : public boost::noncopyable
{
public:
    stream(const location &loc, level l, SinkType &sink)
        : loc_(loc), l_(l), sink_(sink)
    {}

    ~stream() {
        sink_.log(l_, os_.str(), loc_);
    }

    template <typename T>
    std::basic_ostream<char>& operator<<(const T &t)
    {
        return os_ << t;
    }

    template <typename ...Args>
    stream& operator()(const std::string &format, Args &&...args)
    {
        boost::format fmt(format);
        fmt.exceptions(boost::io::no_error_bits);
        detail::formatMessage(fmt, std::forward<Args>(args)...);
        os_ << fmt;
        return *this;
    }

    template <typename ...Args>
    stream& operator()(boost::format &&fmt, Args &&...args)
    {
        fmt.exceptions(boost::io::no_error_bits);
        detail::formatMessage(fmt, std::forward<Args>(args)...);
        os_ << fmt;
        return *this;
    }

private:
    std::ostringstream os_;
    const location loc_;
    const level l_;
    SinkType &sink_;
};

template <typename SinkType, typename ExcType>
class exc_stream : public boost::noncopyable
{
public:
    exc_stream(const location &loc, level l, SinkType &sink)
        : loc_(loc), l_(l), sink_(sink)
    {}

    ~exc_stream() noexcept(false) {
        sink_.log(l_, os_.str(), loc_);
        os_ << " @" << loc_;
        if (!std::uncaught_exception()) {
            throw ExcType(os_.str());
        }
    }

    template <typename T>
    std::basic_ostream<char>& operator<<(const T &t)
    {
        return os_ << t;
    }

private:
    std::ostringstream os_;
    const location loc_;
    const level l_;
    SinkType &sink_;
};

} // namespace dbglog


// don't ask...
#ifdef _WIN32
#  define DBGLOG_PAREN_LEFT_ (
#  define DBGLOG_PAREN_RIGHT_ )
#  define DBGLOG_NARG(...) DBGLOG_ARG_N DBGLOG_PAREN_LEFT_ ,##__VA_ARGS__,8,7,6,5,4,3,2,1,0 DBGLOG_PAREN_RIGHT_
#  define DBGLOG_ARG_N(_0,_1,_2,_3,_4,_5,_6,_7,_8,N,...) N
#else
#  define DBGLOG_NARG(...) DBGLOG_NARG_(__VA_ARGS__, DBGLOG_RSEQ_N())
#  define DBGLOG_NARG_(...) DBGLOG_ARG_N(__VA_ARGS__)
#  define DBGLOG_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#  define DBGLOG_RSEQ_N() 8, 7, 6, 5, 4, 3, 2, 1, 0
#endif

#define DBGLOG_CONCATENATE(arg1, arg2) DBGLOG_CONCATENATE1(arg1, arg2)
#define DBGLOG_CONCATENATE1(arg1, arg2) DBGLOG_CONCATENATE2(arg1, arg2)
#define DBGLOG_CONCATENATE2(arg1, arg2) arg1##arg2

#define DBGLOG_EXPAND_1(LEVEL) \
    if (!dbglog::detail::check_level(dbglog::LEVEL, dbglog::detail::deflog)); \
    else dbglog::stream<dbglog::logger> \
    (DBGLOG_PLACE, dbglog::LEVEL, dbglog::detail::deflog)

#define DBGLOG_EXPAND_2(LEVEL, SINK) \
    if (!dbglog::detail::check_level(dbglog::LEVEL, SINK)); \
    else dbglog::stream<decltype(SINK)>(DBGLOG_PLACE, dbglog::LEVEL, SINK)

#define DBGLOG_EXPAND_3(a, b, c) LOG_TAKES_EITHER_1_OR_2_ARGUMENTS_NOT_3
#define DBGLOG_EXPAND_4(a, b, c, d) LOG_TAKES_EITHER_1_OR_2_ARGUMENTS_NOT_4
#define DBGLOG_EXPAND_5(a, b, c, d, e) LOG_TAKES_EITHER_1_OR_2_ARGUMENTS_NOT_5
#define DBGLOG_EXPAND_6(a, b, c, d, e, f) LOG_TAKES_EITHER_1_OR_2_ARGUMENTS_NOT_6
#define DBGLOG_EXPAND_7(a, b, c, d, e, f, g) LOG_TAKES_EITHER_1_OR_2_ARGUMENTS_NOT_7
#define DBGLOG_EXPAND_8(a, b, c, d, e, f, g, h) LOG_TAKES_EITHER_1_OR_2_ARGUMENTS_NOT_8

#define DBGLOG_RAW_EXPAND_1(LEVEL) \
    if (!dbglog::detail::check_level(LEVEL, dbglog::detail::deflog)); \
    else dbglog::stream<dbglog::logger> \
    (DBGLOG_PLACE, LEVEL, dbglog::detail::deflog)

#define DBGLOG_RAW_EXPAND_2(LEVEL, SINK) \
    if (!dbglog::detail::check_level(LEVEL, SINK)); \
    else dbglog::stream<decltype(SINK)>(DBGLOG_PLACE, LEVEL, SINK)

#define DBGLOG_RAW_EXPAND_3(a, b, c) LOG_TAKES_EITHER_1_OR_2_ARGUMENTS_NOT_3
#define DBGLOG_RAW_EXPAND_4(a, b, c, d) LOG_TAKES_EITHER_1_OR_2_ARGUMENTS_NOT_4
#define DBGLOG_RAW_EXPAND_5(a, b, c, d, e) LOG_TAKES_EITHER_1_OR_2_ARGUMENTS_NOT_5
#define DBGLOG_RAW_EXPAND_6(a, b, c, d, e, f) LOG_TAKES_EITHER_1_OR_2_ARGUMENTS_NOT_6
#define DBGLOG_RAW_EXPAND_7(a, b, c, d, e, f, g) LOG_TAKES_EITHER_1_OR_2_ARGUMENTS_NOT_7
#define DBGLOG_RAW_EXPAND_8(a, b, c, d, e, f, g, h) LOG_TAKES_EITHER_1_OR_2_ARGUMENTS_NOT_8

#define DBGLOG_ADD_LINE_NO(x) DBGLOG_ADD_LINE_NO1(x,__LINE__)
#define DBGLOG_ADD_LINE_NO1(x, y) DBGLOG_ADD_LINE_NO2(x, y)
#define DBGLOG_ADD_LINE_NO2(x, y) x##y

#define DBGLOG_ONCE_EXPAND_1(LEVEL)                                     \
    static std::atomic<bool> DBGLOG_ADD_LINE_NO(dbglog_once_guard_)(false); \
    if (!dbglog::detail::check_level                                    \
        (dbglog::LEVEL, dbglog::detail::deflog                          \
         , DBGLOG_ADD_LINE_NO(dbglog_once_guard_)));                    \
    else dbglog::stream<dbglog::logger>                                 \
             (DBGLOG_PLACE, dbglog::LEVEL, dbglog::detail::deflog)

#define DBGLOG_ONCE_EXPAND_2(LEVEL, SINK) \
    static std::atomic<bool> DBGLOG_ADD_LINE_NO(dbglog_once_guard_)(false); \
    if (!dbglog::detail::check_level                                    \
        (dbglog::LEVEL, dbglog::detail::deflog                          \
         , DBGLOG_ADD_LINE_NO(dbglog_once_guard_)));                    \
    else dbglog::stream<decltype(SINK)>(DBGLOG_PLACE, dbglog::LEVEL, SINK)

#define DBGLOG_ONCE_EXPAND_3(a, b, c) LOGONCE_TAKES_EITHER_1_OR_2_ARGUMENTS_NOT_3
#define DBGLOG_ONCE_EXPAND_4(a, b, c, d) LOGONCE_TAKES_EITHER_1_OR_2_ARGUMENTS_NOT_4
#define DBGLOG_ONCE_EXPAND_5(a, b, c, d, e) LOGONCE_TAKES_EITHER_1_OR_2_ARGUMENTS_NOT_5
#define DBGLOG_ONCE_EXPAND_6(a, b, c, d, e, f) LOGONCE_TAKES_EITHER_1_OR_2_ARGUMENTS_NOT_6
#define DBGLOG_ONCE_EXPAND_7(a, b, c, d, e, f, g) LOGONCE_TAKES_EITHER_1_OR_2_ARGUMENTS_NOT_7
#define DBGLOG_ONCE_EXPAND_8(a, b, c, d, e, f, g, h) LOGONCE_TAKES_EITHER_1_OR_2_ARGUMENTS_NOT_8

#define DBGLOG_THROW_EXPAND_1 THROW_TAKES_EITHER_2_OR_3_ARGUMENTS_NOT_1

#define DBGLOG_THROW_EXPAND_2(LEVEL, EXCTYPE)                           \
    dbglog::exc_stream<dbglog::logger, EXCTYPE>                         \
    (DBGLOG_PLACE, dbglog::LEVEL, dbglog::detail::deflog)

#define DBGLOG_THROW_EXPAND_3(LEVEL, SINK, EXCTYPE)                     \
    dbglog::exc_stream<decltype(SINK), EXCTYPE>                    \
    (DBGLOG_PLACE, dbglog::LEVEL, SINK)

#define DBGLOG_THROW_EXPAND_4(a, b, c, d) THROW_TAKES_EITHER_2_OR_3_ARGUMENTS_NOT_4
#define DBGLOG_THROW_EXPAND_5(a, b, c, d, e) THROW_TAKES_EITHER_2_OR_3_ARGUMENTS_NOT_5
#define DBGLOG_THROW_EXPAND_6(a, b, c, d, e, f) THROW_TAKES_EITHER_2_OR_3_ARGUMENTS_NOT_6
#define DBGLOG_THROW_EXPAND_7(a, b, c, d, e, f, g) THROW_TAKES_EITHER_2_OR_3_ARGUMENTS_NOT_7
#define DBGLOG_THROW_EXPAND_8(a, b, c, d, e, f, g, h) THROW_TAKES_EITHER_2_OR_3_ARGUMENTS_NOT_8

#define DBGLOG_THROW_RAW_EXPAND_1 THROW_TAKES_EITHER_2_OR_3_ARGUMENTS_NOT_1

#define DBGLOG_THROW_RAW_EXPAND_2(LEVEL, EXCTYPE)                           \
    dbglog::exc_stream<dbglog::logger, EXCTYPE>                         \
    (DBGLOG_PLACE, LEVEL, dbglog::detail::deflog)

#define DBGLOG_THROW_RAW_EXPAND_3(LEVEL, SINK, EXCTYPE)                     \
    dbglog::exc_stream<decltype(SINK), EXCTYPE>                    \
    (DBGLOG_PLACE, LEVEL, SINK)

#define DBGLOG_THROW_RAW_EXPAND_4(a, b, c, d) THROW_TAKES_EITHER_2_OR_3_ARGUMENTS_NOT_4
#define DBGLOG_THROW_RAW_EXPAND_5(a, b, c, d, e) THROW_TAKES_EITHER_2_OR_3_ARGUMENTS_NOT_5
#define DBGLOG_THROW_RAW_EXPAND_6(a, b, c, d, e, f) THROW_TAKES_EITHER_2_OR_3_ARGUMENTS_NOT_6
#define DBGLOG_THROW_RAW_EXPAND_7(a, b, c, d, e, f, g) THROW_TAKES_EITHER_2_OR_3_ARGUMENTS_NOT_7
#define DBGLOG_THROW_RAW_EXPAND_8(a, b, c, d, e, f, g, h) THROW_TAKES_EITHER_2_OR_3_ARGUMENTS_NOT_8

#define DBGLOG_PLACE dbglog::location                       \
    (__FILE__, (const char*)__FUNCTION__, __LINE__, true)

#endif // shared_dbglog_stream_hpp_included_

