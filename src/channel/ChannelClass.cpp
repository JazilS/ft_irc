# include "irc.hpp"

Channel::Channel(const std::string name) : _name(name), _modes(""), _topic("")
{
	return ;
}

Channel::~Channel()
{
	return ;
}

std::string	Channel::GetName()
{
	if (this->_name.empty())
		return ("");
	else
		return (this->_name);
}

std::string	Channel::GetModes()
{
	return (this->_modes);
}

std::string Channel::GetTopic()
{
	return (this->_topic);
}

std::vector<User *>	&Channel::GetUsers()
{
	return (this->_users);
}

std::vector<User *>	&Channel::GetOpers()
{
	return (this->_opers);
}

bool	Channel::HasPass()
{
	if (this->_modes.find('k') == std::string::npos)
		return (false);
	else
		return (true);
}

bool	Channel::IsPassCorrect(std::string password)
{
	if (this->_modes.find('k') != std::string::npos)
	{
		if (this->_password == password)
			return (true);
		else
			return (false);
	}
	else
		return (true);
}

bool	Channel::IsLimitExceeded()
{
	if (this->_modes.find('l') != std::string::npos)
	{
		if (this->_users.size() >= this->_limit)
			return (true);
	}
	return (false);
}

bool	Channel::IsOper(User *toCheck)
{
	User *user = NULL;

	for (std::vector<User *>::iterator it = this->_opers.begin(); it != this->_opers.end(); it++)
	{
		user = *it;
		if (toCheck->GetNickname() == user->GetNickname())
			return (true);
	}
	return (false);
}

void	Channel::DelOper(User *toDel)
{
	User *user = NULL;

	for (std::vector<User *>::iterator it = this->_opers.begin(); it != this->_opers.end(); it++)
	{
		user = *it;
		if (toDel->GetNickname() == user->GetNickname())
		{
			this->_opers.erase(it);
			return ;
		}
	}
}

std::string	Channel::GetClientList()
{
	User *user = NULL;
	std::string userInfo;

	for (std::vector<User *>::iterator it = this->_users.begin(); it != this->_users.end(); it++)
	{
		user = *it;
		if (this->IsOper(user) == true)
			userInfo += '@';
		else
			userInfo += '+';
		userInfo += user->GetNickname();
		userInfo += ' ';
	}
	return (userInfo);
}

std::string	Channel::GetPassword()
{
	return (this->_password);
}

void	Channel::AddUser(User *toAdd)
{
	this->_users.push_back(toAdd);
	return ;
}

void	Channel::AddOper(User *toAdd)
{
	this->_opers.push_back(toAdd);
	return ;
}

void	Channel::AddUserToInviteList(User *toAdd)
{
	if (IsUserInvited(toAdd) == false)
		this->_invited.push_back(toAdd);
	return;
}

void	Channel::SetFounder(std::string founderName)
{
	this->_founder.assign(founderName);
	return ;
}

void	Channel::SetTopic(std::string newTopic)
{
	this->_topic.assign(newTopic);
}

bool	Channel::IsUserInvited(User *toCheck)
{
	if (this->_modes.find('i') == std::string::npos)
		return (true);
	else
	{
		for (std::vector<User *>::iterator it = this->_invited.begin(); it != this->_invited.end(); it++)
		{
			User *user = *it;
			if (user->GetNickname() == toCheck->GetNickname())
			{
				_invited.erase(it);
				return (true);
			}
		}
		return (false);
	}
}

void	Channel::SetModes(std::string modes)
{
	std::string	modes_available = "it";
	for (size_t i = 0; i < modes.size(); i++)
	{
		if (modes_available.find(modes[i]) != std::string::npos)
			this->_modes += modes[i];
	}
	return ;
}

void Channel::SetModes(char mode, std::stack<std::string>* modeParams, Server* server, Command* cmd, User* user)
{
    std::string availableModes = "itkol";
    std::string needParam = "kol";

    if (availableModes.find(mode) == std::string::npos)
    {
        cmd->SendOneMsg(user, ERR_UMODEUNKNOWNFLAG(user->GetNickname()));
        return;
    }

    if (modeParams == NULL || modeParams->empty()) // Check if modeParams is valid and not empty
    {
        return;
    }

    if (needParam.find(mode) != std::string::npos && modeParams->top().empty())
    {
        return;
    }

    switch (mode)
    {
        case 'k':
            SetModeK(modeParams, cmd, user);
            break;
        case 'o':
            SetModeO(modeParams, server, cmd, user);
            break;
        case 'l':
            SetModeL(modeParams, cmd, user);
            break;
        default:
            SetDefaultMode(mode, cmd, user);
            break;
    }
}

