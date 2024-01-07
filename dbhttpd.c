
#include <db_185.h>

#define HTTPSERVER_IMPL
#include "httpbuild/src/httpserver.h"

void handle_request(struct http_request_s *req) {
	http_string_t method = http_request_method(req);
	http_string_t target = http_request_target(req);
	struct http_response_s *res = http_response_init();
	http_response_header(res, "Content-Type", "text/plain");

	/* be naughty and dont check the entire method name,
	 * GET and PUT are the only 3-char methods starting
	 * with G or P */
	if (method.len != 3) {
		http_response_status(res, 405);
		http_response_body(res, "405 owo whats this?\n", 20);
		http_respond(req, res);
		return;
	}

}

int main(int argc, char *argv[]) {
	struct http_server_s *srv = http_server_init(8030, handle_request);
	http_server_listen(srv);
}
