#include "webserv.hpp"

ServerSocket::ServerSocket() : max_socket(0) {}

ServerSocket::ServerSocket(const ServerSocket &copy)
{
	*this = copy;
}

ServerSocket &ServerSocket::operator=(const ServerSocket &copy)
{
	for (size_t i = 0; i < server_fds.size(); i++)
		server_fds[i] = copy.server_fds[i];
	max_socket = copy.max_socket;
	server_addr = copy.server_addr;
	active_sockets = copy.active_sockets;
	read_sockets = copy.read_sockets;
	write_sockets = copy.write_sockets;

	return (*this);
}

ServerSocket::~ServerSocket()
{
	delete[] server;
	delete[] readBuffer;
	std::cout << "Memory was freed" << std::endl;
}

/*
This function calls parsing functions and initalises server sockets.
*/
void ServerSocket::Init(const std::string &configFile)
{
	std::ifstream file(configFile);
	if (!file.is_open())
	{
		std::cerr << "Error opening configuration file" << std::endl;
		exit(1);
	}
	if (file.peek() == EOF)
	{
		std::cerr << "Error: configuration file empty" << std::endl;
		exit(1);
	}
	std::string line;
	servSize = 0;
	while (std::getline(file, line))
	{
		if (line.find("server") != std::string::npos && line.find("{") != std::string::npos)
			servSize++;
	}
	file.close();
	if (servSize < 1)
	{
		std::cerr << "Configuration file needs at least one Server" << std::endl;
		exit(1);
	}
	try{
		this->server = new servers[servSize];
	}
	catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
		exit(1);
    }

	readConfigFile(configFile);

	servPortsCount = 0;
	for (int j = 0; j < servSize; ++j)
	{
		if (server[j].getPorts().size() == 0)
		{
			std::cerr << "Error: no ports found" << std::endl;
			exit(1);
		}
	}

	FD_ZERO(&active_sockets);
	for (int j = 0; j < servSize; ++j)
	{
		for (size_t i = 0; i < server[j].getPorts().size(); ++i)
		{
			int server_fd = socket(AF_INET, SOCK_STREAM, 0);
			if (server_fd < 0)
			{
				perror("In socket");
				exit(EXIT_FAILURE);
			}

			int opt = 1;
			if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
			{
				perror("In setsockopt");
				exit(EXIT_FAILURE);
			}

			fcntl(server_fd, F_SETFL, O_NONBLOCK | FD_CLOEXEC);

			struct sockaddr_in server_addr;
			memset(&server_addr, 0, sizeof(server_addr));
			server_addr.sin_family = AF_INET;
			server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
			server_addr.sin_port = htons(server[j].getPorts()[i]);

			if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
			{
				perror("In bind");
				close(server_fd);
				exit(EXIT_FAILURE);
			}

			if (listen(server_fd, MAX_CLIENTS) < 0)
			{
				perror("In listen");
				close(server_fd);
				exit(EXIT_FAILURE);
			}

			FD_SET(server_fd, &active_sockets);
			server_fds.push_back(server_fd);
			max_socket = std::max(max_socket, server_fd);
			servPortsCount++;
		}
	}

	std::cout << "----Awaiting connections on port(s): ";
	for (int j = 0; j < servSize; ++j)
	{
		for (size_t i = 0; i < server[j].getPorts().size(); ++i)
			std::cout << server[j].getPorts()[i] << " ";
	}
	std::cout << "----" << std::endl << std::endl;
	if (!(MAX_CLIENTS > 0 && MAX_CLIENTS < 1025))
	{
		std::cerr << "Please enter between 1 and 1024 clients" << std::endl;
		exit(1);
	}
	try {
		readBuffer = new std::string[MAX_CLIENTS + 2];
	}
	catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
		exit(1);
    }
	bool end = false;
	while (1)
		Loop(end);
}

/*
This mini-function checks whether socket_ID is a server or not.
*/
bool ServerSocket::_check(int socket_ID)
{
	std::vector<int>::iterator it;
	for (it = server_fds.begin(); it != server_fds.end(); ++it)
	{
		if (socket_ID == *it)
			return true;
	}
	return false;
}

/*
Non-blocking function to receive requests from the clients.
*/
int ServerSocket::_receive(int socket_ID)
{
	int socket_ID_tmp = socket_ID - 3 - servPortsCount; 
	if (socket_ID_tmp > MAX_CLIENTS + 1)
		return 0;
	int bytesRead;
	int trigger = 0;
	char tmpBuffer[100];
	usleep(50);
	memset(tmpBuffer, 0, sizeof(tmpBuffer));
	bytesRead = recv(socket_ID, tmpBuffer, sizeof(tmpBuffer), MSG_PEEK);
	memset(tmpBuffer, 0, sizeof(tmpBuffer));
	if (bytesRead <= 99)
		trigger =  1;
	bytesRead = recv(socket_ID, tmpBuffer, sizeof(tmpBuffer) - 1, MSG_DONTWAIT);
	if (bytesRead > 0)
	{
		tmpBuffer[bytesRead] = '\0';
		readBuffer[socket_ID_tmp] += std::string(tmpBuffer, bytesRead);
		if (trigger == 1)
		 	return -2;
	}
	else if (bytesRead == 0)
	{
		std::cout << "Connection was closed" << std::endl;
		return 0;
	}
	else
		return -1;
	return bytesRead;
}

