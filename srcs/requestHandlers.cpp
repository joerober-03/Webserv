#include "webserv.hpp"

std::string ServerSocket::getFileInfo(std::string path, int type)
{
	FILE * fin;
	std::vector<char> bufferFile;
	std::string response;
	std::string path_cpy;
	int return_value = 0;

	if (type == 0)
	{
		std::map<int, std::string> tmp = parseFileInfo(path);
		std::map<int, std::string>::iterator it;
		it = tmp.begin();
		return_value = it->first;
		if (return_value == 1)
			return (it->second);
		path = it->second;
	}
	else
	{
		std::map<std::string, std::string>::iterator it;
		path_cpy = path;
		if (path_cpy[path_cpy.length() - 1] != '/')
			path_cpy.append("/");
		it = currentServ.getServConf("web_root");
		if (it != currentServ.getConfEnd())
			path = it->second + path;
	}
	path_cpy = path;
	if (type == 0)
	{
		if (path_cpy.length() == 1)
		{
			fin = fopen(path.c_str(), "rb");
		}
		else
			fin = fopen(path.c_str(), "rb");
	}
	else
		fin = fopen(path.c_str(), "rb");
	if (fin == NULL || return_value == -1)
	{
		std::map<std::string, std::string>::iterator it;
		it = currentServ.getServError(std::to_string(404));
		if (it != currentServ.getErrorEnd())
		{
			std::string tmp = it->second;
			std::map<std::string, std::string>::iterator it2;
			it2 = currentServ.getServConf("web_root");
			if (it2 != currentServ.getConfEnd())
				tmp = it2->second + it->second;
			fin = fopen(tmp.c_str(), "rb");
			if (fin == NULL)
			{
				fclose(fin);
				return (buildErrorFiles("404 Not Found"));
			}
		}
		fclose(fin);
		return (callErrorFiles(404));
	}
	fseek(fin, 0, SEEK_END);
	long file_len = ftell(fin);
	rewind(fin);
	bufferFile.clear();
	bufferFile.resize(file_len);
	fread(&bufferFile[0], 1, file_len, fin);
	fclose(fin);
	std::string content(bufferFile.begin(), bufferFile.end()); 
	response = "HTTP/1.1 200 OK\r\n\r\n" + content;
	return (response);
}

std::string ServerSocket::handleGetRequest(const std::string &path, const std::string &buffer)
{
	std::string response;

	if (path.find(".php") != std::string::npos)
	{
		int result = checkPerms(buffer);
		if (result == 0)
			return (callErrorFiles(405));
		if (result > 1)
			return (callErrorFiles(result));
		return executeCGIScript("/usr/bin/php", path, "", "");
	}
	else if (buffer.find("Accept: text/html") != std::string::npos)
		response = getFileInfo(path, 0);
	else
		response = getFileInfo(path, 1);
	return (response);
}

std::string extractFilename(const std::string& header) {
	std::string filename;
	size_t filenamePos = header.find("filename=");
	if (filenamePos != std::string::npos)
	{
		filename = header.substr(filenamePos + 10);
		filename = filename.substr(0, filename.find("\""));
	}
	return filename;
}

/*
This function finds the boundaries in the POST request and extracts the binary body
of the uploaded file.
*/
std::string ServerSocket::handlePostRequest(const std::string& path, const std::string& buffer) {
	std::string body;
	std::string boundary;
	int pos;
	
	std::map<std::string, std::string>::iterator it;
	if (buffer.find("Content-Length: ") == std::string::npos)
		return(callErrorFiles(411));
	bufferSize = getLastPart(buffer, "Content-Length: ");
	pos = buffer.find("Content-Length: ");
	std::string content_len = buffer.substr(pos + 16, 1);
	if (stod(content_len) == 0)
		return (callErrorFiles(400));

	size_t pos_marker = buffer.find("boundary=");
	if (pos_marker == std::string::npos)
	{
		size_t pos_empty_line = buffer.find("\r\n\r\n");
		std::string extracted_line;
    	if (pos_empty_line != std::string::npos)
		{
        	std::string body = buffer.substr(pos_empty_line + 4);
			std::istringstream body_stream(body);
			std::getline(body_stream, extracted_line);
		}
		it = currentServ.getServConf("client_max_body_size");
		if (it != currentServ.getConfEnd())
		{
			double len = extracted_line.length();
			if (len > stod(it->second))
				return (callErrorFiles(413));
		}
		return ("HTTP/1.1 200 OK\r\n\r\n" + extracted_line);
	}

	size_t end_marker;
	size_t i = pos_marker + std::string("boundary=").length() - 1;

	while (++i < buffer.length() && buffer[i] != '\n')
		boundary.push_back(buffer[i]);
	pos_marker = buffer.find("Content-Type", i);
	i = pos_marker - 1;
	while (++i < buffer.length() && buffer[i] != '\n');
	i += 2;
	end_marker = buffer.find(boundary.substr(0, boundary.length() - 1), i);
	while (++i < buffer.length() && i < end_marker - 2)
		body.push_back(buffer[i]);
	it = currentServ.getServConf("client_max_body_size");
	if (it != currentServ.getConfEnd())
	{
		double len = body.length();
		if (len > stod(it->second))
			return (callErrorFiles(413));
	}
	size_t contentDispositPos = buffer.find("Content-Disposition");
	std::string contentDispositHeader = buffer.substr(contentDispositPos, end_marker - contentDispositPos);
	std::string filename = extractFilename(contentDispositHeader);

	uploaded_files.push_back(filename);

	if (path == "/upload.html")
	{
		std::ofstream outfile(("uploaded_files/" + filename).c_str(), std::ios::binary);  // Save the uploaded image with the extracted filename
		if (outfile.fail())
			return "No file was chosen";
		outfile << body;                                                                  // Put the body of the uploaded file into the folder
		outfile.close();

		return handleGetRequest(path, buffer) + "\nSuccessfully uploaded!<br>";
	}
	if (path.find(".php") != std::string::npos)
		return(executeCGIScript("/usr/bin/php", path, body, filename));
	return "Unsupported HTTP method\n";
}

std::string ServerSocket::handleDeleteRequest(const std::string& path)
{
	if ((path == "/upload.html" || path == "/cgi-bin/cgi.php") && !uploaded_files.empty())
	{
		std::string lastFilename = uploaded_files.back();
		uploaded_files.pop_back();

		if (remove(("uploaded_files/" + lastFilename).c_str()) == 0)
			return "HTTP/1.1 200 Ok\r\n\r\n\nResource deleted successfully!";
	}
	return "HTTP/1.1 404 Not Found\r\n\r\n\nResource not found";
}

/*
This function is called in the main loop in _respond() function.
It differentiates between GET, POST and DELETE methods (requests).
CheckPerms checks "deny all" and allowed methods.
*/
std::string ServerSocket::handleHttpRequest(std::string &buffer)
{
	std::istringstream request(buffer);
	std::string method, path, line, protocol, path_cpy;
	request >> method >> path >> protocol;
	if (method != "GET" && method != "POST" && method != "DELETE")
		return (callErrorFiles(501));
	int result = 1;
	result = checkPerms(buffer);

	if (result > 1)
		return (callErrorFiles(result));
	if (!result)
		return (callErrorFiles(405));
	if (protocol != "HTTP/1.1")
		return (callErrorFiles(505));
	if (method == "GET")
		return handleGetRequest(path, buffer);
	else if (method == "POST")
		return handlePostRequest(path, buffer);
	else if (method == "DELETE")
		return handleDeleteRequest(path);
	return (NULL);
}
