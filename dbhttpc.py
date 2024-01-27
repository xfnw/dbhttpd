#!/usr/bin/env python3

import sys, errno, requests
from stat import S_IFDIR, S_IFREG
from fuse import FUSE, FuseOSError, Operations


class DbFS(Operations):
    def __init__(self, prefix):
        self.prefix = prefix
        self.modified = set()
        self.cache = dict()

    def _getfile(self, path):
        if path not in self.cache:
            req = requests.get(self.prefix + path)
            if req.status_code == 404:
                # dbhttpd does not have a distinction between
                # empty and nonexistent files
                self.cache[path] = b''
            else:
                self.cache[path] = req.content

        return self.cache[path]

    # TODO: handle automatically making fake directories, since fuse
    # does not let us have slashes in file names and dbhttpd does not
    # have a concept of normal path rules
    def getattr(self, path, fh=None):
        if path == "/":
            m = S_IFDIR
            size = 0
        else:
            m = S_IFREG
            size = len(self._getfile(path))

        return dict(
            st_mode=m | 0o777,
            st_ctime=0,
            st_atime=0,
            st_mtime=0,
            st_nlink=2,
            st_size=size,
        )

    def truncate(self, path, length, fh=None):
        old = self._getfile(path)
        self.cache[path] = old[:length].ljust(length, b'\0')


    def unlink(self, path):
        requests.put(url=self.prefix + path);

        if path in self.modified:
            self.modified.remove(path)
        if path in self.cache:
            del self.cache[path]

    def read(self, path, size, offset, fh):
        return self._getfile(path)[offset : offset + size]

    def write(self, path, buf, offset, fh):
        old = self._getfile(path)
        self.cache[path] = old[:offset].ljust(offset, b'\0') + buf + old[offset + len(buf) :]
        self.modified.add(path)

        # what is this supposed to do?
        # borrowed from example, errors out without it...
        return len(self.cache)

    def release(self, path, fh):
        if path in self.modified:
            requests.put(url=self.prefix + path, data=self.cache[path])
            self.modified.remove(path)
        if path in self.cache:
            del self.cache[path]


if __name__ == "__main__":
    prefix = sys.argv[1]
    mountpoint = sys.argv[2]

    FUSE(DbFS(prefix), mountpoint, nothreads=True, foreground=True)
