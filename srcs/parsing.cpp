#include "webserv.hpp"

int	ServerSocket::checkPerms(const std::string &buffer)
{
	std::map<std::string, std::string>::iterator it;
	std::istringstream request(buffer);
	std::string method, path, line, path_cpy;
	request >> method >> path;
	it = currentServ.getServConf("web_root");
	if (it != currentServ.getConfEnd())
	{
		path_cpy = it->second + path_cpy;
		struct stat s;
		if (stat(path_cpy.c_str(), &s) == 0)
		{
			path_cpy = path;
			if(s.st_mode & S_IFDIR)
			{
				if (path_cpy[path_cpy.length() - 1] != '/')
					path_cpy.append("/");
			}
		}
		else
			path_cpy = path;
	}

	int trigger = 0;
	int trigger2 = 0;
	while (trigger2 == 0)
	{
		for (int i = 0; i < currentServ.getLocationSize() && trigger == 0; i++)
		{
			std::map<std::string, std::string>::iterator it = currentServ.getServLocation(i, "location");
			if (!((it->second.substr(0, it->second.rfind("/")) + "/").compare(path_cpy.substr(0, path_cpy.rfind("/")) + "/")))
			{
				trigger2 = 1;
				it = currentServ.getServLocation(i, "deny");
				if (it != currentServ.getLocationEnd(i))
				{
					if (it->second == "all")
					{
						it = currentServ.getServLocation(i, "return");
						if (it != currentServ.getLocationEnd(i))
							return (stod(it->second));
					}
				}
				for (it = currentServ.getLocationBegin(i); it != currentServ.getLocationEnd(i); it++)
				{
					if (it->second == "allow" && it->first == method)
					{
						trigger = 1;
						break ;
					}
				}
			}
		}
		if (path_cpy.length() == 1)
			break;
		if (trigger2 == 0)
		{
			int pos = path_cpy.length();
			pos--;
			while (path_cpy[pos] != '/')
				pos--;
			path_cpy = path_cpy.substr(0, pos);
		}
	}
	return (trigger);
}

std::map<int, std::string> ServerSocket::parseFileInfo(std::string path)
{
	struct stat s;
	std::string path_cpy;
	std::string tmp;
	std::map<int, std::string> response;
	std::map<std::string, std::string>::iterator it;
	int trigger = 0;

	for (int k = 0; k < currentServ.getLocationSize(); k++)
	{
	 	it = currentServ.getServLocation(k, "location");
		if (it != currentServ.getLocationEnd(k))
		{
			if (it->second == path)
			{
				it = currentServ.getServLocation(k, "redirect");
				if (it != currentServ.getLocationEnd(k))
				{
					response[1] = "HTTP/1.1 302 Found\r\nLocation: " + it->second + "\r\n\r\n";
					return (response);
				}
			}
		}
	}
	path_cpy = path;
	if (path_cpy[path_cpy.length() - 1] != '/')
		path_cpy.append("/");
	it = currentServ.getServConf("web_root");
	if (it != currentServ.getConfEnd())
		path = it->second + path;
	if (path_cpy.length() == 1)
	{
		for (int i = 0; i < currentServ.getLocationSize(); i++)
		{
			std::map<std::string, std::string>::iterator it = currentServ.getServLocation(i, "location");
			if (it != currentServ.getLocationEnd(i))
			{
				if (it->second == "/")
				{
					std::map<std::string, std::string>::iterator it = currentServ.getServLocation(i, "default_file");
					if (it != currentServ.getLocationEnd(i))
					{
						path = path + it->second;
						trigger = 1;
					}
				}
			}
		}
	}
	if (stat(path.c_str(), &s) == 0 && trigger == 0)
	{
		if(s.st_mode & S_IFDIR)
		{
			int path_cpy_len = path_cpy.length();
			while (trigger == 0)
			{
				for (int i = 0; i < currentServ.getLocationSize(); i++)
				{
					it = currentServ.getServLocation(i, "location");
					if (it != currentServ.getLocationEnd(i))
					{
						if (it->second == path_cpy)
						{
							it = currentServ.getServLocation(i, "autoindex");
							if (it != currentServ.getLocationEnd(i))
							{
								trigger = 2;
								if (it->second == "on")
								{
									std::map<std::string, std::string>::const_iterator it = currentServ.getServConf("index");
									if (it != currentServ.getConfEnd())
									{
										if (path_cpy_len == 1)
											path = path + it->second;
										else
											path = path + "/" + it->second;		
									}
									else
									{
										if (path_cpy_len == 1)
											path = path + "index.html";
										else
											path = path + "/index.html";
									}
									trigger = 1;
								}
							}
						}
					}
				}
				if (path_cpy.length() == 1)
					break;
				if (trigger == 0)
				{
					int pos = path_cpy.length() - 1;
					if (path_cpy[pos] == '/')
						pos--;
					while (path_cpy[pos] != '/')
						pos--;
					path_cpy = path_cpy.substr(0, pos + 1);
				}
			}
		}
		else if( s.st_mode & S_IFREG )
			trigger = 1;
	}
	else
		trigger = 1;
	if (trigger == 0 || trigger == 2)
	{
		response[1] = callErrorFiles(403);
		return (response);
	}
	FILE * fin;
	fin = fopen(path.c_str(), "rb");
	if (fin == NULL)
		response[-1] = "";
	else
		response[0] = path;
	fclose(fin);
	return(response);
}

