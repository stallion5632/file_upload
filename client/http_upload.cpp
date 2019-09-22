#include "http_upload.h"
#include <iostream>
#include <ostream>
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using boost::asio::ip::tcp;

// use multipart/form-data rfc1867
// ref: https://www.boost.org/doc/libs/1_47_0/doc/html/boost_asio/example/http/client/sync_client.cpp
// ref:https://www.jianshu.com/p/29e38bcc8a1d
// ref:https://www.cnblogs.com/linbc/p/5034286.html
// ref:https://stackoverflow.com/questions/38514601/synchronous-https-post-with-boost-asio

// @host:upload server ip 
// @port:upload server port
// @url: service location, eg:/upload
// @filename: file name to be saved by file server
// @pData: first address of the data to be saved
// @dataSize: size of the data to be saved
int post(const std::string& host, const std::string& port, const std::string& url, 
	const std::string& filename, const char* pData, uint64_t dataSize)
{
	try
	{
		boost::asio::io_service io_service;
		// if reuse io_service
		if (io_service.stopped())
			io_service.reset();

		// Get a list of endpoints corresponding to the server name.
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(host, port);
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

		// Try each endpoint until we successfully establish a connection.
		tcp::socket socket(io_service);
		boost::asio::connect(socket, endpoint_iterator);

		std::string prefix = "--";
		std::string boundary = boost::uuids::to_string(boost::uuids::random_generator()()); // Use uuid as boundary
		std::string newline = "\r\n";
		std::string contentDisposition = "Content-Disposition: form-data; name=\"file\"; filename=\"" + filename + "\"";

		// Calculate length of entire HTTP request - goes into header
		uint64_t requestLength = 0;
		requestLength += prefix.length() + boundary.length() + newline.length();
		requestLength += contentDisposition.length();
		requestLength += newline.length() + newline.length();
		requestLength += dataSize;
		requestLength += newline.length() + prefix.length() + boundary.length() + prefix.length() + newline.length();


		// Form the request. We specify the "Connection: close" header so that the
		// server will close the socket after transmitting the response. This will
		// allow us to treat all data up until the EOF as the content.
		std::stringstream ss;
		ss << "POST " << url << " HTTP/1.1" << newline;;
		ss << "Host: " << host << ":" << port << newline;
		ss << "Accept: */*" << newline;
		ss << "Content-Length: " << requestLength << newline;
		ss << "Content-Type: multipart/form-data; boundary=" << boundary << newline;
		ss << "Connection: close" << newline;
		ss << newline;

		ss << prefix;
		ss << boundary << newline;
		ss << contentDisposition << newline;;
		ss << newline;

		socket.write_some(boost::asio::buffer(ss.str()));

		// Send Data 
		uint64_t sent = 0;
		while (sent < dataSize)
		{
			try
			{
				uint64_t now = std::min((uint64_t)(dataSize - sent), (uint64_t)(1024*1024));
				socket.write_some(boost::asio::buffer((char*)pData + sent, now));
				sent += now;
			}
			catch(const std::exception& e)
			{
				std::cerr << e.what() << '\n';
				return -1;
			}
		}

		
		std::string endStr = newline + prefix + boundary + prefix + newline;
		//Close request
		socket.write_some(boost::asio::buffer(endStr));
		
		// Read the response status line. The response streambuf will automatically
		// grow to accommodate the entire line. The growth may be limited by passing
		// a maximum size to the streambuf constructor.
		boost::asio::streambuf response;
		boost::asio::read_until(socket, response, "\r\n");

		// Check that response is OK.
		std::istream response_stream(&response);
		std::string http_version;
		response_stream >> http_version;
		unsigned int status_code;
		response_stream >> status_code;
		std::string status_message;
		std::getline(response_stream, status_message);
		if (!response_stream || http_version.substr(0, 5) != "HTTP/")
		{
			std::cout << "Invalid response\n";
			return -2;
		}
		if (status_code != 200)
		{
			std::cout << "Response returned with status code " << status_code << "\n";
			return -3;
		}

		// Read the response headers, which are terminated by a blank line.
		boost::asio::read_until(socket, response, "\r\n\r\n");

		// Read until EOF, writing data to output as we go.
		boost::system::error_code error;
		while (boost::asio::read(socket, response,
			boost::asio::transfer_at_least(1), error))
			std::cout << &response;
		if (error != boost::asio::error::eof)
			return -4;
	}
	catch(const std::exception& e)
	{
		std::cout << e.what() << '\n';
		return -5;
	}
	
	return 0;
}

int post(const std::string& host, const std::string& port, const std::string& url,
	const std::string& filename, const std::string& data)
{
	return post(host, port, url, filename, data.c_str(), data.length());
}
