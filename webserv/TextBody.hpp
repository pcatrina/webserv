#pragma once
#include <IBody.hpp>
namespace ft
{
	class TextBody : public IBody
	{
	private:
		TextBody();
		std::string	_text;
	public:
		TextBody(std::string const & text);
		virtual ~TextBody();
		TextBody(const TextBody &ref);
		TextBody &operator=(const TextBody &ref);

		bool				getFd(void);
		std::string			to_string(void) const;
		unsigned int		size(void) const;
		BodyType			getType(void);
	};

}