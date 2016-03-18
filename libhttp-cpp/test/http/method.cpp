#include "gtest/gtest.h"

#include "http_service.h"

TEST (http_conformance_test, get) {
    http_service service("method_conformance", "method_conformance");

    const std::string request = "GET /basic.html HTTP/1.1\r\n\r\n";
    http_request structured_request = service.parse_request(request);

    http_response response = service.execute(structured_request);

    EXPECT_EQ(200, response.status_code);
    EXPECT_EQ(false, response.message_body.empty());
}

TEST (http_conformance_test, head) {
    http_service service("method_conformance", "method_conformance");

    const std::string request = "HEAD /basic.html HTTP/1.1\r\n\r\n";
    http_request structured_request = service.parse_request(request);

    http_response response = service.execute(structured_request);

    EXPECT_EQ(200, response.status_code);
    EXPECT_EQ(true, response.message_body.empty());
}
