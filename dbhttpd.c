#include <db_185.h>
#include <stdlib.h>
#include <time.h>

#define HTTPSERVER_IMPL
#include "httpbuild/src/httpserver.h"

#define STRIP_LEADING(in) \
	if (*(char *)(in.data) == '/') { \
		in.data += 1; \
		in.size -= 1; \
	}

DB *db;
time_t last_sync = 0;

int do_sync() {
	time_t new_time = time(NULL);
	if (new_time - 20 < last_sync)
		return 0;

	last_sync = new_time;
	db->sync(db, 0);

	return 1;
}

void handle_signal(int sig) {
	if (sig == SIGHUP) {
		printf("got sighup, syncing...\n");
		db->sync(db, 0);
		return;
	}

	db->close(db);
	exit(0);
}

void handle_get(struct http_request_s *req, struct http_response_s *res) {
	char *content_type = "text/plain";
	DBT key;
	DBT value;

	http_string_t target = http_request_target(req);
	key.data = (char *)target.buf;
	key.size = target.len;
	STRIP_LEADING(key);

	if (db->get(db, &key, &value, 0) == 0) {
		http_response_status(res, 200);
		http_response_body(res, value.data, value.size - 1);
		if (value.size > 3 && *(char *)value.data == '<') {
			switch (((char *)value.data)[1]) {
			case '?': content_type = "text/xml"; break;
			case '!': content_type = "text/html"; break;
			}
		}
	} else {
		http_response_status(res, 404);
		http_response_body(res, "404 nyot f-f-found\n", 19);
	}

	http_response_header(res, "Content-Type", content_type);
	http_respond(req, res);
}

void handle_put(struct http_request_s *req, struct http_response_s *res) {
	DBT key;
	DBT value;

	http_string_t target = http_request_target(req);
	key.data = (char *)target.buf;
	key.size = target.len;
	STRIP_LEADING(key);

	http_string_t body = http_request_body(req);
	value.data = (char *)body.buf;
	value.size = body.len + 1;

	if (body.len == 0) {
		db->del(db, &key, 0);
		http_response_status(res, 200);
		http_response_body(res, "deweted\n", 8);
	} else if (db->put(db, &key, &value, 0) == 0) {
		int sync = do_sync();
		http_response_status(res, 200);
		http_response_body(res, "<:3\n" + sync, 4 - sync);
	} else {
		http_response_status(res, 500);
		http_response_body(res, "500 s-s-s-something went wwong\n", 31);
	}

	http_response_header(res, "Content-Type", "text/plain");
	http_respond(req, res);
}

void handle_request(struct http_request_s *req) {
	http_string_t method = http_request_method(req);
	struct http_response_s *res = http_response_init();

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
	http_response_header(res, "Content-Type", "text/plain");
	http_response_status(res, 405);
	http_response_body(res, "405 owo whats this?\n", 20);
	http_respond(req, res);
}

int main(int argc, char *argv[]) {
	int port = 8030;
	if (argc > 1) {
		db = dbopen(argv[1], O_CREAT | O_RDWR | F_RDLCK, 0644, DB_HASH,
			    NULL);
		if (db == NULL) {
			printf("failed to open db\n");
			return 4;
		}
		if (argc == 3 && (port = atoi(argv[2])) == 0) {
			printf("what port\n");
			return 2;
		}
	} else {
		printf("usage: %s file [port]\n", *argv);
		return 3;
	}

	struct http_server_s *srv = http_server_init(port, handle_request);

	struct sigaction act;
	memset(&act, 0, sizeof act);
	act.sa_handler = handle_signal;

	sigaction(SIGHUP, &act, 0);
	sigaction(SIGINT, &act, 0);
	sigaction(SIGTERM, &act, 0);

	http_server_listen(srv);
}
