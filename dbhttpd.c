#include <db_185.h>

#define HTTPSERVER_IMPL
#include "httpbuild/src/httpserver.h"

void *db;

void handle_get(struct http_request_s *req, struct http_response_s *res) {}

void handle_put(struct http_request_s *req, struct http_response_s *res) {}

void handle_request(struct http_request_s *req) {
	http_string_t method = http_request_method(req);
	http_string_t target = http_request_target(req);
	struct http_response_s *res = http_response_init();
	http_response_header(res, "Content-Type", "text/plain");

	/* be naughty and dont check the entire method name,
	 * GET and PUT are the only 3-char methods starting
	 * with G or P */
	if (method.len != 3)
		goto err;

	switch (*method.buf) {
	case 'G': return handle_get(req, res);
	case 'P': return handle_put(req, res);
	}

err:
	http_response_status(res, 405);
	http_response_body(res, "405 owo whats this?\n", 20);
	http_respond(req, res);
}

int main(int argc, char *argv[]) {
	int port = 8030;
	if (argc > 1) {
		db = dbopen(argv[1], O_CREAT|O_RDWR|F_RDLCK, 0644, DB_HASH, NULL);
		if (argc == 3 && (port = atoi(argv[2])) == 0) {
			printf("what port\n");
			return 2;
		}
	} else {
		printf("usage: %s file [port]\n", *argv);
		return 3;
	}
	struct http_server_s *srv = http_server_init(port, handle_request);
	http_server_listen(srv);
}
