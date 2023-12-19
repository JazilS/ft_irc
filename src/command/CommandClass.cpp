#include "irc.hpp"

Command::Command(std::string src)
{
	size_t	pos = 0;
	bool	first = true;

	if (src.empty())
	{
		_name = "";
		return ;
	}
	while (pos != std::string::npos)
	{
		pos = src.find(' ');
		if (first == true)
		{
			_name = src.substr(0, pos);
			first = false;
		}
		else
			_param.push_back(src.substr(0, pos));
		src = src.erase(0, pos + 1);
	}
	this->SetUpCommandsContainer();
}

Command::~Command()
{
	return ;
}

void	Command::ExecCommand(int clientFd, Server *server)
{
	if (this->_commands.find(this->_name) != _commands.end())
	{
		std::cout << this->_name << std::endl;
		(this->*this->_commands[this->_name])(server->GetUserByFd(clientFd), server);
	}
}

void	Command::SetUpCommandsContainer()
{
    _commands["PASS"] = &Command::PASS;
	_commands["NICK"] = &Command::NICK; 
    _commands["USER"] = &Command::USER; 
    _commands["MODE"] = &Command::MODE;
    _commands["PING"] = &Command::PING;
	_commands["PRIVMSG"] =&Command::PRIVMSG;
	_commands["JOIN"] = &Command::JOIN;
	_commands["WHOIS"] = &Command::WHOIS;
	_commands["INVITE"] = &Command::INVITE;
	_commands["PART"] = &Command::PART;
	_commands["TOPIC"] = &Command::TOPIC; 
	_commands["KICK"] = &Command::KICK;
	_commands["QUIT"] = &Command::QUIT;
}

void	Command::QUIT(User *user, Server *server)
{
	if (user->GetValidity() == true)
	{
		std::vector<Channel *>	channels;

		channels = user->GetChannels();
		for (size_t i = 0; i < channels.size(); i++)
		{
			if (channels[i]->GetUsers().size() == 1)
				server->RemoveChannel(channels[i]);
			else
			user->LeaveChannel(channels[i]);
			channels[i]->RemoveUser(user);
		}
	}
	return ;
}

void	Command::KICK(User *user, Server *server)
{
	Channel		*chan = NULL;
	User		*dest = NULL;
	std::string	chanStr;
	bool		hasChan = false;

	if (user->GetValidity() == true)
	{
		for (size_t i = 0; i < this->_param.size(); i++)
		{
			if (this->_param[i][0] == ':')
				break;
			else if (this->_param[i][0] == '#' && this->_param[i].size() > 1) // si il y a un channel de preciser
			{
				chanStr.assign(this->_param[i]);
				chanStr.erase(0, 1);
				chan = server->GetChannelByName(chanStr);
				if (!chan)
					SendOneMsg(user, ERR_NOSUCHCHANNEL(user->GetNickname(), '#' + chanStr));
				else if (chan->HasUser(user) == false)
					SendOneMsg(user, ERR_NOTONCHANNEL(user->GetNickname(), chan->GetName()));
				else if (chan->IsOper(user) == false)
					SendOneMsg(user, ERR_CHANOPRIVSNEED(user->GetNickname(), chan->GetName()));
				else
					hasChan = true;
			}
			else
			{
				if (hasChan == false)
					continue;
				else
				{
					dest = server->GetUserByNickname(this->_param[i]);
					if (!dest)
						SendOneMsg(user, ERR_USERNOTINCHANNEL(user->GetNickname(), this->_param[i], '#' + chan->GetName()));
					else
					{
						SendOneMsg(dest, KICK_CLIENT(user->GetNickname(), user->GetUsername(), "Kick", chan->GetName(), this->_param[i]));
						chan->RemoveUser(user);
						hasChan = false;
					}
				}
			}
		}
	}
}

void	Command::PART(User *user, Server *server)
{
	Channel		*chan;
	std::string	chanStr;
	
	if (user->GetValidity() == true)
	{
		for (size_t i = 0; i < this->_param.size(); i++)
		{
			if (this->_param[i][0] == ':')
				break;
			if (this->_param[i][0] == '#' && this->_param[i].size() > 1)
			{
				chanStr.assign(this->_param[i]);
				chanStr.erase(0, 1);
				chan = server->GetChannelByName(chanStr);
				if (!chan)
					SendOneMsg(user, ERR_NOSUCHCHANNEL(user->GetNickname(), '#' + chanStr));
				else if (chan->HasUser(user) == false)
					SendOneMsg(user, ERR_NOTONCHANNEL(user->GetNickname(), chan->GetName()));
				else
				{
					if (chan->GetUsers().size() == 1)
						server->RemoveChannel(chan);
					user->LeaveChannel(chan);
					chan->RemoveUser(user);
					SendOneMsg(user, PART_CHANEL(user->GetNickname(), user->GetUsername(), "PART", chan->GetName()));
				}
			}
	}
	}
	
	return ;
}

