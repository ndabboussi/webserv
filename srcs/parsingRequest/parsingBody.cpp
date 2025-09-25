#include "parsingRequest.hpp"

static void parseType1(HttpRequest &req, std::istringstream &requestStream)
{
	(void)req, (void)requestStream;
}

static void parseType2(HttpRequest &req, std::istringstream &requestStream, std::string ContentType)
{
	(void)req, (void)requestStream;

	if (!ContentType.find("boundary="))
		return ;//error in header

	size_t pos = ContentType.find("boundary=") + 9;
	std::string boundary = ContentType.substr(pos, ContentType.size() - pos);
	std::string line;

	int header = 0, body = 0;
	while (std::getline(requestStream, line))
	{
		if (line != "--" + boundary + "--")
			break ;
		if (line == "--" + boundary)
			header = 1, body = 0;
		else if (line == "\r")
			header = 0, body = 1;
		else if (header)
		{
			if (!line.find("Content-Disposition") || !line.find("Content-Type"))
				return ;//bad request 400
		}
		else if (body)
		{

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