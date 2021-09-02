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
#ifndef dbglog_logfile_hpp_included_
#define dbglog_logfile_hpp_included_

#include <set>
#include <string>
#include <iostream>

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

#ifndef _WIN32
#include <sys/types.h>
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>

#ifdef _WIN32
typedef int mode_t;
typedef int uid_t;
typedef int gid_t;
#define S_IRUSR _S_IREAD
#define S_IWUSR _S_IWRITE
#endif // _WIN32

// implement TEMP_FAILURE_RETRY if not present on platform (via C++11 lambda)
#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(operation) [&]()->int {       \
        for (;;) { int e(operation);                     \
            if ((-1 == e) && (EINTR == errno)) continue; \
            return e;                                    \
        }                                                \
    }()
#endif

namespace dbglog {

namespace detail {
    const ::mode_t DefaultMode(S_IRUSR | S_IWUSR);
}

class logger_file : boost::noncopyable {
public:
    logger_file();

    ~logger_file();

    bool log_file(const std::string &filename
                  , ::mode_t mode = detail::DefaultMode);

    bool log_file_truncate();

    bool tie(int fd, bool remember=true);

    bool untie(int fd, const std::string &path = "/dev/null"
               , ::mode_t mode = detail::DefaultMode);

    bool log_file_owner(uid_t owner, gid_t group);

    bool closeOnExec(bool value);

protected:
    bool write_file(const std::string &line) {
        return write_file(line.data(), line.size());
    }

    bool write_file(const char *data, size_t left);

    bool use_file() const { return use_file_; }

private:
    bool open_file(const std::string &filename, int dest
                   , ::mode_t mode);

    void retie();

    int safeDup2(int oldfd, int newfd);

    bool use_file_; //!< Log to configured file
    std::string filename_; //!< log file filename
    int fd_; //!< fd associated with output file

    boost::mutex m_;
    std::set<int> ties_;
};

} // namespace dbglog

#endif // dbglog_logfile_hpp_included_