bool	Command::CheckErrorTopic(User *user, Channel *channel, std::string channelName)
{
	if (channelName == "")
	{
		SendOneMsg(user, ERR_NEEDMOREPARAMS(user->GetNickname(), this->_name));
		return (true);
	}		
	if (!channel)
	{
		SendOneMsg(user, ERR_NOSUCHCHANNEL(user->GetNickname(), channelName));
		return (true);
	}
	if (!channel->HasUser(user))
	{
		SendOneMsg(user, ERR_NOTONCHANNEL(user->GetNickname(), channel->GetName()));
		return (true);
	}
	return (false);
}

void	Command::TOPIC(User *user, Server *server)
{
	if (user->GetValidity() == true)
	{
		Channel		*channel = server->GetChannelByName(_param[0].erase(0, 1));

		if (this->CheckErrorTopic(user, channel, _param[0]))
			return ;
		if (_param.size() == 1) // le user demande quel est le sujet de chan
		{
			if (channel->GetTopic() == "")
				SendOneMsg(user, RPL_NOTOPIC(user->GetNickname(), channel->GetName()));
			else
				SendOneMsg(user, RPL_TOPIC(user->GetNickname(), channel->GetName(), channel->GetTopic()));
		}
		else                   // le user veut set un nouveau topic
		{
			if (channel->GetModes().find('t') == std::string::npos) // le topic n'est pas protégé
			{
				channel->SetTopic(this->GetTopic());
				SendGroupedMsg(channel->GetUsers(), RPL_TOPIC(user->GetNickname(), channel->GetName(), channel->GetTopic()));
			}
			else               // le topic est protégé -> seul un operateur peut le modifier
			{
				if (channel->IsOper(user))
				{
					channel->SetTopic(this->GetTopic());
					SendGroupedMsg(channel->GetUsers(), RPL_TOPIC(user->GetNickname(), channel->GetName(), channel->GetTopic()));
				}
				else
					SendOneMsg(user, ERR_CHANOPRIVSNEED(user->GetNickname(), channel->GetName()));
			}
		}
	}
}

void	Command::INVITE(User *user, Server *server)
{
	Channel *chan = NULL;
	User	*target = NULL;
	std::string tmp;

	if (user->GetValidity() == true)
	{
		if (this->_param.size() < 2)
			return ;

		tmp.assign(this->_param[1]);
		if (this->_param[1].find('#') != std::string::npos) // si contient un '#', supprimer
			this->_param[1].erase(0, 1);
		if (server->HasChannel(this->_param[1]) == true) // channel exist
		{
			chan = server->GetChannelByName(this->_param[1]);
			target = server->GetUserByFd(server->GetFdByNickName(this->_param[0]));
			if (chan->HasUser(user) == false) 															// le user est pas membre du channel
				SendOneMsg(user, ERR_NOTONCHANNEL(user->GetNickname(), tmp));
			else if (chan->IsOper(user) == false && chan->GetModes().find('i') != std::string::npos)	// user pas op et channel en invite only
				SendOneMsg(user, ERR_CHANOPRIVSNEED(user->GetNickname(), tmp));
			else if (chan->HasUser(target))																// user target est deja dans channel
				SendOneMsg(user, ERR_USERONCHANNEL(target->GetNickname(), this->_param[1]));
			else																						// inviter le client
			{
				SendOneMsg(user, RPL_INVITING(user->GetNickname(), target->GetNickname(), chan->GetName()));
				SendOneMsg(target, INVITE_CLIENT(user->GetNickname(), user->GetUsername(), "Invite", target->GetNickname(), chan->GetName()));
				chan->AddUserToInviteList(target);
			}
		}
		else
			SendOneMsg(user, ERR_NOSUCHCHANNEL(user->GetNickname(), this->_param[1]));				    // channel inexistant
	}
}

void	Command::PrintWhoIs(User *user, User *target)
{
	SendOneMsg(user, RPL_WHOISUSER(target->GetNickname(), target->GetHostname(), target->GetUsername()));
	SendOneMsg(user, RPL_ENDOFWHOIS(target->GetNickname()));
	return ;
}

