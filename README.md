# dbhttpd

berkeley db glued to an http server

there is probably no real use for this, but why not?

this is probably insecure, and anyone can `PUT` limitless files,
it would be a bad idea to expose this to the internet.

# dbhttpc

dbhttpd glued to a work-in-progress fuse filesystem, since all the
existing http fuse filesystems i can find seem to depend on range
requests and do not handle writing via `PUT` like dbhttpd needs.

currently does not allow accessing every path that dbhttpd supports,
since fuse has to respect normal path rules while dbhttpd just uses
paths as arbitrary keys.

