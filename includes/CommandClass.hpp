#ifndef COMMANDCLASS_HPP
# define COMMANDCLASS_HPP

class Channel;
class User;
class Server;
class Command;

typedef void (Command::*CmdFctPtr)(User *, Server *);

class Command
{
	public:
	
	Command(std::string src);
	~Command();

	// getter
	std::vector<std::string>			GetParameters();
	std::string							GetCmdName();
	std::string							GetMsg();
	std::string							GetTopic();

	// utils
	void								ExecCommand(int clientFd, Server *server);
	void 								SendToChannel(User *user, Server *server);
	void 								SendToUser(User *user, Server *server);
	void								SendOneMsg(User *recipient, std::string msg);
	void								SendGroupedMsg(std::vector<User *> recipients, std::string msg);
	void								SendChanMsg(std::vector<User *> recipients, User *sender, std::string msg);
	void								SetUpCommandsContainer();
	void								SetModeParams(std::vector<std::string> *param);
	void								PrintWhoIs(User *user, User *target);
	bool								CheckErrorTopic(User *user, Channel *channel, std::string chanName);
	bool								CheckErrorMode(User *user, Channel *channel, std::string chanName);

	// toutes les commandes
	void						MODE(User *user, Server *server);
	void						JOIN(User *user, Server *server);
	void						PING(User *user, Server *server);
	void						NICK(User *user, Server *server);
	void						USER(User *user, Server *server);
	void						PASS(User *user, Server *server);
	void						PRIVMSG(User *user, Server *server);
	void						WHOIS(User *user, Server *server);
	void						INVITE(User *user, Server *server);
	void						PART(User *user, Server *server);
	void						TOPIC(User *user, Server *server);
	void						KICK(User *user, Server *server);
	void						QUIT(User *user, Server *server);
	
	private:

	std::string							_name;
	std::vector<std::string>			_param;
	std::stack<std::string>				_modeParams;
	std::map<std::string, CmdFctPtr>	_commands;
};

#endif