void	Command::WHOIS(User *user, Server *server)
{
	if (user->GetValidity() == true)
	{
		if (user->GetAuth() == false)
			return ;
		for (size_t index = 0 ; index < this->_param.size() ; index++)
		{
			int fdToFind = -1;
			fdToFind = server->GetFdByNickName(this->_param[index]);
			if (fdToFind != -1)
				PrintWhoIs(user, server->GetUserByFd(fdToFind));
			else
				SendOneMsg(user, ERR_NOSUCHNICK(this->_param[index]));
		}
	}
	return ;
}

void	Command::JOIN(User *user, Server *server)
{
	Channel		*chan;
	std::string	modes;
	std::string replies;
	size_t		pos;
	size_t		len;
	bool		newChanCreated = false;

	if (user->GetValidity() == true)
	{
		for (unsigned long i = 0; i < this->_param.size(); i++)
		{
			switch (this->_param[i][0])
			{
				case '#':
					this->_param[i].erase(0, 1); // enlever le '#' du nom du channel
					if (server->HasChannel(this->_param[i]) == false) // channel inexistant
					{
						chan = server->AddChannel(this->_param[i]); // ajouter channel dans classe server
						user->JoinChannel(chan); // ajout du channel dans vector de classe user
						chan->AddUser(user); // ajout du user dans vector de classe channel
						chan->AddOper(user); // ajout du user dans vector operator de classe channel
						chan->SetFounder(user->GetNickname()); // set le user en tant que fondateur du channel
						SendOneMsg(user, RPL_JOIN(user->GetNickname(), chan->GetName()));
						if (chan->GetTopic().empty() == false)
							SendOneMsg(user, RPL_TOPIC(user->GetNickname(), chan->GetName(), chan->GetTopic()));
						std::string userNickname = "@" + user->GetNickname();
						SendOneMsg(user, RPL_NAMREPLY(userNickname, chan->GetName(), chan->GetClientList()));
						SendOneMsg(user, RPL_ENDOFNAMES(user->GetNickname(), chan->GetName()));
						newChanCreated = true;
					}
					else // channel existant
					{
						chan = server->GetChannelByName(this->_param[i]);
						if (this->_param.size() > 1 && this->_param[i + 1][0] != '+') // si a 2e arg et n'est pas un mode
						{
							if (chan->IsPassCorrect(this->_param[i + 1]) == true
								&& chan->IsUserInvited(user) == true && chan->IsLimitExceeded() == false) // password correct, user invited et pas d'exces d'user
							{
								server->AddUserToChannel(user, this->_param[i]); // ajouter user a map de channel dans classe server
								SendOneMsg(user, RPL_JOIN(user->GetNickname(), chan->GetName()));
								if (chan->GetTopic().empty() == false)
									SendOneMsg(user, RPL_TOPIC(user->GetNickname(), chan->GetName(), chan->GetTopic()));
								SendOneMsg(user, RPL_NAMREPLY(user->GetNickname(), chan->GetName(), chan->GetClientList()));
								SendOneMsg(user, RPL_ENDOFNAMES(user->GetNickname(), chan->GetName()));
							}
							else if (chan->IsPassCorrect(this->_param[i + 1]) == false) // password incorrect
								SendOneMsg(user, ERR_BADCHANNELKEY(user->GetNickname(), this->_param[i]));
							else if (chan->IsUserInvited(user) == false) // user n'est pas invite
								SendOneMsg(user, ERR_INVITEONLYCHAN(user->GetNickname(), chan->GetName()));
							else // channel a atteint nbr d'user max
								SendOneMsg(user, ERR_CHANNELISFULL(user->GetNickname(), chan->GetName()));
						}
						else // pas de 2e arg ou bien c'est un mode
						{
							if (chan->HasPass() == false && chan->IsUserInvited(user) == true
								&& chan->IsLimitExceeded() == false) // chan pas de mdp, user invited, chan limit pas atteinte
							{
								server->AddUserToChannel(user, this->_param[i]); // ajouter user a map de channel dans classe server
								user->JoinChannel(chan);
								SendOneMsg(user, RPL_JOIN(user->GetNickname(), chan->GetName()));
								if (chan->GetTopic().empty() == false)
									SendOneMsg(user, RPL_TOPIC(user->GetNickname(), chan->GetName(), chan->GetTopic()));
								SendOneMsg(user, RPL_NAMREPLY(user->GetNickname(), chan->GetName(), chan->GetClientList()));
								SendOneMsg(user, RPL_ENDOFNAMES(user->GetNickname(), chan->GetName()));
							}
							else if (chan->HasPass() == true) // channel a un password
								SendOneMsg(user, ERR_BADCHANNELKEY(user->GetNickname(), this->_param[i]));
							else if (chan->IsUserInvited(user) == false) // user n'est pas invited
								SendOneMsg(user, ERR_INVITEONLYCHAN(user->GetNickname(), chan->GetName()));
							else // channel a atteint nbr d'user max
								SendOneMsg(user, ERR_CHANNELISFULL(user->GetNickname(), chan->GetName()));
						}
					}
					break ;
				case '+':
					if (this->_param[i].size() > 1 && newChanCreated == true) // si ne contient pas seulement '+' et que un nouveau chan a ete creer
					{
						pos = 1;
						len = (this->_param[i].length()) - 1;
						modes = this->_param[i].substr(pos, len);
						chan->SetModes(modes);
					}
					break;
			}
		}
	}
}

