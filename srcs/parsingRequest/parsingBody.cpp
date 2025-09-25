#include "parsingRequest.hpp"

static void parseType1(HttpRequest &req, std::istringstream &requestStream)
{
	(void)req, (void)requestStream;
}

static void parseType2(HttpRequest &req, std::istringstream &requestStream, std::string ContentType)
{
	if (!ContentType.find("boundary="))
		return ;//error in header

	size_t pos = ContentType.find("boundary=") + 9;
	std::string boundary = ContentType.substr(pos, ContentType.size() - pos);
	std::string line, key, value;

	int header = 0, body = 0, file = 0;
	(void)file;
	while (std::getline(requestStream, line))
	{
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
					return ;//bad request 400
				first += 6;
				size_t end = line.find("\"", first + 1);
				if (end == std::string::npos)
					return ;//bad request 400
				key = line.substr(first, end - first);
				first = line.find("filename=\"");
				if (first != std::string::npos) // if filename is found
				{
					first += 10;
					end = line.find("\"", first + 1);
					if (end == std::string::npos)
						return ;//bad request 400
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
				return ;//bad request 400
		}
		else if (body)
		{
			if (req.body.find("filename") != req.body.end())
				;//fill the file
			else
				req.body.insert(std::make_pair(key, line));
		}
	}
	for (std::map<std::string, std::string>::const_iterator it = req.body.begin(); it != req.body.end(); it++)
		std::cout << "key = " << it->first << ", value = " << it->second << std::endl;
}

static void parseType3(HttpRequest &req, std::istringstream &requestStream)
{
	(void)req, (void)requestStream;
}

int parseBody(HttpRequest &req, std::istringstream &requestStream)
{
	std::string ContentType;
	(void)requestStream;
	if (req.header.find("Content-Type") != req.header.end())
		ContentType = req.header.find("Content-Type")->second;
	else
		return 1;//error in header
	if (ContentType == "application/x-www-form-urlencoded")
		parseType1(req, requestStream);
	else if (ContentType.find("multipart/form-data"))
		parseType2(req, requestStream, ContentType);
	else if (ContentType == "application/json")
		parseType3(req, requestStream);
	else
		return 1;//Content-Type body not supported

	return 0;
}