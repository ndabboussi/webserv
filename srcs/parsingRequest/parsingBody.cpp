#include "parsingRequest.hpp"

static void error400(HttpRequest &req)
{
	std::cerr << RED "Error 400: Bad request "<< RESET << std::endl;
	req.error = 400;
}

static void decodeStr(std::string &info)
{
	for (size_t i = 0; i < info.size(); i++)
	{
		if (info[i] == '+')
			info[i] = ' ';
		else if (info[i] == '%' && i + 2 < info.size())
		{
			char hex[3] = {info[i + 1], info[i + 2], '\0'};
			int value;
			std::istringstream(hex) >> std::hex >> value;
			std::string dest;
			dest += static_cast<char>(value);
			info.replace(i, 3, dest);
			i += 2;
		}
	}
}

static void parseType1(HttpRequest &req, std::istringstream &requestStream)
{
	if (req.header.find("Content-Length") == req.header.end())
		return error400(req);
	char buffer[4096] = {0};
	int bSize = 4096;
	int nb = std::atoi(req.header.find("Content-Length")->second.c_str());
	std::string line;
	if (bSize > nb) //Get the body
	{
		requestStream.read(buffer, nb);
		line += buffer;
	}
	else
	{
		int tmpNb = bSize;
		while (1)
		{
			if (nb <= 0)
				break ;
			if (tmpNb > nb)
				requestStream.read(buffer, nb);
			else
				requestStream.read(buffer, tmpNb);
			line += buffer;
			nb -= tmpNb;
		}
	}
	size_t last = 0, next = 0;
	while ((next = line.find('=', last)) != std::string::npos) //Parse the body
	{
		std::string key, value;
		key = line.substr(last, next - last);
		last = next + 1;
		if ((next = line.find('&', last)) != std::string::npos)
		{
			value = line.substr(last, next - last);
			last = next + 1;
		}
		else
			value = line.substr(last, line.size() - last);
		decodeStr(key);
		decodeStr(value);
		req.body.insert(std::make_pair(key, value)); // if no = found in the body
	}
	if (next == std::string::npos && last == 0)
		return error400(req);
}


static int createFileAtRightPlace(std::ofstream &Fout, std::string &path, std::string &name)
{
	int depth = 0, i = 0;
	for (size_t j = 0; j < path.size(); j++)
	{
		if (path[j] && path[j] != '/' && i == 0)
		{
			i = 1;
			depth++;
		}
		else if (path[j] == '/')
			i = 0;
	}
	if (chdir(path.c_str()))
		return 1;
	Fout.open(name.c_str(), std::ios::binary);
	while (depth--)
		chdir("..");
	return 0;
}

static int fillFile(HttpRequest &req, std::istringstream &requestStream, std::string &boundary)
{
	std::ofstream Fout;
	if (createFileAtRightPlace(Fout, req.path, req.body.find("filename")->second) || !Fout.is_open())
	{
		std::cerr << RED "Error 500: Internal server error: "<< RESET << std::endl;
		req.error = 500;
		return 1;
	}
	std::string pat1 = "--" + boundary + "--";
	std::string pat2 = "--" + boundary;
	std::vector<char> buffer(8192);
	std::vector<char> bodyContent;
	while (1)
	{
		requestStream.read(buffer.data(), buffer.size());
		bodyContent.insert(bodyContent.end(), buffer.begin(), buffer.begin() + requestStream.gcount());
		std::vector<char>::iterator it1 = std::search(bodyContent.begin(), bodyContent.end(), pat1.begin(), pat1.end());
		std::vector<char>::iterator it2 = std::search(bodyContent.begin(), bodyContent.end(), pat2.begin(), pat2.end());
		it1 = (it2 != bodyContent.end()) ? it2 : it1;
		if (it1 != bodyContent.end())
		{
			int pos = std::distance(bodyContent.begin(), it1);
			Fout.write(bodyContent.data(), pos - 2);
			size_t dist = bodyContent.size() - pos;
			requestStream.clear();
			requestStream.seekg(-static_cast<std::streamoff>(dist), std::ios::cur);
			break;
		}
		if (requestStream.eof())
			break ;
	}
	if (requestStream.eof())
		return error400(req), 1;
	Fout.close();
	return 0;
}

static void parseType2(HttpRequest &req, std::istringstream &requestStream, std::string &ContentType)
{
	if (ContentType.find("boundary=") == std::string::npos || requestStream.eof())
		return error400(req);
	size_t pos = ContentType.find("boundary=") + 9;
	std::string boundary = ContentType.substr(pos, ContentType.size() - pos);
	if (boundary.empty())
		return error400(req);
	std::string line, key, value;
	int header = 0, body = 0, file = 0;
	while (std::getline(requestStream, line))
	{
		if (line.find('\r') != std::string::npos)
			line.erase(line.find_last_of('\r'));
		if (line == std::string("--" + boundary + "--"))
			break ;
		if (line == "--" + boundary)
			header = 1, body = 0;
		else if (line == "")
			header = 0, body = 1;
		else if (header)
		{
			if (line.find("Content-Disposition:", 0) != std::string::npos)
			{
				size_t first = line.find("name=\"");
				if (first == std::string::npos)
					return error400(req);
				first += 6;
				size_t end = line.find("\"", first + 1);
				if (end == std::string::npos)
					return error400(req);
				key = line.substr(first, end - first);
				first = line.find("filename=\"");
				if (first != std::string::npos) // if filename is found
				{
					first += 10;
					end = line.find("\"", first + 1);
					if (end == std::string::npos)
						return error400(req);
					std::string name = line.substr(first, end - first);
					if (!name.empty())
						req.body.insert(std::make_pair("filename", name));
				}
			}
			else if (line.find("Content-Type:") != std::string::npos)
			{
				size_t first = line.find("Content-Type:") + 14;
				std::string type = line.substr(first, line.size() - first);
				if (!type.empty())
					req.body.insert(std::make_pair("Content-Type", type));
			}
			else
				return error400(req);
		}
		else if (body)
			req.body.insert(std::make_pair(key, line));
		if (body && req.body.find("filename") != req.body.end() && !file)
		{
			if (fillFile(req, requestStream, boundary))
				return ;
			file = 1;
		}
	}
}

static void parseType3(HttpRequest &req, std::istringstream &requestStream)
{
	(void)req, (void)requestStream;
}

int parseBody(HttpRequest &req, std::istringstream &requestStream)
{
	std::string ContentType;
	if (req.header.find("Content-Type") != req.header.end())
		ContentType = req.header.find("Content-Type")->second;
	else
		return error400(req), 1;//error in header
	if (ContentType == "application/x-www-form-urlencoded")
		parseType1(req, requestStream);
	else if (ContentType.find("multipart/form-data") != std::string::npos)
		parseType2(req, requestStream, ContentType);
	else if (ContentType == "application/json")
		parseType3(req, requestStream);
	else
		return error400(req), 1;//Content-Type body not supported
	if (req.error)
		return 1;
	return 0;
}