void	Command::SetModeParams(std::vector<std::string> *param)
{
	int i = 2;				// les parametres de mode sont stocké à partir de cet indice (ex: #channel(0) +ok-v(1) mp(2))
	int j = param->size() - 1;

	_modeParams.push("");	// on push une valeur par defaut au cas ou il n'y a pas d'arg pour eviter les segfault par la suite
	if (!param->empty())
	{
		while (j >= i)
		{
			_modeParams.push((*param)[j]);
			j--; 			// on rempli a l'envers car c'est une stack
		}
	}
}

bool	Command::CheckErrorMode(User *user, Channel *channel, std::string chanName)
{
	if (!channel)
	{	
		SendOneMsg(user, ERR_NOSUCHCHANNEL(user->GetNickname(), chanName));
		return (true);
	}
	if (_param.size() == 1) // qd irssi envoie "MODE #<channel_name>"
	{	
		SendOneMsg(user, RPL_CHANNELMODEIS(user->GetNickname(), channel->GetName(), channel->GetModes()));
		return (true);
	}
	if (!channel->IsOper(user))
	{	
		SendOneMsg(user, ERR_CHANOPRIVSNEED(user->GetNickname(), chanName));
		return (true);
	}
	return (false);
}

void	Command::MODE(User *user, Server *server)
{
	if (user->GetValidity() == true)
	{
		SetModeParams(&_param);

		if (_param[0][0] != '#')	// on ne gere que les mode de channel pas ceux des clients
			return;					// fail en silence
		
		_param[0].erase(0, 1);		//on supprime le hashtag
		Channel	*channel = server->GetChannelByName(_param[0]);

		if (CheckErrorMode(user, channel, _param[0]))
			return ;
		int i = 0;					// index des flag de mode
		
		if (_param[1][i] == '+')
			while (_param[1][++i] != '-' && _param[1][i])
				channel->SetModes(_param[1][i], &_modeParams, server, this, user); // on incremente j quand on a utilisé un param
		if (_param[1][i] == '-')
			while (_param[1][++i])
				channel->UnsetModes(_param[1][i], &_modeParams, server, this, user);
	}
	return ;
}

void	Command::PASS(User *user, Server *server)
{
	if (user->GetAuth() == false)
	{
		if ((this->_param[0] != server->GetServerPassword()))
		{
			std::cout << "PASS PAS OK" << std::endl;
			std::cout << this->_param[0] << std::endl;
			std::cout <<  server->GetServerPassword() << std::endl;
			SendOneMsg(user, ERR_PASSWDMISMATCH(user->GetNickname()));
            epoll_ctl(server->GetEpollFd(), EPOLL_CTL_DEL, user->GetFd(), server->GetClientEvent());
			close(user->GetFd());
			server->RemoveUser(user);
		}
		else 
		{
			std::cout << "PASS OK" << std::endl;
			user->SetAuth(true);
		}
	}
}

void	Command::NICK(User *user, Server *server)
{
	if (!user)
		return;
	if (user->GetAuth() == true)
	{
		user->SetNickname(this->_param[0], server);
		if (user->GetNickname().empty() == true)
		{
			std::cout << "NICK PAS OK" << std::endl;
			SendOneMsg(user, ERR_NICKNAMEINUSE(_param[0]));
			epoll_ctl(server->GetEpollFd(), EPOLL_CTL_DEL, user->GetFd(), server->GetClientEvent());
			close(user->GetFd());
			server->RemoveUser(user);
		}
		else
			std::cout << "NICK OK" << std::endl;
	}
	return ;
}

