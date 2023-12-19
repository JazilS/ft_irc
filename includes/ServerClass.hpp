#ifndef SERVERCLASS_HPP
# define SERVERCLASS_HPP

class	User;
class	Channel;

class Server
{
	public:

	Server(int port, const char *password);
	~Server();

	// utils
	void								LaunchServer();
	static void 						SignalHandler(int);
	void								AddUser();
	Channel*							AddChannel(std::string name);
	void								AddUserToChannel(User *user, std::string name);
	std::string 						HandlePackets(std::string &currentCmd, const std::string &packet);
	bool								HasChannel(std::string name);
	bool								IsPassCorrect(std::string channel, std::string password);
	bool								HasPass(std::string channel);
	void								RemoveChannel(Channel *toRemove);
	void								RemoveUser(User *toRemove);
	
	//getter
	Channel								*GetChannelByName(std::string);
	int									GetFdByNickName(std::string nickName);
	User								*GetUserByNickname(std::string nickName);
	User								*GetUserByFd(int fd);
	std::string							GetServerPassword( void );
	int									GetEpollFd();
	epoll_event*						GetClientEvent();
	std::map<int, User *>&				GetUsers();

	private:
	std::string							_buffer;
	int									_port;
	std::string							_password;
	int									_socketServer;
	int									_epollfd;
	sockaddr_in							_serverAddress;
	epoll_event							_serverEvent;
	epoll_event							_clientEvent;
	epoll_event 						_events[MAX_EVENTS];
	std::map<int, User *>				_users;
	std::map<std::string, Channel *>	_channels;
	std::map<int, std::string>	_buffer_map;
};

# endif