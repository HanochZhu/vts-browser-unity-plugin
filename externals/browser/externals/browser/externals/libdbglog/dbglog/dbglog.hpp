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
#ifndef shared_dbglog_dbglog_hpp_included_
#define shared_dbglog_dbglog_hpp_included_

#include "logger.hpp"
#include "stream.hpp"
#include "config.hpp"
#include "mask.hpp"

namespace dbglog {
    const unsigned short millis(3);
    const unsigned short micros(6);

    /** Thread safety: thread safe.
     */
    inline module make_module() {
        return module(detail::deflog);
    }

    /** Thread safety: thread safe.
     */
    inline module make_module(const std::string &name)
    {
        return module(name, detail::deflog);
    }

    /** Thread safety: none.
     */
    inline void set_mask(unsigned int mask)
    {
        return detail::deflog.set_mask(mask);
    }

    inline void set_mask(const mask &m)
    {
        return detail::deflog.set_mask(m);
    }

    /** Thread safety: none.
     */
    inline void set_mask(const std::string &m)
    {
        return detail::deflog.set_mask(mask(m));
    }

    /** Thread safety: none.
     */
    inline unsigned int get_mask()
    {
        return detail::deflog.get_mask();
    }

    /** Thread safety: none.
     */
    inline std::string get_mask_string()
    {
        return detail::deflog.get_mask_string();
    }

    /** Thread safety: none.
     */
    inline void log_thread(bool value = true)
    {
        return detail::deflog.log_thread(value);
    }

    /** Thread safety: none.
     */
    inline void log_pid(bool value = true)
    {
        return detail::deflog.log_pid(value);
    }

    /** Thread safety: none.
     */
    inline void log_console(bool value = true)
    {
        return detail::deflog.log_console(value);
    }

    /** Thread safety: none.
     */
    inline bool get_log_console()
    {
        return detail::deflog.get_log_console();
    }

    /** Thread safety: thread safe.
     */
    inline bool log_file(const std::string &filename)
    {
        return detail::deflog.log_file(filename);
    }

    /** Thread safety: thread safe.
     */
    inline bool log_file_truncate()
    {
        return detail::deflog.log_file_truncate();
    }

    /** Thread safety: thread safe.
     */
    inline bool log_file_owner(long uid, long gid)
    {
        return detail::deflog.log_file_owner(uid, gid);
    }

    /** Thread safety: thread safe.
     */
    void thread_id(const std::string &id);

    /** Thread safety: thread safe.
     */
    std::string thread_id();

    /** Thread safety: none.
     */
    inline void log_time_precision(unsigned short precision) {
        detail::deflog.log_time_precision(precision);
    }

    /** Thread safety: none.
     */
    inline unsigned short log_time_precision() {
        return detail::deflog.log_time_precision();
    }

    /** Thread safety: none.
     */
    inline void add_sink(const Sink::pointer &sink) {
        detail::deflog.addSink(sink);
    }

     /** Thread safety: none.
     */
    inline void clear_sinks() {
        detail::deflog.clearSinks();
    }

    /** Thread safety: none.
     */
    inline bool tie(int fd) {
        return detail::deflog.tie(fd);
    }

    /** Thread safety: none.
     */
    inline bool untie(int fd) {
        return detail::deflog.untie(fd);
    }

    /** Thread safety: none.
     */
    inline bool closeOnExec(bool value) {
        return detail::deflog.closeOnExec(value);
    }

    /** Thread safety: none.
     */
    inline void log_line_prefix(const std::string &prefix) {
        detail::deflog.set_prefix(prefix);
    }

    /** Thread safety: none.
    */
    inline const std::string& log_line_prefix() {
        return detail::deflog.get_prefix();
    }

   /** System-independent current process ID getter.
    */
    int process_id();

} // namespace dbglog

/** Main log facility.
 *  Usage:
 *      Log with dbglog::info1 level to default logger
 *          LOG(info1) << "text";
 *
 *      Log with dbglog::debug level to logger L
 *      LOG(info1, L) << "text";
 *
 *  L must be instance of a class with this interface:
 *
 *  class LoggerConcept {
 *      void log(dbglog::level l, const std::string &message
 *               , const dbglog::location &loc);
 *
 *      bool check_level(dbglog::level l);
 *  };
 */
#define LOG(...) \
    DBGLOG_CONCATENATE(DBGLOG_EXPAND_, DBGLOG_NARG(__VA_ARGS__) \
                       (__VA_ARGS__))

/** Same as LOG(...) but level specifier can be anything (i.e. not one of level
 *  enumerations).
 */
#define LOGR(...) \
    DBGLOG_CONCATENATE(DBGLOG_RAW_EXPAND_, DBGLOG_NARG(__VA_ARGS__) \
                       (__VA_ARGS__))

/** One shot log facility.
 *  Same as LOG but logs almost once during program lifetime.
 */
#define LOGONCE(...) \
    DBGLOG_CONCATENATE(DBGLOG_ONCE_EXPAND_, DBGLOG_NARG(__VA_ARGS__) \
                       (__VA_ARGS__))

/** Log'n'throw convenience logger.
 *
 *  Same as LOG but throws exception of given type (2nd or 3rd) initialized with
 *  log line (only logged content, no time, location etc. is appended)
 */
#define LOGTHROW(...) \
    DBGLOG_CONCATENATE(DBGLOG_THROW_EXPAND_, DBGLOG_NARG(__VA_ARGS__) \
                       (__VA_ARGS__))

/** Same as LOGTHROW but level specifier can be anything (i.e. not one of level
 *  enumerations).
 */
#define LOGTHROWR(...) \
    DBGLOG_CONCATENATE(DBGLOG_THROW_RAW_EXPAND_, DBGLOG_NARG(__VA_ARGS__) \
                       (__VA_ARGS__))

#endif // shared_dbglog_dbglog_hpp_included_
