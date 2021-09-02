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
#include <new>

#include <sys/stat.h>
#include <unistd.h>

#include <pthread.h>

#include "lockfile.hpp"

namespace utility {

class LockFiles::Lock::Internals {
public:
    Internals(const boost::filesystem::path &path
              , ino_t inode, int fd
              , const LockFiles::wpointer &locker)
        : path_(path), inode_(inode), fd_(fd), locker_(locker)
    {}

    ~Internals() {
        auto locker(locker_.lock());
        if (locker) {
            locker->destroy(inode_);
        }

        if (-1 == ::close(fd_)) {
            std::system_error e(errno, std::system_category());
            LOG(err1) << "Cannot close lock " << path_ << " file: <"
                      << e.code() << ", " << e.what() << ">.";
        }
    }

    void lock();
    void unlock();

    void fork_prepare();
    void fork_parent();
    void fork_child();

private:
    // in-process lock
    std::mutex mutex_;
    // inter-process lock
    boost::filesystem::path path_;
    ino_t inode_;
    int fd_;
    LockFiles::wpointer locker_;
};

LockFiles::Lock::Lock(const std::shared_ptr<Lock::Internals> &lock)
    : lock_(lock)
{}

void LockFiles::Lock::lock() { lock_->lock(); }
void LockFiles::Lock::unlock() { lock_->unlock(); }

// implement TEMP_FAILURE_RETRY if not present on platform (via C++11 lambda)
#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(operation) [&]()->int {       \
        for (;;) { int e(operation);                     \
            if ((-1 == e) && (EINTR == errno)) continue; \
            return e;                                    \
        }                                                \
    }()
#endif

void LockFiles::Lock::Internals::lock()
{
    LOG(debug) << "Locking " << path_ << " (" << inode_ << "/" << fd_ << ").";

    // make lock unique in this process
    std::unique_lock<std::mutex> guard(mutex_);

    struct ::flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    auto res(TEMP_FAILURE_RETRY(::fcntl(fd_, F_SETLKW, &lock)));
    if (-1 == res) {
        std::system_error e(errno, std::system_category());
        LOG(err2) << "Cannot lock file " << path_ << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }

    // Everything is fine, keep locked
    guard.release();
}

void LockFiles::Lock::Internals::unlock()
{
    LOG(debug) << "Unlocking " << path_ << " (" << inode_ << "/" << fd_ << ").";

    // make lock unique in this process
    std::unique_lock<std::mutex> guard(mutex_, std::adopt_lock);

    struct ::flock lock;
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    auto res(TEMP_FAILURE_RETRY(::fcntl(fd_, F_SETLKW, &lock)));
    if (-1 == res) {
        std::system_error e(errno, std::system_category());
        LOG(err2) << "Cannot unlock file " << path_ << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }

    // TODO: what to do if the above throws? mutex unlocked, file
    // locked... hmmm?
}

LockFiles::Lock LockFiles::create(const boost::filesystem::path &path)
{
    auto self(shared_from_this());

    // open file and determine its inode
    auto fd(::open(path.string().c_str(), O_RDWR));
    if (-1 == fd) {
        std::system_error e(errno, std::system_category());
        LOG(err1) << "Cannot open lock file " << path << ": <"
                  << e.code() << ", " << e.what() << ">.";
        throw e;
    }

    struct stat buf;
    if (-1 == ::fstat(fd, &buf)) {
        std::system_error e(errno, std::system_category());
        LOG(err1) << "Cannot stat lock file " << path << ": <"
                  << e.code() << ", " << e.what() << ">.";
        ::close(fd);
        throw e;
    }

    // lock access to map
    std::unique_lock<std::mutex> guard(mapLock_);

    auto inode(buf.st_ino);
    auto fmap(map_.find(inode));
    if (fmap != map_.end()) {
        // already in lock map
        auto l(fmap->second.lock());
        if (l) {
            ::close(fd);
            return Lock(l);
        }
        // stale entry -> remove
        map_.erase(fmap);
    }

    // new entry (file should be kept open)
    auto lock(std::make_shared<Lock::Internals>(path, inode, fd, self));

    map_.insert(std::make_pair(inode, lock));

    return Lock(lock);
}

void LockFiles::destroy(ino_t inode)
{
    // remove inode info from lock map
    std::unique_lock<std::mutex> guard(mapLock_);
    map_.erase(inode);
}

LockFiles::pointer lockFiles;

void LockFiles::Lock::Internals::fork_prepare()
{
    mutex_.lock();
}

void LockFiles::Lock::Internals::fork_parent()
{
    mutex_.unlock();
}

void LockFiles::Lock::Internals::fork_child()
{
    new ((void*) &mutex_) std::mutex();
}

void LockFiles::fork_prepare()
{
    // lock mapLock before fork
    mapLock_.lock();

    // re-initialize mutexes in all lock files
    for (auto &i : map_) {
        auto internals(i.second.lock());
        if (internals) {
            internals->fork_prepare();
        }
    }
}

void LockFiles::fork_parent()
{
    // re-initialize mutexes in all lock files
    for (auto &i : map_) {
        auto internals(i.second.lock());
        if (internals) {
            internals->fork_parent();
        }
    }

    // unlock it after fork
    mapLock_.unlock();
}

void LockFiles::fork_child()
{

    // we need to re-initialize the mutex (ugly as hell, I DO know...)
    new ((void*) &mapLock_) std::mutex();

    // not needed to be locked, this is single threaded

    // re-initialize mutexes in all lock files
    for (auto &i : map_) {
        auto internals(i.second.lock());
        if (internals) {
            internals->fork_child();
        }
    }

    // destroy all files
    map_.clear();
}

struct LockFilesInitializer {
    static LockFilesInitializer instance;
    LockFilesInitializer();

    void fork_prepare() { lockFiles->fork_prepare(); }

    void fork_parent() { lockFiles->fork_parent(); }

    void fork_child() { lockFiles->fork_child(); }
};
LockFilesInitializer LockFilesInitializer::instance;

extern "C" {

void fork_prepare()
{
    LockFilesInitializer::instance.fork_prepare();
}

void fork_parent()
{
    LockFilesInitializer::instance.fork_parent();
}

void fork_child()
{
    LockFilesInitializer::instance.fork_child();
}

LockFilesInitializer::LockFilesInitializer() {
    utility::lockFiles.reset(new LockFiles);
    ::pthread_atfork(&utility::fork_prepare, &utility::fork_parent
                     , &utility::fork_child);
}

} // extern C

} // namespace utility
