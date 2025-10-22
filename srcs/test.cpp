//Après select(), on lit les pipes CGI disponibles :

for (size_t i = 0; i < clients.size(); i++)
{
    if (clients[i].isCgiRunning())
    {
        int cgiFd = clients[i].getCgiOutputFd();
        if (cgiFd > 0 && FD_ISSET(cgiFd, &readfds))
        {
            char buf[4096];
            ssize_t n = read(cgiFd, buf, sizeof(buf));
            if (n > 0)
                clients[i].setCgiBuffer(clients[i].getCgiBuffer() + std::string(buf, n));
            else if (n == 0)
            {
                // CGI terminé
                close(cgiFd);
                clients[i].setCgiOutputFd(-1);

                int status = 0;
                waitpid(clients[i].getCgiPid(), &status, WNOHANG);
                clients[i].setCgiRunning(false);
            }
        }
    }
}



//Si CGI n’a pas encore fini, on retourne true (rien à envoyer encore).
//Quand _cgiRunning == false et _cgiBuffer non vide, on parse et on envoie.
bool Response::cgiResponse(Client &client, Context &context)
{
    if (!this->_request.isCgi)
        return false;

    // Cas 1: CGI pas encore lancé
    if (!client.isCgiRunning() && client.getCgiOutputFd() <= 0)
    {
        CGI cgi(this->_request, client);
        cgi.setCgiInfos(this->_request, this->_server);
        cgi.executeCgi(this->_request, this->_server, this->_clientFd, context);
        return true;
    }

    // Cas 2: CGI lancé mais pas terminé
    if (client.isCgiRunning())
        return true;

    // Cas 3: CGI terminé → on parse et envoie
    std::string rawOutput = client.getCgiBuffer();
    int cgiStatus = 200;
    std::map<std::string, std::string> headers;
    CGI parser(this->_request, client);
    std::string body = parser._parseCgiOutput(rawOutput, cgiStatus, headers);

    this->_code = cgiStatus;
    this->setStatusLine();
    for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
        this->setHeader(it->first, it->second);

    if (headers.find("Content-Length") == headers.end())
        this->setHeader("Content-Length", toString(body.size()));

    this->setBody(body, headers.count("Content-Type") ? headers["Content-Type"] : "text/html");
    this->sendTo();

    client.setCgiBuffer("");
    return true;
}
