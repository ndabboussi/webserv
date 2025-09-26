#include "parsingRequest.hpp"

static void parseType1(HttpRequest &req, std::istringstream &requestStream)
{
	(void)req, (void)requestStream;
}


static void createFileAtRightPlace(std::ofstream &Fout, std::string &path, std::string &name)
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
	{
		;//impossible d'aller dans le repertoire
	}
	Fout.open(name.c_str(), std::ios::binary);
	while (depth--)
		chdir("..");
}

static int fillFile(HttpRequest &req, std::istringstream &requestStream, std::string &line, std::string boundary)
{
	std::ofstream Fout;
	createFileAtRightPlace(Fout, req.path, req.body.find("filename")->second);
	if (!Fout.is_open())
	{
		//error dest file couldnt be oppened
		return 1;
	}
	line += '\n';
	Fout << line;
	while (std::getline(requestStream, line))
	{
		if (line == std::string("--" + boundary + "--") || line == std::string("--" + boundary))
			break ;
		line += "/n";
		Fout << line;
	}
	if (requestStream.eof())
		;//error bad request 400
	Fout.close();
	return 0;
}

static void parseType2(HttpRequest &req, std::istringstream &requestStream, std::string ContentType)
{
	if (!ContentType.find("boundary="))
		return ;//error in header

	size_t pos = ContentType.find("boundary=") + 9;
	std::string boundary = ContentType.substr(pos, ContentType.size() - pos);
	std::string line, key, value;

	int header = 0, body = 0;
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
			{
				if (fillFile(req, requestStream, line, boundary))
					return ;//error bad request 400
				if (line == std::string("--" + boundary + "--"))
					break ;
				if (line == "--" + boundary)
					header = 1, body = 0;
			}
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