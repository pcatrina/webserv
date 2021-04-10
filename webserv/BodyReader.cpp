//
// Created by Gerry Shona on 4/3/21.
//

#include <BodyReader.hpp>
#include <iostream>

ft::BodyReader::BodyReader()
{/* Illegal */}


unsigned int ft::BodyReader::_max_id;

ft::BodyReader::BodyReader(int input_fd, int content_length, std::string rem) :
		_remainder_of_header(rem), _last_readed(), _block_size_i(0),
		_read_buff(0),
		_output_fd(-1),
		_ended(false),
		_written_size(0),
		_input_fd(input_fd), _offset(0), _content_length(content_length)
{

	if (!rem.empty())
		_state = s_remains;
	else
		_state = s_len;
	//_filename = ft::to_string(static_cast<int>(_max_id));
	openFile();
	_max_id++;
}

ft::BodyReader::~BodyReader()
{}

void
ft::BodyReader::openFile() {
	char 		dir[MAX_PATH_LEN];
	char 		*_ret = 0;
	int ret;
	_ret = getcwd(dir, MAX_PATH_LEN);
	if (!_ret)
		throw ft::runtime_error("Too long path to executable");
	_filename = std::string(dir) + "/" + std::string(TMP_DIR) + "/" + ft::to_string(static_cast<int>(_max_id));
	ret = open(_filename.c_str(), O_TRUNC | O_WRONLY | O_CREAT, 0666);
		if( ret == -1)
			throw ft::runtime_error("BODY READER: CANT OPEN FILE: " +std::string(strerror(errno)));
		_output_fd = ret;
}

int ft::BodyReader::write_block(const char *buff, int len, int offset) {
	int ret = write(_output_fd, buff + offset + _offset, len - offset);
	if (ret != -1)
		_written_size += ret;
	else
		return - 1;
	if (ret != len - offset - _offset)
		throw ft::runtime_error("Error in writing block:" + std::string(strerror(errno)));
	if (_offset == 1)
		_offset = 0;
}


int ft::BodyReader::get_resultFd() const {
	if (!_ended)
		throw ft::runtime_error("Body was not fully readed\n");
	return _output_fd;
}

unsigned int ft::BodyReader::getSize() const {
	return  _written_size;
}

int ft::BodyReader::endReading(int ret) {
	if (!_ended)
	{
		_ended = true;
		ft_close(_output_fd);
	}
	return ret;
}

int ft::BodyReader::readBody() {
	if (_content_length != LEN_CHUNKED)
		return (readByLen());
	int ret;
	if (_state == s_remains) {
		ret = readRem();
		if (ret != 1)
			return endReading(ret);
		else
			return 1;
	}
	switch(_state) {
		case s_p_block: return(readPBlock()); break;
		case s_pp_block: return readPPBlock();
		case s_r_ending: return(readEnding()); break;
		case s_len: return (readChunkLen(1)); break;
		case s_a1_len: return (readChunkLen(2)); break;
		case s_a_len: return (readChunkLen(3)); break;
		case s_block: return(readChunk()); break;
		case s_end: return 0; break;
		case s_remains: throw ft::runtime_error("WE SHOULDN\'N BE HERE\n"); break;
	}
	return 0;
}

int ft::BodyReader::readRem()
{
	size_t pos ;
	while (!_remainder_of_header.empty())
	{
		pos = _remainder_of_header.find("\r\n");
		//_block_size = _remainder_of_header;
		if (pos == std::string::npos)
		{                // В остатке нет длины блока
			_state = s_len;
			pos = _remainder_of_header.find('\r');
			if (pos != std::string::npos) {
				_offset = 0;
				_block_size = _remainder_of_header.substr(0, pos);
				if (_block_size == "0")
					_state = s_r_ending;
				else {
					_state = s_block;
					_block_size_i = strtol(_block_size.c_str(), NULL, 16);
					_block_size.clear();
				}
				return 1;
			}
			return 1;
		}
		else
		{
			_block_size = _remainder_of_header.substr(0, pos);
			if (_block_size == "0")
			{
				_state = s_end;
				return (0);
			}
			_block_size_i = static_cast<int>(strtol(_block_size.c_str(), NULL, 16)); //         !!!!!!!!!!!
			// ATTENTION
			// !!!!!!!!!
			_remainder_of_header.erase(0, pos + 2);
			if ((size_t)_block_size_i >= _remainder_of_header.size())
			{
				if ((size_t)_block_size_i == _remainder_of_header.size())
					_state = s_a_len;
				else
					_state = s_p_block;
				write_block(_remainder_of_header.c_str(), _remainder_of_header.size());

				_block_size.clear();
				return 1;
			}
			else
			{
				//pos = _remainder_of_header.find("\r\n");

				//if (pos == std::string::npos)
				//	throw ft::runtime_error("AAAAAAAAAAAAAAAAAAAA");
				_state = s_block;
				try {
					write_block(_remainder_of_header.c_str(), _block_size_i);
				}
				catch (ft::runtime_error){return  -1;}
				if (_remainder_of_header.size() == (size_t)_block_size_i + 1)
				{
					_state = s_a1_len;
					_block_size.clear();
					return 1;
				}
				_remainder_of_header.erase(0, _block_size_i + 2);
			}
		}
	}
	if (_state == s_len)
	{
		_block_size_i = static_cast<int>(strtol(_block_size.c_str(), NULL, 16));
		_offset = 1;
		_state = s_block;
	}
	else if (_state == s_block)
	{
		_block_size.clear();
		_state = s_len;
	}
	return 1;
}

