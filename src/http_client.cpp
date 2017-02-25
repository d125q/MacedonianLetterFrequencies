#include <iostream>
#include <istream>
#include <ostream>
#include <boost/bind.hpp>
#include "http_client.hpp"

using boost::asio::ip::tcp;

http_client::http_client(boost::asio::io_service& io_service,
			 const std::string& hostname, const std::string& path)
    : socket_(io_service), resolver_(io_service)
{
    // Form the request.  Specify the "Connection: close" header so that
    // the server will close the socket after transmitting the response.
    // This will allow us to treat all data up until EOF as the content.
    std::ostream request_stream(&request_);

    request_stream << "GET " << path << " HTTP/1.0\r\n";
    request_stream << "Host: " << hostname << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

    // Start the async resolve to translate the hostname into a list of
    // endpoints.
    tcp::resolver::query query(hostname, "http");
    resolver_.async_resolve(query,
			    boost::bind(&http_client::handle_resolve,
					this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::iterator));
}

http_client::~http_client() {}

void http_client::handle_resolve(const boost::system::error_code& err,
				 tcp::resolver::iterator endpoint_iterator)
{
    if (!err) {
	// Attempt to connect to one of the endpoints.
	tcp::endpoint endpoint{*endpoint_iterator};
	socket_.async_connect(endpoint,
			      boost::bind(&http_client::handle_connect,
					  this,
					  boost::asio::placeholders::error,
					  ++endpoint_iterator));
    }
    else {
	std::cerr << "Error resolving: " << err.message() << "\n";
    }
}

void http_client::handle_connect(const boost::system::error_code& err,
				 tcp::resolver::iterator endpoint_iterator)
{
    if (!err) {
	// Connected successfully.  Write the request to the socket.
	boost::asio::async_write(socket_, request_,
				 boost::bind(&http_client::handle_write_request,
					     this,
					     boost::asio::placeholders::error));
    }
    else if (endpoint_iterator != tcp::resolver::iterator()) {
	// The connection failed.  Try the next endpoint.
	socket_.close();
	tcp::endpoint endpoint{*endpoint_iterator};
	socket_.async_connect(endpoint,
			      boost::bind(&http_client::handle_connect,
					  this,
					  boost::asio::placeholders::error,
					  ++endpoint_iterator));
    }
    else {
	std::cerr << "Error connecting: " << err.message() << "\n";
    }
}

void http_client::handle_write_request(const boost::system::error_code& err)
{
    if (!err) {
	boost::asio::async_read_until(socket_, response_, "\r\n",
				      boost::bind(&http_client::handle_read_status_line,
						  this,
						  boost::asio::placeholders::error));
    }
    else {
	std::cerr << "Error writing: " << err.message() << "\n";
    }
}

void http_client::handle_read_status_line(const boost::system::error_code& err)
{
    if (!err) {
	// Read and discard the status line.
	// Additionally, make sure that a status code 200 (OK) was received.
	std::istream response_stream(&response_);
	std::string http_version;
	unsigned int status_code;
	std::string status_message;

	response_stream >> http_version;
	response_stream >> status_code;
	std::getline(response_stream, status_message);

	if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
	    std::cerr << "Invalid response\n";
	    return;
	}

	if (status_code != 200) {
	    std::cerr << "Response returned with status code "
		      << status_code << "\n";
	    return;
	}

	// Read the headers.  They are separated from by payload
	// by a blank line.
	boost::asio::async_read_until(socket_, response_, "\r\n\r\n",
				      boost::bind(&http_client::handle_read_headers,
						  this,
						  boost::asio::placeholders::error));
    }
    else {
	std::cerr << "Error reading status line: " << err.message() << "\n";
    }
}

void http_client::handle_read_headers(const boost::system::error_code& err)
{
    if (!err) {
	// Discard the headers.
	std::istream response_stream(&response_);
	std::string header_line;

	while (std::getline(response_stream, header_line) && header_line != "\r");

	// Read the payload in chunks of at least 512 bytes each.
	boost::asio::async_read(socket_, response_,
				boost::asio::transfer_at_least(512),
				boost::bind(&http_client::handle_read_content,
					    this,
					    boost::asio::placeholders::error));
    }
    else {
	std::cerr << "Error reading headers: " << err.message() << "\n";
    }
}

void http_client::handle_read_content(const boost::system::error_code& err)
{
    if (!err) {
	// Read some more data.
	boost::asio::async_read(socket_, response_,
				boost::asio::transfer_at_least(512),
				boost::bind(&http_client::handle_read_content,
					    this,
					    boost::asio::placeholders::error));
    }
    else if (err != boost::asio::error::eof) {
	std::cerr << "Error reading content: " << err.message() << "\n";
    }
    else {
	// EOF was reached.  Invoke the callback.
	boost::asio::streambuf::const_buffers_type response_bufs{response_.data()};
	process_content(std::string(boost::asio::buffers_begin(response_bufs),
				    boost::asio::buffers_end(response_bufs)));
    }
}

// void http_client::process_content(const std::string& content)
// {
//     UErrorCode status = U_ZERO_ERROR;
    
//     icu::UnicodeString alphabet(u8"абвгдѓежзѕијклљмнњопрстќуфхцчџш");
//     icu::UnicodeString unicode_content = icu::UnicodeString::fromUTF8(icu::StringPiece(content));
//     icu::RegexMatcher matcher(u8"а", 0, status);
//     icu::StringCharacterIterator it(unicode_content);

//     matcher.reset(unicode_content);

//     if (matcher.find()) {
// 	std::cout << matcher.start(status) << "\n";
//     }

//     // while (it.hasNext()) {
//     // 	std::cout << it.next() << " ";
//     // }
// }


// int main(int argc, char* argv[])
// {
//     try {
// 	if (argc != 3) {
// 	    std::cout << "Usage: async_client <server> <path>\n";
// 	    std::cout << "Example:\n";
// 	    std::cout << "  async_client www.boost.org /LICENSE_1_0.txt\n";
// 	    return 1;
// 	}

// 	boost::asio::io_service io_service;
// 	http_client c(io_service, argv[1], argv[2]);
// 	io_service.run();
//     }
//     catch (std::exception& e) {
// 	std::cout << "Exception: " << e.what() << "\n";
//     }

//     return 0;
// }
