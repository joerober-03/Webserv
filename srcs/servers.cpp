#include "webserv.hpp"

servers::servers()
{
}

servers::servers(const servers &copy)
{
	*this = copy;
}

servers &servers::operator=(const servers &copy)
{
	this->server_config = copy.server_config;
	this->server_error = copy.server_error;
	this->ports = copy.ports;
	this->server_location = copy.server_location;
	return (*this);
}

servers::~servers() {}

/*
We have to handle multiple servers. They are saved as a vector of our class "servers".
All the functions below return an iterator because it's the only way to access the memory.
*/

std::map<std::string, std::string>::iterator servers::getServConf(const std::string &key)
{
	std::map<std::string, std::string>::iterator it = this->server_config.find(key);
	return it;
}

std::map<std::string, std::string>::iterator servers::getServError(const std::string &key)
{
	std::map<std::string, std::string>::iterator it = this->server_error.find(key);
	return it;
}

std::vector<double> servers::getPorts() const
{ return ports; }

std::map<std::string, std::string>::iterator  servers::getServLocation(const int index, const std::string &key)
{
	std::map<std::string, std::string>::iterator it = this->server_location[index].find(key);
	return it;
}

std::map<std::string, std::string>::iterator servers::getConfEnd()
{
	std::map<std::string, std::string>::iterator it = this->server_config.end();
	return it;
}

std::map<std::string, std::string>::iterator servers::getErrorEnd()
{
	std::map<std::string, std::string>::iterator it = this->server_error.end();
	return it;
}

std::map<std::string, std::string>::iterator  servers::getLocationEnd(const int index)
{
	std::map<std::string, std::string>::iterator it = this->server_location[index].end();
	return it;
}

std::map<std::string, std::string>::iterator  servers::getLocationBegin(const int index)
{
	std::map<std::string, std::string>::iterator it = this->server_location[index].begin();
	return it;
}

int  servers::getLocationSize()
{
	return server_location.size();
}


void	servers::setServConf(const std::string first, const std::string second)
{
	this->server_config[first] = second;
}

void	servers::setServError(const std::string first, const std::string second)
{
	this->server_error[first] = second;
}

void	servers::setPorts(const double port)
{
	this->ports.push_back(port);
}

void	servers::setServLocation(const std::map<std::string, std::string> rhs)
{
	this->server_location.push_back(rhs);
}