void Channel::SetModeK(std::stack<std::string>* modeParams, Command* cmd, User* user)
{
	(void)user;
	(void)cmd;
	_modes += 'k';
    _password = modeParams->top();
    modeParams->pop();
    cmd->SendGroupedMsg(_users, SET_CHANEL_MODE(user->GetNickname(), user->GetUsername(), cmd->GetCmdName(), _name, 'k'));
}

void Channel::SetModeO(std::stack<std::string>* modeParams, Server* server, Command* cmd, User* user)
{
    User* newOper = server->GetUserByNickname(modeParams->top());

    if (newOper == NULL)
        return;
    if (!IsOper(newOper))
        AddOper(newOper);
    cmd->SendGroupedMsg(_users, SET_NEWOPER(user->GetNickname(), user->GetUsername(), cmd->GetCmdName(), _name, 'o', modeParams->top()));
    modeParams->pop();
}

void Channel::SetModeL(std::stack<std::string>* modeParams, Command* cmd, User* user)
{
    int newLimit = atoi(modeParams->top().c_str());

    modeParams->pop();
    if (newLimit < 1 || newLimit > INT_MAX)
    {
        cmd->SendOneMsg(user, ERR_INVALIDLIMIT(user->GetNickname(), _name));
    }
    else
	{
		_modes += 'l';
        _limit = newLimit;
		 cmd->SendGroupedMsg(_users, SET_CHANEL_MODE(user->GetNickname(), user->GetUsername(), cmd->GetCmdName(), _name, "l"));
	}
}

void Channel::SetDefaultMode(char mode, Command* cmd, User* user)
{
    if (_modes.find(mode) == std::string::npos)
    {
        _modes += mode;
        cmd->SendGroupedMsg(_users, SET_CHANEL_MODE(user->GetNickname(), user->GetUsername(), cmd->GetCmdName(), _name, mode));
    }
}

void Channel::UnsetModes(char mode, std::stack<std::string> *modeParams, Server *server, Command *cmd, User *user)
{
    if (!isValidMode(mode))
    {
        cmd->SendOneMsg(user, ERR_UMODEUNKNOWNFLAG(user->GetNickname()));
        return;
    }

    if (mode != 'o')
    {
        unsetMode(mode, user, cmd);
    }

    if (mode == 'k')
    {
        unsetPassword();
    }
    else if (mode == 'o')
    {
        unsetOperator(modeParams, server, cmd, user);
    }
    else if (mode == 'l')
    {
        unsetLimit();
    }
}

bool Channel::isValidMode(char mode)
{
    std::string availableModes = "itkol";
    return availableModes.find(mode) != std::string::npos;
}

void Channel::unsetMode(char mode, User *user, Command *cmd)
{
    size_t modePosition = _modes.find(mode);
    if (modePosition != std::string::npos)
    {
        _modes.erase(modePosition, 1);
        cmd->SendGroupedMsg(_users, UNSET_CHANEL_MODE(user->GetNickname(), user->GetUsername(), cmd->GetCmdName(), _name, mode));
    }
}

void Channel::unsetPassword()
{
    _password = "";
}

void Channel::unsetOperator(std::stack<std::string> *modeParams, Server *server, Command *cmd, User *user)
{
    if (modeParams->top().empty())
    {
        return;
    }

    User *toDel = server->GetUserByNickname(modeParams->top());

    if (toDel->GetNickname() == _founder)
    {
        cmd->SendOneMsg(user, ERR_NOPRIVILEGES(user->GetNickname(), _name));
        return;
    }

    DelOper(toDel);
    cmd->SendGroupedMsg(_users, UNSET_OPER(user->GetNickname(), user->GetUsername(), cmd->GetCmdName(), _name, 'o', modeParams->top()));
    modeParams->pop();
}

void Channel::unsetLimit()
{
    _limit = INT_MAX;
}

bool Channel::HasUser(User* user)
{
    if (user == NULL)
        return false;

    std::vector<User*>::iterator it = this->_users.begin();
    std::vector<User*>::iterator ite = this->_users.end(); 

    while (it != ite)
    {
        User* usertmp = *it;

        if (usertmp == NULL)
        {
            it++;
            continue;
        }

        if (usertmp->GetNickname() == user->GetNickname())
            return true;
        it++;
    }

    return false;
}

void	Channel::RemoveUser(User *toRemove)
{
    if (toRemove == NULL)
        return;

    for (std::vector<User *>::iterator it = this->_users.begin(); it != this->_users.end(); )
    {
        if ((*it) != NULL && toRemove->GetNickname() == (*it)->GetNickname())
        {
            this->DelOper(*it);
            it = this->_users.erase(it);
        }
        else
            ++it;
    }

    std::vector<User *>::iterator it2 = this->_users.begin();
    if (*it2 != NULL)
        this->AddOper(*it2);
}