int ft::BodyReader::readChunkLen(int n)
{
	char buff[3];
	int ret;
	ret = read(_input_fd, buff, n);
	//std::cout << "READED: "<< n << " [ "<< _input_fd << "]\n";

	if (ret == -1)
		return endReading(-2);
	_last_readed = buff[n -1];
	if (std::string("ABCDEFabcdef0123456789\r").find(_last_readed) == std::string::npos)
		throw ft::runtime_error("WRONG STATE IN BODY READER");
	if (_last_readed == '\r')
	{
		_state = s_block;
		if (_block_size == "0")
		{
			_state = s_r_ending;
			return (1);
		}
		if (_block_size.empty())
			return -1;
		_block_size_i = static_cast<int>(strtol(_block_size.c_str(), NULL, 16));  // !!!!!!!!!!!! ATTENTION !!!!!!!!!!!!!!!!!!!
		_block_size.clear();
		return 1;
	}else
	{
		_block_size += _last_readed;
		_state = s_len;
		return 1;
	}
}

int ft::BodyReader::readChunk()
{
	try {
	if (readWriteBlock(_block_size_i + 3, 1) == -1)
		return (-1);
	}catch (ft::runtime_error){return -1;}
	if (_state != s_pp_block)
		_state = s_len;
	return (1);
}

int ft::BodyReader::readWriteBlock(int size, int offset) {
	int ret;

	_read_buff = (char*)malloc(size + _offset);
	if (!_read_buff)
		throw ft::runtime_error("Malloc failed");
	ret = read(_input_fd, _read_buff, size + _offset);
	_last_readed_bytes = ret;
	if (ret == -1)
		return endReading(-2);
	if (ret != size) {
		//free(_read_buff);
		//return endReading(-1);
		_state = s_pp_block;
		_last_readed_bytes = ret;

		return 1;
	}
	if (offset == 1 && _read_buff[0 + _offset] != '\n')
	{
		free(_read_buff);
		return endReading(-1);
	}
	if (_state != s_pp_block && _read_buff[size -1 + _offset] == '\n' && _read_buff[size -2 + _offset] == '\r')
		size -= 2;
	//else
	//	return endReading(-1);
	write_block(_read_buff, size, offset);
	free(_read_buff);
	_read_buff = nullptr;
	return 1;
}

int ft::BodyReader::readPPBlock() {
	int size = _block_size_i - _last_readed_bytes + 3;
	int ret = readWriteBlock(size);
	if (_last_readed_bytes == size)
		_state = s_len;
	return ret;
}
int ft::BodyReader::readPBlock() {
	int ret;
	_state = s_len;
	ret = readWriteBlock( _block_size_i - _remainder_of_header.size() + 2);
	_remainder_of_header.clear();
	return ret;
}

int ft::BodyReader::readEnding() {
	char buff[4];
	int ret;
	ret = read(_input_fd, buff, 3);
	if(ret == -1)
		return endReading(-2);
	buff[3] =0;
	if (ret == -1 || ft::ft_strcmp(buff,"\n\r\n"))
		ret = -1;
	else ret = 0;
	return endReading(ret);
}

const std::string &ft::BodyReader::getFilename() const {
	return _filename;
}

void ft::BodyReader::reset() {
	_max_id = 0;
}

ft::IBody *ft::BodyReader::getBody() {
	if (!_ended)
		throw ft::runtime_error("Body was not fully readed\n");
	return new FileBody(_filename, 0);
}

int ft::BodyReader::readByLen()
{
	int ret;
	_read_buff = (char*)malloc(_content_length);
	if (!_read_buff)
		throw ft::runtime_error("Malloc failed");
	ret = read(_input_fd, _read_buff, _content_length);
	if (ret == -1)
		return endReading(-2);
		//throw ft::runtime_error("READ FAILED");
	if (ret != _content_length) {
		free(_read_buff);
		_read_buff = 0;
		return endReading(-1);
	}
	ret = write(_output_fd, _read_buff, _content_length);
	if (ret == -1)
		throw ft::runtime_error("WRITE FAILED");
	return (endReading(0));
}

unsigned int ft::BodyReader::getMaxId() {
	return _max_id;
}


