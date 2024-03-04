#include "webserv.hpp"

std::string  ServerSocket::getLastPart(const std::string &str, const std::string &cut)
{
    size_t start = str.find(cut) + cut.length();
	size_t end = start - 1;
	while (str[++end] != '\r');

	return (str.substr(start, end));
}

void mfree(char **f)
{
    int i = 0;

    while(f[i] != 0)
        free (f[i++]);
	free (f);
}

std::string getPathInfo(std::string path)
{
	if (path.rfind(".") != std::string::npos && path.rfind(".") < path.rfind("/"))
		return path.substr(path.rfind("/"), path.rfind(".") - path.rfind("/") - 1);

	return "";
}

std::string getQueryString(std::string path)
{
	if (path.rfind("?") != std::string::npos && path.rfind("?") < path.length() - 1)
		return path.substr(path.rfind("?") + 1, path.length() - 1);

	return "";
}

bool checkValue(const std::string value)
{
	try{
		if (value.find_first_not_of("0123456789") != std::string::npos)
		{
			std::cerr << "Error: wrong characters in config detected" << std::endl;
			exit(1);
		}
		stod(value);
		return true;
	}
	catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
		exit(1);
    }
	return false;
}

void    ServerSocket::checkFdSets()
{
    if (FD_ISSET(currentSocket, &read_sockets))
        FD_CLR(currentSocket, &active_sockets);
    if (FD_ISSET(currentSocket, &write_sockets))
        FD_CLR(currentSocket, &active_write);
	close(currentSocket);
}