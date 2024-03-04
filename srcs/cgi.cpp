#include "webserv.hpp"

/*
This function executes scripts that have a shebang. In our case, it's cgi.php but it can be .py or .pl.
*/
std::string ServerSocket::executeCGIScript(const std::string &shebang, const std::string &cgiScriptPath, const std::string &body, const std::string &filename)
{
	std::string response_data;
	int stdin_pipe[2];
	int stdout_pipe[2];
	char **argv = (char **)malloc(sizeof(char *) * 3);
	if (argv == NULL)
	{
		checkFdSets();
		return (callErrorFiles(500));
	}
	char **envp;

	std::string path;
	std::map<std::string, std::string>::iterator it = currentServ.getServConf("web_root");
	if (it != currentServ.getConfEnd())
		path = it->second + cgiScriptPath;
	else
		path = cgiScriptPath;
	argv[0] = strdup(shebang.c_str());
	argv[1] = strdup(path.c_str());
	argv[2] = 0;
	if (filename[0] == 0)
	{
		envp = (char **)malloc(sizeof(char *) * 7);
		if (envp == NULL)
		{
			checkFdSets();
			return (callErrorFiles(500));
		}
		envp[0] = strdup("REQUEST_METHOD=GET");
		envp[6] = 0;
	}
	else
	{
		envp = (char **)malloc(sizeof(char *) * 8);
		if (envp == NULL)
		{
			checkFdSets();
			return (callErrorFiles(500));
		}
		envp[0] = strdup("REQUEST_METHOD=POST");
		envp[6] = strdup((std::string("FILENAME=").append(filename)).c_str());
		envp[7] = 0;
	}
	envp[1] = strdup((std::string("CONTENT_LENGTH=").append(bufferSize)).c_str());
	envp[2] = strdup(std::string("PATH_INFO=").append(getPathInfo(currentPath)).c_str());
	envp[3] = strdup("PATH_TRANSLATED=");
	envp[4] = strdup(std::string("QUERY_STRING=").append(getQueryString(currentPath)).c_str());
	envp[5] = strdup(std::string("SERVER_NAME=").append(serverName).c_str());

	if (pipe(stdin_pipe) == -1 || pipe(stdout_pipe) == -1)
	{
		perror("In pipe");
		checkFdSets();
		return (callErrorFiles(500));
	}
	pid_t pid = fork();
	if (pid == -1)
	{
		perror("In fork");
		checkFdSets();
		return (callErrorFiles(500));
	}
	if (pid == 0)
	{
		close(stdin_pipe[1]);
		close(stdout_pipe[0]);
		dup2(stdin_pipe[0], STDIN_FILENO);
		dup2(stdout_pipe[1], STDOUT_FILENO);

		execve(argv[0], argv, envp);
		perror("In execve");
		exit(EXIT_FAILURE);
		return "";
	}
	else
	{
		close(stdin_pipe[0]);
		close(stdout_pipe[1]);
		write(stdin_pipe[1], body.c_str(), body.size());
		close(stdin_pipe[1]);

		char buffer[1024];
		memset(buffer, 0, sizeof(buffer));

		response_data.clear();
		response_data.append("HTTP/1.1 200 OK\r\n\r\n");
		int bytes_read;
		while ((bytes_read = read(stdout_pipe[0], buffer, sizeof(buffer) - 1)) > 0)
		{
			response_data.append(buffer, bytes_read);
			memset(buffer, 0, bytes_read);
		}
		if (bytes_read == -1)
		{
			std::cerr << "Couldn't read any output" << std::endl;
			checkFdSets();
			return (callErrorFiles(500));
		}
		int status;
		waitpid(pid, &status, 0);
		mfree(argv);
		mfree(envp);
		status = status / 256;
		if (status == 1)
			return (callErrorFiles(404));
		if (status == 255)
			return (callErrorFiles(400));
	}
	return response_data;
}