void ServerSocket::parseLocation(const std::vector<std::string> &tmpLine, int index, int ind_serv)
{
	(void) index;
	std::map<std::string, std::string> tmp;
	for (size_t i = 0; i < tmpLine.size(); i++)
	{
		std::string toRead = tmpLine[i];
		std::istringstream iss(toRead);
		std::string key, value;
		if (iss >> key)
		{
			if (iss >> value)
			{
				if (key == "return")
				{
					if (!checkValue(value))
					{
						std::cerr << "Error: wrong config format - code error" << std::endl;
						exit(1);
					}
				}
				if (key == "}")
					break;
				if (key == "allow")
					tmp[value] = key;
				else
					tmp[key] = value;
			}
			else
			{
				std::cerr << "Error in location conf" << std::endl;
				exit(1);
			}
			if (iss >> value && i != 0)
			{
				std::cerr << "Error: too many arguments in one line in conf" << std::endl;
				exit(1);
			}
		}
		else
		{
			std::cerr << "Error in location conf" << std::endl;
			exit(1);
		}
	}
	server[ind_serv].setServLocation(tmp);
}

void ServerSocket::readConfigFile(const std::string &configFile)
{
	int index = 0;
	int ind_serv = -1;
	servers tmp;
	int inside = 0;
	int trigger;
	int bracket_counter = 0;
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
	while (std::getline(file, line))
	{
		std::istringstream iss(line);
		std::string key, value;
		if (line.find("server") == 0)
		{
			if (inside == 1)
			{
				std::cerr << "Error: wrong config format" << std::endl;
				exit(1);
			}
			inside = 1;
			ind_serv++;
			if (line.find("{") != std::string::npos)
				bracket_counter++;
			else
			{
				std::cerr << "Error: wrong config format" << std::endl;
				exit(1);
			}
			continue;
		}
		if (iss >> key)
		{
			if (key == "}")
			{
				inside = 0;
			}
			else if (key == "listen")
			{
				if (inside == 0)
				{
					std::cerr << "Error: wrong config format" << std::endl;
					exit(1);
				}
				if (iss >> value)
				{
					if (checkValue(value))
					{
						if (stod(value) >= 1024 && stod(value) <= 65535)
							server[ind_serv].setPorts(stod(value));
						else
						{
							std::cerr << "Error in ports config" << std::endl;
							exit(1);
						}
					}
				}
				else
				{
					std::cerr << "Error in ports config" << std::endl;
					exit(1);
				}
				if (iss >> value)
				{
					std::cerr << "Error: too many arguments in one line in conf" << std::endl;
					exit(1);
				}
			}
			else if (key == "error_page")
			{
				if (inside == 0)
				{
					std::cerr << "Error: wrong config format" << std::endl;
					exit(1);
				}
				if (iss >> key)
				{
					if (iss >> value)
					{
						if (!checkValue(key))
						{
							std::cerr << "Error: wrong config format" << std::endl;
							exit(1);
						}
						server[ind_serv].setServError(key, value);
					}
					else
					{
						std::cerr << "Error: wrong error_page format" << std::endl;
						exit(1);
					}
				}
				else
				{
					std::cerr << "Error: wrong error_page format" << std::endl;
					exit(1);
				}
				if (iss >> value)
				{
					std::cerr << "Error: too many arguments in one line in conf" << std::endl;
					exit(1);
				}
			}
			else if (key == "location")
			{
				if (inside == 0)
				{
					std::cerr << "Error: wrong config format" << std::endl;
					exit(1);
				}
				trigger = 0;
				std::vector<std::string> tmpLine;
				if (line.find("{") == std::string::npos)
				{
					std::cerr << "Error: wrong config format" << std::endl;
					exit(1);
				}
				tmpLine.push_back(line);
				while (std::getline(file, line))
				{
					if (line.find("{") != std::string::npos)
					{
						std::cerr << "Error: wrong config format" << std::endl;
						exit(1);
					}
					if (line.find("}") != std::string::npos)
					{
						bracket_counter++;
						trigger = 1;
						break;
					}
					tmpLine.push_back(line);
				}
				if (trigger == 0)
				{
					std::cerr << "Error: wrong config format" << std::endl;
					exit(1);
				}
				parseLocation(tmpLine, index, ind_serv);
				index++;
			}
			else
			{
				if (inside == 0)
				{
					std::cerr << "Error: wrong config format" << std::endl;
					exit(1);
				}
				if (iss >> value)
					server[ind_serv].setServConf(key, value);
				else
				{
					std::cerr << "Error: wrong config format" << std::endl;
					exit(1);
				}
				if (key == "client_max_body_size")
				{
					if (!checkValue(value))
					{
						std::cerr << "Error: wrong config format" << std::endl;
						exit(1);
					}
				}
				if (iss >> value)
				{
					std::cerr << "Error: too many arguments in one line in conf" << std::endl;
					exit(1);
				}
			}
		}
		if (line.find("{") != std::string::npos || line.find("}") != std::string::npos)
			bracket_counter++;
	}
	if (bracket_counter % 2 != 0 || bracket_counter == 0)
	{
		std::cerr << "Error: wrong config format" << std::endl;
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < servSize; i++)
	{
		if (server[i].getServConf("web_root") == server[i].getConfEnd())
		{
			std::cerr << "Missing web_root in Server number " << i + 1 << std::endl;
			exit(1);
		}
	}
	file.close();
}