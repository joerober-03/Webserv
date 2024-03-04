#pragma once

#include "webserv.hpp"

#include <iostream>
#include <map>
#include <vector>

class servers
{
    private:

    std::map<std::string, std::string> server_config;
    std::map<std::string, std::string> server_error;
    std::vector<double> ports;
    std::vector<std::map<std::string, std::string> > server_location;

    public:

    servers();
    servers(const servers &copy);
    servers &operator=(const servers &copy);
    ~servers();

    std::map<std::string, std::string>::iterator getServConf(const std::string &key);
    std::map<std::string, std::string>::iterator getServError(const std::string &key);
    std::vector<double> getPorts() const;
    std::map<std::string, std::string>::iterator getServLocation(const int index, const std::string &key);
    
    std::map<std::string, std::string>::iterator  getLocationEnd(const int index);
    std::map<std::string, std::string>::iterator getErrorEnd();
    std::map<std::string, std::string>::iterator getConfEnd();

    std::map<std::string, std::string>::iterator  getLocationBegin(const int index);

    int  getLocationSize();
    
    void	setServConf(const std::string first, const std::string second);
    void	setServError(const std::string first, const std::string second);
    void	setPorts(const double port);
    void	setServLocation(const std::map<std::string, std::string> rhs);
};