/*
This function builds a response and sends it back to the client.
*/
int ServerSocket::_respond(int socket_ID)
{
	int socket_ID_tmp = socket_ID - 3 - servPortsCount; 
    // std::cout << "buffer " << readBuffer[socket_ID_tmp] << std::endl; // DEBUG PRINT
	if (socket_ID_tmp > MAX_CLIENTS + 1)
		return -1;
	if (readBuffer[socket_ID_tmp].length() == 0)
		return -1;
	serverName = getLastPart(readBuffer[socket_ID_tmp], "Host: ");
    int ret;
    std::string response;
    std::string host;
    size_t pos = readBuffer[socket_ID_tmp].find("Host: ");
    if (pos != std::string::npos)
    {
        pos += 6;
        while (readBuffer[socket_ID_tmp][pos] != ':')
            pos++;
        pos++;
        int limit = pos;
        while (readBuffer[socket_ID_tmp][limit] != '\r')
            limit++;
        limit -= pos;
        host = readBuffer[socket_ID_tmp].substr(pos, limit);
    }
    if (host.empty())
        return -1;
    for (int j = 0; j < servSize; ++j)
    {
        for (size_t i = 0; i < server[j].getPorts().size(); ++i)
        {
            if (server[j].getPorts()[i] == stod(host))
            {
                currentServ = server[j];
                break;
            }
        }
    }
	currentSocket = socket_ID;
    if (readBuffer[socket_ID_tmp].length() == 0)
        response = callErrorFiles(400);
    else
        response = handleHttpRequest(readBuffer[socket_ID_tmp]);
    ret = send(socket_ID, response.c_str(), response.size(), 0);
	readBuffer[socket_ID_tmp].clear();
    return ret;
}


/*
This is the core of the program, this is where we select, accept,
and either receive requests from the client or respond to them.
*/
void ServerSocket::Loop(bool end)
{
	int ret, ready = 0, new_sd;
	bool close_connection;

	read_sockets = active_sockets;
	FD_ZERO(&active_write);
	while (!end)
	{
		struct timeval tv;
		tv.tv_sec = 3;
		tv.tv_usec = 0;
		FD_ZERO(&read_sockets);
		FD_ZERO(&write_sockets);
		read_sockets = active_sockets;
		write_sockets = active_write;
		ret = select(max_socket + 1, &read_sockets, &write_sockets, NULL, &tv);
		if (ret == -1)
		{
			std::cerr << "Error in select(): " << strerror(errno) << std::endl;
			return;
		}
		if (ret == 0)
			std::cout << "IN TIMEOUT" << std::endl;
		ready = ret;
		std::cout << std::endl
				  << "==== WAITING ====" << std::endl;
		for (int socket_ID = 0; socket_ID <= max_socket; socket_ID++)
		{
			if (FD_ISSET(socket_ID, &read_sockets) || FD_ISSET(socket_ID, &write_sockets))
			{
				if (_check(socket_ID))
				{
					new_sd = accept(socket_ID, NULL, NULL);
					if (new_sd < 0)
					{
						if (errno != EWOULDBLOCK)
						{
							std::cerr << "Error accepting client connection" << std::endl;
							end = true;
						}
						break;
					}
					fcntl(new_sd, F_SETFL, O_NONBLOCK | FD_CLOEXEC);
					FD_SET(new_sd, &active_sockets);
					max_socket = (new_sd > max_socket) ? new_sd : max_socket;
				}
				else
				{
					close_connection = false;
					if (FD_ISSET(socket_ID, &read_sockets))
					{
						ret = _receive(socket_ID);
						if (ret == 0)
						{
							std::cout << "Connection with client " << socket_ID << " was closed" << std::endl;
							FD_CLR(socket_ID, &active_sockets);
							close(socket_ID);
						}
						else if (ret == -1)
						{
							std::cerr << "Error in receive for client " << socket_ID << std::endl;
							FD_CLR(socket_ID, &active_sockets);
							close(socket_ID);
						}
						else if (ret == -2)
						{
							FD_CLR(socket_ID, &active_sockets);
							FD_SET(socket_ID, &active_write);
						}
					}
					else if (FD_ISSET(socket_ID, &write_sockets))
					{
						ret = _respond(socket_ID);
						if (ret == -1)
							std::cerr << "Send() error" << std::endl;
						else if (ret == 0)
							std::cerr << "No data was sent" << std::endl;
						close_connection = true;
						usleep(1700);
					}
					if (close_connection)
					{
						close(socket_ID);
						FD_CLR(socket_ID, &active_write);
						FD_CLR(socket_ID, &active_sockets);
						for (int i = 0; i < MAX_CLIENTS + 6; i++)
						{
							if (FD_ISSET(i, &active_sockets))
								max_socket = i;
							if (FD_ISSET(i, &active_write))
								max_socket = i;
						}
					}
				}
			}
		}
	}
}

/*
The main function launches the program with the correct configuration file.
*/
int main(int argc, char **argv)
{
	if (argc == 2)
	{
		std::ifstream file(argv[1]);
		if (file.is_open())
		{
			file.close();
			ServerSocket ss;
			ss.Init(argv[1]);
		}
		else
			std::cerr << "Error: Unexistant configuration file" << std::endl;
		file.close();
	}
	else if (argc == 1)
	{
		ServerSocket ss;
		ss.Init("tools/conf/config.txt");
	}
	else
		std::cout << "Wrong number of arguments" << std::endl;
	return (EXIT_FAILURE);
}