void	Command::USER(User *user, Server *server)
{
	(void)server;

	if (!user)
		return ;
	if (user->GetAuth() == true && this->_param.size() >= 4 && user->GetNickname().empty() == false)
	{
		std::cout << "USER OK" << std::endl;
		user->SetUsername(this->_param[0]);
		user->SetHostname(this->_param[1]);
		user->SetServername(this->_param[2]);
		user->SetRealname(&_param);
		user->SetValidity(true);
		SendOneMsg(user, RPL_WELCOME(user->GetNickname()));
	}
	else
	{
		std::cout << "USER PAS OK" << std::endl;
		epoll_ctl(server->GetEpollFd(), EPOLL_CTL_DEL, user->GetFd(), server->GetClientEvent());
		close(user->GetFd());
		server->RemoveUser(user);
	}
	return ;
}


void	Command::PING(User *user, Server *server)
{
	if (user->GetValidity() == true)
	{
		(void)server;

		if (user->GetAuth())
			SendOneMsg(user, RPL_PONG);
	}
	return ;
}

std::string	Command::GetMsg()
{
	std::string	msg;
	size_t 		i = 1;

	while (i < _param.size())
	{
		msg += _param[i];
		msg += " ";
		i++;
	}
	return (msg);
}

std::string	Command::GetTopic()
{
	std::string	topic;
	size_t 		i = 1;

	while (i < _param.size())
	{
		topic += _param[i];
		topic += " ";
		i++;
	}
	return (topic);
}

void	Command::SendToUser(User *user, Server *server)
{
	size_t	i;

	while (_param[0] != "")
	{
		i = _param[0].find(",");

		if (i == std::string::npos)
			i = _param[0].size();
		User	*recipient = server->GetUserByNickname(_param[0].substr(0, i));

		if (recipient) // si le user appartient bien au server
			SendOneMsg(recipient, RPL_PRIVMSG_CLIENT(user->GetNickname(), this->GetMsg()));
		else // le user est inconnu
			SendOneMsg(user, ERR_NOSUCHNICK(_param[0].substr(0, i)));
		_param[0].erase(0, i + 1);
	}
}

void	Command::SendToChannel(User *user, Server *server)
{
	this->_param[0].erase(0, 1); // on retire le '#' devant le nom du channel
	Channel	*recipient = server->GetChannelByName(this->_param[0]);

	if (server->HasChannel(this->_param[0]) == false) // check si le chan existe
	{
		SendOneMsg(user, ERR_NOSUCHCHANNEL(user->GetNickname(), _param[0]));
		return ;
	}
	if (!recipient->HasUser(user) && recipient->IsUserInvited(user) == false) // check si le user appartient bien au channel
	{
		SendOneMsg(user, ERR_CANNOTSENDTOCHAN(this->_param[0], recipient->GetName()));
		return ;
	}
	if (this->_param[1] == "") // check si le msg n'est pas vide
	{
		SendOneMsg(user, ERR_NOTEXTTOSEND(this->_param[0]));
		return ;
	}
	SendChanMsg(recipient->GetUsers(), user, RPL_PRIVMSG_CHANNEL(user->GetNickname(), recipient->GetName(), this->GetMsg()));
}

void	Command::PRIVMSG(User *user, Server *server)
{
	if (user->GetValidity() == true)
	{
		if (_param.size() < 2)
		{
			SendOneMsg(user, ERR_NEEDMOREPARAMS(user->GetNickname(), this->_name));
			return;
		}
		if (this->GetParameters()[0][0] == '#')
			this->SendToChannel(user, server);
		else
			this->SendToUser(user, server);
	}
	return ;
}

std::string	Command::GetCmdName()
{
	return(this->_name);
}

std::vector<std::string>	Command::GetParameters()
{
	return (this->_param);
}

void		Command::SendGroupedMsg(std::vector<User *> recipients, std::string msg)
{
	std::vector<User *>::iterator	it = recipients.begin();
	std::vector<User *>::iterator	ite = recipients.end();
	int 							len = msg.size();

	while (it != ite)
	{
		send((*it)->GetFd(), msg.c_str(), len, 0);
		it++;
	}
}

void		Command::SendChanMsg(std::vector<User *> recipients, User *sender, std::string msg)
{
	std::vector<User *>::iterator	it = recipients.begin();
	std::vector<User *>::iterator	ite = recipients.end();
	int 							len = msg.size();

	while (it != ite)
	{
		if ((*it)->GetNickname() != sender->GetNickname())
			send((*it)->GetFd(), msg.c_str(), len, 0);
		it++;
	}
}

void        Command::SendOneMsg(User* recipient, std::string msg)
{
	int 		len = msg.size();

	if ((send(recipient->GetFd(), msg.c_str(), len, 0)) != len)
		return ;
}