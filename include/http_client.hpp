#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <string>
#include <boost/asio.hpp>

class http_client {
private:
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::streambuf request_;
    boost::asio::streambuf response_;

public:
    http_client(boost::asio::io_service& io_service,
		const std::string& hostname, const std::string& path);
    virtual ~http_client();

private:
    void handle_resolve(const boost::system::error_code& err,
			boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

    void handle_connect(const boost::system::error_code& err,
			boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

    void handle_write_request(const boost::system::error_code& err);

    void handle_read_status_line(const boost::system::error_code& err);

    void handle_read_headers(const boost::system::error_code& err);

    void handle_read_content(const boost::system::error_code& err);

protected:
    virtual void process_content(const std::string& content) = 0;
};


#endif /* HTTP_CLIENT_H */
