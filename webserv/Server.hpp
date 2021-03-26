#pragma once
#include <IServer.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <exception>
#include <fcntl.h>
#include <unistd.h>

static const char webpage[] =
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>"
"<html>"
"<head>"
"<title>Welcome to myNGINX!</title>"
"<style>"
"    body {"
"        width: 35em;"
"        margin: 0 auto;"
"        font-family: Tahoma, Verdana, Arial, sans-serif;"
"    }"
"</style>"
"</head>"
"<body>"
"<h1>Welcome to myNGINX!</h1>"
"<p>If you see this page, the nginx web server is successfully installed and "
"working. Further configuration is required.</p>"

"<p>For online documentation and support please refer to"
"<a href=\"http://nginx.org/\">nginx.org</a>.<br/>"
"Commercial support is available at "
"<a href=\"http://nginx.com/\">nginx.com</a>.</p>"

"<p><em>Thank you for using nginx.</em></p>"
"<img src=\"trump.gif\">"
"</body>"
"</html>";
static const char					default_hostname[] = "localhost";

class Server : public IServer
{
private:





	struct sockaddr_in		serv_addr;
	struct hostent			*server;
	std::string				hostname;
	int						sockfd, port, rc, pid, err, status;

	int						process(int sockfd);;
	int						_ncmp(char *buff, char *str);
	int						ft_sendfile(int out_fd, char *filename);
public:
	Server();
	virtual ~Server();
	Server(const Server &ref);
	Server &operator=(const Server &ref);

	IRequest	&getRequest(void);
	int			sendResponce(const IResponse &resp);
	int			acceptConnection();
	int			processConnection();
	int			parentForkPart();
};
