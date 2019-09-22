
#include <string>


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
	const std::string& filename, const char* pData, uint64_t dataSize);

int post(const std::string& host, const std::string& port, const std::string& url,
	const std::string& filename, const std::string& data);
