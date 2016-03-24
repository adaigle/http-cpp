#include "gtest/gtest.h"

#include <memory>

#include "http_service.h"

class http_conformance_method_test : public ::testing::Test {
protected:
    virtual void SetUp() {
        service_ = std::make_unique<http_service>("method_conformance", "method_conformance", "method_conformance");
    }

    virtual void TearDown() {
        // Nothing to do...
    }

    std::unique_ptr<http_service> service_;
};

TEST_F (http_conformance_method_test, get) {

    const std::string request = "GET /basic.html HTTP/1.1\r\n\r\n";
    http_request structured_request = http_service::parse_request(request);

    http_response response = service_->execute(structured_request);

    EXPECT_EQ(http_constants::status::http_ok, response.status_code);
    EXPECT_FALSE(response.message_body.empty()) << "";
}

TEST_F (http_conformance_method_test, head) {
    const std::string request = "HEAD /basic.html HTTP/1.1\r\n\r\n";
    const http_request structured_request = http_service::parse_request(request);

    http_response response = service_->execute(structured_request);

    ///////////////////////////////////////////////////////
    // Checking basic HEAD request success.
    EXPECT_EQ(http_constants::status::http_ok, response.status_code);
    EXPECT_TRUE(response.message_body.empty()) << "RFC2616 section 9.4: The server MUST NOT return a message-body in the response.";

    // Make sure the meta-information in a HEAD request is identitical to the one returned in a GET request.
    http_request get_structured_request = structured_request;
    get_structured_request.method = http_constants::method::m_get;
    http_response get_response = service_->execute(get_structured_request);

    ///////////////////////////////////////////////////////
    // Comparing 'general header'
    for (const auto& header : get_response.general_header) {
        const auto it = response.general_header.find(header.first);
        EXPECT_NE(it, response.general_header.cend());
        if (it != response.general_header.cend()) {
            EXPECT_EQ(it->second, header.second) << "RFC2616 section 9.4: The metainformation contained in the HTTP headers in response to a HEAD request SHOULD be identical to the information sent in response to a GET request.";
        }
    }
    EXPECT_EQ(get_response.general_header.size(), response.general_header.size());

    ///////////////////////////////////////////////////////
    // Comparing 'response header'
    for (const auto& header : get_response.response_header) {
        const auto it = response.response_header.find(header.first);
        EXPECT_NE(it, response.response_header.cend());
        if (it != response.response_header.cend()) {
            EXPECT_EQ(it->second, header.second) << "RFC2616 section 9.4: The metainformation contained in the HTTP headers in response to a HEAD request SHOULD be identical to the information sent in response to a GET request.";
        }
    }
    EXPECT_EQ(get_response.response_header.size(), response.response_header.size());

    ///////////////////////////////////////////////////////
    // Comparing 'entity header'
    for (const auto& header : get_response.entity_header) {
        const auto it = response.entity_header.find(header.first);
        EXPECT_NE(it, response.entity_header.cend());
        if (it != response.entity_header.cend()) {
            EXPECT_EQ(it->second, header.second) << "RFC2616 section 9.4: The metainformation contained in the HTTP headers in response to a HEAD request SHOULD be identical to the information sent in response to a GET request.";
        }
    }
    EXPECT_EQ(get_response.entity_header.size(), response.entity_header.size());
}

TEST_F (http_conformance_method_test, post) {

    ///////////////////////////////////////////////////////
    // Invalid request
    {
        const std::string invalid_request = "POST /invalid.html HTTP/1.1\r\n\r\n";
        http_request structured_request = http_service::parse_request(invalid_request);

        http_response response = service_->execute(structured_request);

        EXPECT_TRUE(response.status_code == http_constants::status::http_ok || response.status_code == http_constants::status::http_no_content) << "RFC2616 section 9.5: The action performed by the POST method might not result in a resource that can be identified by a URI. In this case, either 200 (OK) or 204 (No Content) is the appropriate response status, depending on whether or not the response includes an entity that describes the result.";
    }

    ///////////////////////////////////////////////////////
    // Valid request
    {
        const std::string valid_request = "POST /basic.html HTTP/1.1\r\n\r\n";
        http_request structured_request = http_service::parse_request(valid_request);

        http_response response = service_->execute(structured_request);

        EXPECT_EQ(http_constants::status::http_created, response.status_code) << "RFC2616 section 9.5: If a resource has been created on the origin server, the response SHOULD be 201 (Created).";
        const auto it = response.response_header.find("Location");
        EXPECT_NE(it, response.response_header.cend()) << "RFC2616 section 9.5: If a resource has been created on the origin server, the response SHOULD contain a Location header (see section 14.30).";
        if (it != response.response_header.cend()) {
            EXPECT_FALSE(it->second.empty());
        }
    }
}
