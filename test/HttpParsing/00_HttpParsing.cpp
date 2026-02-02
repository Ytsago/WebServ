#include <iostream>

void	test_basic_get_request();
void	test_post_with_body();
void	test_fragmented_parsing();
void	test_empty_body_with_content_length_zero();
void	test_uri_too_long();
void	test_invalid_http_version();
void	test_missing_spaces_in_request_line();
void	test_negative_content_length();
void	test_body_too_large();
void	test_header_without_colon();
void	test_chunked_encoding();
void	test_multiple_requests();
void	test_header_value_trimming();
void	test_large_body_fragmented();
void	test_case_insensitive_headers();

void	HttpParserLauncher() {
	std::cout << "========== HTTP Parser Test Suite ==========\n\n";

	test_basic_get_request();
	test_post_with_body();
	test_fragmented_parsing();
	test_empty_body_with_content_length_zero();
	test_uri_too_long();
	test_invalid_http_version();
	test_missing_spaces_in_request_line();
	test_negative_content_length();
	test_body_too_large();
	test_header_without_colon();
	test_chunked_encoding();
	test_multiple_requests();
	test_header_value_trimming();
	test_large_body_fragmented();
	test_case_insensitive_headers();
	
	std::cout << "\n========== Tests Complete ==========\n";
}
