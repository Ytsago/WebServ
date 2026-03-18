#include "CgiHandler.hpp"
#include "Logger.hpp"
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

CgiHandler::CgiHandler() {}

CgiHandler::CgiHandler(const CgiHandler &other) 
{
	(void)other;
}

CgiHandler	&CgiHandler::operator=(const CgiHandler &other) 
{
	(void)other;
	return (*this);
}

CgiHandler::~CgiHandler() {
}

static void	dup_fd(int fd1, int fd2)
{
	if (dup2(fd1, fd2) == -1)
		throw std::runtime_error("dup2 error");
}

static void	close_pipes(int *pipefd)
{
	int	i;

	i = 0;
	while (i < 4)
	{
		close(pipefd[i]);
		i++;
	}
}

static std::map<std::string, std::string> build_env(const ServerConfig &server, HttpRequest &request, LocationConfig &location, std::string &path)
{
	std::map<std::string, std::string> env;
	std::string	uri = request.getUri();
	std::string	query;
	size_t		pos;

	pos = uri.find('?');
	query = (pos == std::string::npos) ? "" : uri.substr(pos + 1);
	env["REQUEST_METHOD"] = request.getMethod();
	env["QUERY_STRING"] = query;
	env["CONTENT_LENGTH"] = request.getHeader("Content-Length");
	env["CONTENT_TYPE"] = request.getHeader("Content-Type");
	env["PATH_INFO"] = uri;
	env["PATH_TRANSLATED"] = path;
	env["SCRIPT_NAME"] = location.get_index();
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["SERVER_NAME"] = server.get_server_name();
	env["SERVER_PORT"] = server.get_listen_port();
	return (env);
}

static void	clear_envp(char **envp)
{
	size_t	i = 0;

	while (envp[i])
	{
		delete [] envp[i];
		i++;
	}
	delete [] envp;
}

static char **map_to_envp(std::map<std::string, std::string> &env)
{
	char	**envp;
	size_t	idx = 0;
	size_t	map_size = env.size();
	std::map<std::string, std::string>::iterator it;

	envp = new char*[map_size + 1];
	for (size_t i = 0; i <= map_size; ++i)
    	envp[i] = NULL;
	try
	{
		for (it = env.begin(); it != env.end(); it++, idx++)
		{
			std::string entry = it->first + "=" + it->second;
			char *field = new char[entry.size() + 1];
			std::copy(entry.begin(), entry.end(), field);
			field[entry.size()] = '\0';
			envp[idx] = field;
		}
	}
	catch(const std::exception& e)
	{
		clear_envp(envp);
		throw;
	}
	return (envp);
}

t_pipe CgiHandler::execute_cgi(const ServerConfig &server, HttpRequest &request, LocationConfig &location, std::string &path, pid_t &pid)
{
	std::map<std::string, std::string> env;
	std::vector<char>	body = request.getBody();
	char	**envp;
	int		pipefd[4];

	Logger::record(DEBUG) << "Cgi path: " << location.get_cgi_path() << "\npath: " << path;
	env = build_env(server, request, location, path);
	envp = map_to_envp(env);
	if (access(path.c_str(), X_OK) == -1)
	{
		pid = -1;
		return (t_pipe){-1, -1};
	}
	for (int i = 0; i < 2; i++)
	{
		if (pipe(pipefd + i * 2) == -1)
		{
			pid = -1;
			return (t_pipe) {-1, -1};
		}
	}
	pid = fork();
	if (pid == -1) {
		clear_envp(envp);
		close_pipes(pipefd);
		return (t_pipe) {-1, -1};
	}
	else if (pid == 0)
	{
		std::string	cgi_path = location.get_cgi_path();
		dup_fd(pipefd[0], STDIN_FILENO);
		dup_fd(pipefd[3], STDOUT_FILENO);
		close_pipes(pipefd);
		char **argv = new char*[3];
		argv[0] = const_cast<char *>(cgi_path.c_str());
		argv[1] = const_cast<char *>(path.c_str());
		argv[2] = NULL;
		execve(location.get_cgi_path().c_str(), argv, envp);
		delete [] argv;
		clear_envp(envp);
		exit(0);
	}
	clear_envp(envp);
	close (pipefd[0]);
	close (pipefd[3]);
	return (t_pipe) {pipefd[1], pipefd[2]};
}


