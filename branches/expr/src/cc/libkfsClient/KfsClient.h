//---------------------------------------------------------- -*- Mode: C++ -*-
// $Id$ 
//
// Created 2006/04/18
// Author: Sriram Rao (Kosmix Corp.) 
//
// Copyright 2006 Kosmix Corp.
//
// This file is part of Kosmos File System (KFS).
//
// Licensed under the Apache License, Version 2.0
// (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.
//
// \file KfsClient.h
// \brief Kfs Client-library code.
//
//----------------------------------------------------------------------------

#ifndef LIBKFSCLIENT_KFSCLIENT_H
#define LIBKFSCLIENT_KFSCLIENT_H

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

#include <sys/stat.h>

#include "KfsAttr.h"

namespace KFS {

class KfsClientImpl;

/// Maximum length of a filename
const size_t MAX_FILENAME_LEN = 256;

///
/// \brief The KfsClient is the "bridge" between applications and the
/// KFS servers (either the metaserver or chunkserver): there can be
/// only one client per application; the client can interface with
/// only one metaserver.
///
class KfsClient {
    // Make the constructor private to get a Singleton.
    KfsClient();
    KfsClient(const KfsClient &other);
    const KfsClient & operator=(const KfsClient &other);

public:
    static KfsClient *Instance() {
        static KfsClient instance;
        return &instance;
    }
    ///
    /// @param[in] propFile that describes where the server is and
    /// other client configuration info.
    /// @retval 0 on success; -1 on failure
    ///
    int Init(const char *propFile);

    ///
    /// @param[in] metaServerHost  Machine on meta is running
    /// @param[in] metaServerPort  Port at which we should connect to
    /// @retval 0 on success; -1 on failure
    ///
    int Init(const std::string metaServerHost, int metaServerPort);

    bool IsInitialized();

    ///
    /// Provide a "cwd" like facility for KFS.
    /// @param[in] pathname  The pathname to change the "cwd" to
    /// @retval 0 on sucess; -errno otherwise
    ///
    int Cd(const char *pathname);

    /// Get cwd
    /// @retval a string that describes the current working dir.
    ///
    std::string GetCwd();

    ///
    /// Make a directory hierarcy in KFS.  If the parent dirs are not
    /// present, they are also made.
    /// @param[in] pathname		The full pathname such as /.../dir
    /// @retval 0 if mkdir is successful; -errno otherwise
    int Mkdirs(const char *pathname);

    ///
    /// Make a directory in KFS.
    /// @param[in] pathname		The full pathname such as /.../dir
    /// @retval 0 if mkdir is successful; -errno otherwise
    int Mkdir(const char *pathname);

    ///
    /// Remove a directory in KFS.
    /// @param[in] pathname		The full pathname such as /.../dir
    /// @retval 0 if rmdir is successful; -errno otherwise
    int Rmdir(const char *pathname);

    ///
    /// Remove a directory hierarchy in KFS.
    /// @param[in] pathname		The full pathname such as /.../dir
    /// @retval 0 if rmdir is successful; -errno otherwise
    int Rmdirs(const char *pathname);

    ///
    /// Read a directory's contents
    /// @param[in] pathname	The full pathname such as /.../dir
    /// @param[out] result	The contents of the directory
    /// @retval 0 if readdir is successful; -errno otherwise
    int Readdir(const char *pathname, std::vector<std::string> &result);

    ///
    /// Read a directory's contents and retrieve the attributes
    /// @param[in] pathname	The full pathname such as /.../dir
    /// @param[out] result	The files in the directory and their attributes.
    /// @retval 0 if readdirplus is successful; -errno otherwise
    ///
    int ReaddirPlus(const char *pathname, std::vector<KfsFileAttr> &result);

    ///
    /// Stat a file and get its attributes.
    /// @param[in] pathname	The full pathname such as /.../foo
    /// @param[out] result	The attributes that we get back from server
    /// @param[in] computeFilesize  When set, for files, the size of
    /// file is computed and the value is returned in result.st_size
    /// @retval 0 if stat was successful; -errno otherwise
    ///
    int Stat(const char *pathname, struct stat &result, bool computeFilesize = true);

    ///
    /// Helper APIs to check for the existence of (1) a path, (2) a
    /// file, and (3) a directory.
    /// @param[in] pathname	The full pathname such as /.../foo
    /// @retval status: True if it exists; false otherwise
    ///
    bool Exists(const char *pathname);
    bool IsFile(const char *pathname);
    bool IsDirectory(const char *pathname);

    ///
    /// Create a file which is specified by a complete path.
    /// @param[in] pathname that has to be created
    /// @param[in] numReplicas the desired degree of replication for
    /// the file.
    /// @param[in] exclusive  create will fail if the exists (O_EXCL flag)
    /// @retval on success, fd corresponding to the created file;
    /// -errno on failure.
    ///
    int Create(const char *pathname, int numReplicas = 3, bool exclusive = false);

