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
            self.cache[path] = req.content

        return self.cache[path]

    # TODO: handle making fake directories, since fuse does not let us
    # have slashes in file names and dbhttpd does not have a concept
    # of normal path rules
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

    def read(self, path, size, offset, fh):
        return self._getfile(path)[offset : offset + size]

    def release(self, path, fh):
        if path in self.modified:
            requests.put(url=self.prefix + path, data=self.cache[path])
            self.modified.remove(path)
        del self.cache[path]


if __name__ == "__main__":
    prefix = sys.argv[1]
    mountpoint = sys.argv[2]

    FUSE(DbFS(prefix), mountpoint, nothreads=True, foreground=True)
