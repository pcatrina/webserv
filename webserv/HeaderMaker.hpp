//
// Created by Skipjack Adolph on 4/4/21.
//

#ifndef WEBSERV_HEADERMAKER_HPP
#define WEBSERV_HEADERMAKER_HPP

#include <Client.hpp>
#include <webserv.hpp>

#include <string>

namespace ft {
	class HeaderMaker {
	public:
		typedef			std::string::size_type strPos;

		static void		headerBuilder(const std::string &text, IHeader *header, Client::req_r_states &state);

		static void		fillHeader(std::string subLine, IHeader *header, Client::req_r_states &states);

		static void		firstLine(std::string const &line, IHeader *header, Client::req_r_states &state);
		static void		fillMethod(const std::string &line, IHeader *header);
		static void		fillUrl(const std::string &line, IHeader *header);
		static void		checkHttp(const std::string &line, IHeader *header);
		static void		readHeader(Client *client, char *buff);
		static void		validateHeader(IHeader *header);
		static bool 	methodNeedsBody(methods_enum method);
	};
}

#endif //WEBSERV_HEADERMAKER_HPP
