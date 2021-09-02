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

#include "../logfile.hpp"

#include <io.h>

namespace dbglog {

logger_file::logger_file()
    : use_file_(false), fd_(-1)
{
    if (::_sopen_s(&fd_, "NUL", _O_WRONLY, _SH_DENYNO
                   , detail::DefaultMode))
    {
        throw std::runtime_error
            ("Cannot open NUL for log file. Oops.");
    }
}

logger_file::~logger_file() {
    if (-1 == fd_) {
        return;
    }

    ::_close(fd_);

    for (auto fd : ties_) {
        ::_close(fd);
    }
}

bool logger_file::log_file(const std::string &filename
              , int mode)
{
    boost::mutex::scoped_lock guard(m_);
    if (filename.empty()) {
        if (!open_file("NUL", fd_, mode)) {
            return false;
        }
        use_file_ = false;
        retie(); // back to NUL to allow log file to be closed
    } else {
        if (!open_file(filename, fd_, mode)) {
            return false;
        }
        use_file_ = true;
        retie(); // point ties to freshly open log file
    }

    filename_ = filename;
    return true;
}

bool logger_file::log_file_truncate() {
    boost::mutex::scoped_lock guard(m_);
    if (filename_.empty()) { return false; }

    std::cerr << "Cannot truncate file on Windows." << std::endl;
    return false;
}

bool logger_file::tie(int fd, bool remember) {
    boost::mutex::scoped_lock guard(m_);

    // check for duplicate
    if (ties_.find(fd) != ties_.end()) {
        // already tied -> fine
        return true;
    }

    // TODO: add untie function + remember fd to be re-tied on file reopen
    if (-1 == safeDup2(fd_, fd)) {
        std::cerr << "Error duplicating fd(" << fd_ << ") to fd("
                  << fd << "): " << errno << std::endl;
        return false;
    }

    // remember fd
    if (remember) {
        ties_.insert(fd);
    }
    return true;
}

bool logger_file::untie(int fd, const std::string &path
           , int mode)
{
    boost::mutex::scoped_lock guard(m_);

    // check for existence
    if (ties_.find(fd) == ties_.end()) {
        // not tied -> error
        return false;
    }

    // point fd to something else
    if (!open_file(path, fd, mode)) {
        return false;
    }

    // forget fd
    ties_.erase(fd);
    return true;
}

bool logger_file::log_file_owner(int owner, int group) {
    return false;
}

bool closeOnExec(bool value) {
    return false;
}

bool logger_file::write_file(const char *data, std::size_t left) {
    if (!use_file_) {
        return false;
    }

    while (left) {
        auto written(TEMP_FAILURE_RETRY(::_write(fd_, data, int(left))));
        if (-1 == written) {
            std::cerr << "Error writing to log file: "
                      << errno << std::endl;
            break;
        }
        left -= written;
        data += written;
    }
    return true;
}

bool logger_file::open_file(const std::string &filename, int dest, int mode) {
    class file_closer {
    public:
        file_closer(const char *filename, int mode)
            : fd_(-1)
        {
            ::_sopen_s(&fd_, filename, _O_WRONLY | _O_CREAT | _O_APPEND
                       , _SH_DENYNO, mode);
        }

        ~file_closer() {
            if (-1 == fd_) {
                return;
            }

            ::_close(fd_);
        }

        operator int() {
            return fd_;
        }

    private:
        int fd_;
    } f(filename.c_str(), mode);

    if (-1 == f) {
        std::cerr << "Error opening log file <" << filename
                  << ">: " << errno << std::endl;
        return false;
    }

    if (-1 == safeDup2(f, dest)) {
        std::cerr << "Error duplicating fd(" << f << ") to fd("
                  << dest << "): " << errno << std::endl;
        return false;
    }

    return true;
}

void logger_file::retie() {
    // retie all tied file descriptors
    for (auto fd : ties_) {
        if (-1 == safeDup2(fd_, fd)) {
            std::cerr << "Error duplicating fd(" << fd_ << ") to fd("
                      << fd << "): " << errno << std::endl;
        }
    }
}

int logger_file::safeDup2(int oldfd, int newfd) {
    for (;;) {
        int res(::_dup2(oldfd, newfd));
        if (res != -1) {
            // OK
            return res;
        }

        // some error
        switch (res) {
        case EBUSY:
            // race condition between open and dup -> try again
        case EINTR:
            // interrupted -> try again
            continue;
        }

        // fatal error
        return res;
    }
}

} // namespace dbglog

