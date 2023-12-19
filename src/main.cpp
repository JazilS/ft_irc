#include "irc.hpp"

volatile sig_atomic_t g_signal = false;

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cout << "Error: please provide a password and a valid port to connect to irc server\n";
		return 1;
	}
	std::string port = av[1];
	std::string pass = av[2];

	if (port.empty() == true || pass.empty() == true)
		return 1;
	Server server(atoi(port.c_str()), pass.c_str());
	server.LaunchServer();
	return 0;
}