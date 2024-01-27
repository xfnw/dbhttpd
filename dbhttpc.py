#!/usr/bin/env python3

import sys, errno
from stat import S_IFDIR, S_IFREG
from fuse import FUSE, FuseOSError, Operations


class DbFS(Operations):
    def __init__(self, prefix):
        self.prefix = prefix
        self.cache = {}

    def _getfile(self, path):
        if path not in self.cache:
            self.cache[path] = b"meow meow meow"

        return self.cache[path]

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


if __name__ == "__main__":
    prefix = sys.argv[1]
    mountpoint = sys.argv[2]

    FUSE(DbFS(prefix), mountpoint, nothreads=True, foreground=True)
