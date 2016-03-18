#include "gtest/gtest.h"

#include <memory>

#include "http_service.h"

class http_conformance_method_test : public ::testing::Test {
protected:
    virtual void SetUp() {
        service_ = std::make_unique<http_service>("method_conformance", "method_conformance");
    }

    virtual void TearDown() {
        // Nothing to do...
    }

    std::unique_ptr<http_service> service_;
};

TEST_F (http_conformance_method_test, get) {

    const std::string request = "GET /basic.html HTTP/1.1\r\n\r\n";
    http_request structured_request = service_->parse_request(request);

    http_response response = service_->execute(structured_request);

    EXPECT_EQ(200, response.status_code);
    EXPECT_EQ(false, response.message_body.empty());
}

TEST_F (http_conformance_method_test, head) {
    const std::string request = "HEAD /basic.html HTTP/1.1\r\n\r\n";
    http_request structured_request = service_->parse_request(request);

    http_response response = service_->execute(structured_request);

    EXPECT_EQ(200, response.status_code);
    EXPECT_EQ(true, response.message_body.empty());
}
