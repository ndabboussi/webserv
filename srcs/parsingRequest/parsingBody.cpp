#include "parsingRequest.hpp"

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
		return (void)error400(req);
	char buffer[4096] = {0};
	int bSize = 4096;
	int nb = std::atoi(req.header.find("Content-Length")->second.c_str());
	std::string line;
	if (bSize > nb) 	//Get the body
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
	while ((next = line.find('=', last)) != std::string::npos) 	//Parse the body
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
		req.body.insert(std::make_pair(key, value)); 	// if no = found in the body
	}
	if (next == std::string::npos && last == 0)
		return (void)error400(req);
}


static int createFileAtRightPlace(std::ofstream &Fout, std::string &path, std::string &name, HttpRequest &req)
{
	std::string tmp;
	std::string nPath = path;
	int depth = 0, i = 0;
	size_t pos = 0;
	if (req.isCgi)
	{
		if ((pos = path.find_last_of('/')) != std::string::npos)
		{
			pos++;
			nPath = path.substr(0, pos);
			tmp = path.substr(0, pos) + name;
		}
		else
			tmp = '/' + name;
	}
	else
		tmp = nPath + "/" + name;
	for (size_t j = 0; j < nPath.size(); j++)
	{
		if (nPath[j] && nPath[j] != '/' && i == 0)
		{
			i = 1;
			depth++;
		}
		else if (nPath[j] == '/')
			i = 0;
	}
	if (isAFile(tmp) > 0)
		req.statusCode = 205;
	else
		req.statusCode = 201;
	if (chdir(nPath.c_str()))
		return 1;
	Fout.open(name.c_str(), std::ios::binary);
	while (depth--)
		chdir("..");
	return 0;
}

static int fillFile(HttpRequest &req, std::istringstream &requestStream, std::string &boundary)
{
	std::ofstream Fout;
	if (createFileAtRightPlace(Fout, req.path, *req.fileNames.rbegin(), req) || !Fout.is_open())
		return error500(req);
	std::string pat1 = "\r\n--" + boundary + "--";
	std::string pat2 = "\r\n--" + boundary;
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
			Fout.write(bodyContent.data(), pos);
			size_t dist = bodyContent.size() - pos;
			requestStream.clear();
			requestStream.seekg(-static_cast<std::streamoff>(dist), std::ios::cur);
			break;
		}
		if (requestStream.eof())
			break ;
	}
	if (requestStream.eof())
		return error400(req);
	Fout.close();
	return 0;
}

static void parseType2(HttpRequest &req, std::istringstream &requestStream, std::string &ContentType)
{
	if (ContentType.find("boundary=") == std::string::npos || requestStream.eof())	//checking if the header provides a boundary, if not, bad request
		return (void)error400(req);
	size_t pos = ContentType.find("boundary=") + 9;
	std::string boundary = ContentType.substr(pos, ContentType.size() - pos);
	if (boundary.empty())
		return (void)error400(req);	//if there is no boundary, bad request
	std::string line, key, value;
	int header = 0, body = 0, file = 0;	//In this specific post type, info are separated by boundaries, and each information between have it's own header and body
	while (std::getline(requestStream, line))	//While there is a body to read
	{
		if (line.find('\r') != std::string::npos)
			line.erase(line.find_last_of('\r')); 	//Erase the \r at the end of the line
		if (line == std::string("--" + boundary + "--"))	//If we reach the end boundary, we're done reading the body
			break ;
		if (line == "--" + boundary)	//if we found a boundary, we switch to reading header mode
			header = 1, body = 0;
		else if (line == "")	//if we found an empty line (\r\n), we switch to reading body mode
			header = 0, body = 1;
		else if (header)	//if we're in reading header mode
		{
			if (line.find("Content-Disposition:", 0) != std::string::npos)	//If we found line where there is the variable to store
			{
				size_t first = line.find("name=\"");	//if there is a variable to receive
				if (first == std::string::npos)
					return (void)error400(req);
				first += 6;	//advance pos in line to set it where the name of the variable start
				size_t end = line.find("\"", first);	//error if there is no closing brackets
				if (end == std::string::npos)
					return (void)error400(req);
				key = line.substr(first, end - first);	//store the key
				first = line.find("filename=\"");
				if (first != std::string::npos) 	// if filename is found
				{
					first += 10;
					end = line.find("\"", first);	//error if there is no closing brackets
					if (end == std::string::npos)
						return (void)error400(req);
					std::string name = line.substr(first, end - first);	//store the name of the file
					if (!name.empty())
					{
						file = 1;
						req.fileNames.push_back(name);
					}
				}
			}
			else if (line.find("Content-Type:") != std::string::npos)	//if there is a file store it's content type // not really usefull
				;
			else 	//Header wringly formated (error 400 bad request)
				return (void)error400(req);
		}
		else if (body && !file) //if we're in body mode insert the value according to the key
			req.body.insert(std::make_pair(key, line));
		if (body && file) //if we have a file to upload
		{
			if (fillFile(req, requestStream, boundary)) //if any errors during upload return the error
				return ;
			file = 0;
		}
	}
}

static void parseType3(HttpRequest &req, std::istringstream &requestStream)
{
	std::string			text;
	std::vector<char>	buffer(8192);

	while (1)
	{
		requestStream.read(buffer.data(), buffer.size());
		text.append(buffer.data());
		if (requestStream.eof())
			break ;	
	}
	std::cout << BOLD YELLOW << text << RESET << std::endl;
	req.body.insert(std::make_pair("text", text));
}

int parseBody(HttpRequest &req, std::istringstream &requestStream)
{
	std::string ContentType;
	if (req.header.find("Content-Type") != req.header.end())
		ContentType = req.header.find("Content-Type")->second;
	else if (req.url == "/logout")
		return 0;
	else
		return error400(req);	//error in header
	if (ContentType == "application/x-www-form-urlencoded")
		parseType1(req, requestStream);
	else if (ContentType.find("multipart/form-data") != std::string::npos)
		parseType2(req, requestStream, ContentType);
	else if (ContentType == "text/plain")
		parseType3(req, requestStream);
	else
		return error501(req);	//Content-Type body not supported
	if (req.statusCode)
		return 1;
	return 0;
}