    ///
    /// Remove a file which is specified by a complete path.
    /// @param[in] pathname that has to be removed
    /// @retval status code
    ///
    int Remove(const char *pathname);

    ///
    /// Rename file/dir corresponding to oldpath to newpath
    /// @param[in] oldpath   path corresponding to the old name
    /// @param[in] newpath   path corresponding to the new name
    /// @param[in] overwrite  when set, overwrite the newpath if it
    /// exists; otherwise, the rename will fail if newpath exists
    /// @retval 0 on success; -1 on failure
    ///
    int Rename(const char *oldpath, const char *newpath, bool overwrite = true);

    ///
    /// Open a file
    /// @param[in] pathname that has to be opened
    /// @param[in] openFlags modeled after open().  The specific set
    /// of flags currently supported are:
    /// O_CREAT, O_CREAT|O_EXCL, O_RDWR, O_RDONLY, O_WRONLY, O_TRUNC, O_APPEND
    /// @param[in] numReplicas if O_CREAT is specified, then this the
    /// desired degree of replication for the file
    /// @retval fd corresponding to the opened file; -errno on failure
    ///
    int Open(const char *pathname, int openFlags, int numReplicas = 3);

    ///
    /// Return file descriptor for an open file
    /// @param[in] pathname of file
    /// @retval file descriptor if open, error code < 0 otherwise
    int Fileno(const char *pathname);

    ///
    /// Close a file
    /// @param[in] fd that corresponds to a previously opened file
    /// table entry.
    ///
    int Close(int fd);

    ///
    /// Read/write the desired # of bytes to the file, starting at the
    /// "current" position of the file.
    /// @param[in] fd that corresponds to a previously opened file
    /// table entry.
    /// @param buf For read, the buffer will be filled with data; for
    /// writes, this buffer supplies the data to be written out.
    /// @param[in] numBytes   The # of bytes of I/O to be done.
    /// @retval On success, return of bytes of I/O done (>= 0);
    /// on failure, return status code (< 0).
    ///
    ssize_t Read(int fd, char *buf, size_t numBytes);
    ssize_t Write(int fd, const char *buf, size_t numBytes);

    ///
    /// \brief Sync out data that has been written (to the "current" chunk).
    /// @param[in] fd that corresponds to a file that was previously
    /// opened for writing.
    ///
    int Sync(int fd);

    /// \brief Adjust the current position of the file pointer similar
    /// to the seek() system call.
    /// @param[in] fd that corresponds to a previously opened file
    /// @param[in] offset offset to which the pointer should be moved
    /// relative to whence.
    /// @param[in] whence one of SEEK_CUR, SEEK_SET, SEEK_END
    /// @retval On success, the offset to which the filer
    /// pointer was moved to; (off_t) -1 on failure.
    ///
    off_t Seek(int fd, off_t offset, int whence);
    /// In this version of seek, whence == SEEK_SET
    off_t Seek(int fd, off_t offset);

    /// Return the current position of the file pointer in the file.
    /// @param[in] fd that corresponds to a previously opened file
    /// @retval value returned is analogous to calling ftell()
    off_t Tell(int fd);

    ///
    /// Truncate a file to the specified offset.
    /// @param[in] fd that corresponds to a previously opened file
    /// @param[in] offset  the offset to which the file should be truncated
    /// @retval status code
    int Truncate(int fd, off_t offset);

    ///
    /// Given a starting offset/length, return the location of all the
    /// chunks that cover this region.  By location, we mean the name
    /// of the chunkserver that is hosting the chunk. This API can be
    /// used for job scheduling.
    ///
    /// @param[in] pathname	The full pathname of the file such as /../foo
    /// @param[in] start	The starting byte offset
    /// @param[in] len		The length in bytes that define the region
    /// @param[out] locations	The location(s) of various chunks
    /// @retval status: 0 on success; -errno otherwise
    ///
    int GetDataLocation(const char *pathname, off_t start, size_t len,
                        std::vector< std::vector <std::string> > &locations);

    ///
    /// Get the degree of replication for the pathname.
    /// @param[in] pathname	The full pathname of the file such as /../foo
    /// @retval count
    ///
    int16_t GetReplicationFactor(const char *pathname);

    ///
    /// Set the degree of replication for the pathname.
    /// @param[in] pathname	The full pathname of the file such as /../foo
    /// @param[in] numReplicas  The desired degree of replication.
    /// @retval -1 on failure; on success, the # of replicas that will be made.
    ///
    int16_t SetReplicationFactor(const char *pathname, int16_t numReplicas);

private:
    KfsClientImpl *mImpl;
};

/// Given a error status code, return a string describing the error.
/// @param[in] status  The status code for an error.
/// @retval String that describes what the error is.
extern std::string ErrorCodeToStr(int status);

extern KfsClient *getKfsClient();

}

#endif // LIBKFSCLIENT_KFSCLIENT_H