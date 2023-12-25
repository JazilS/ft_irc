# include "irc.hpp"

User::User() : _isAuth(false), _isValid(false)
{
	return ;
}

User::~User()
{
	return ;
}

void User::LeaveChannel(Channel* toLeave)
{
    std::vector<Channel*>::iterator it = _channels.begin();

    while (it != _channels.end())
    {
        if ((*it)->GetName() == toLeave->GetName())
        {
            it = _channels.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void	User::JoinChannel(Channel *toJoin)
{
	this->_channels.push_back(toJoin);
	return ;
}

void	User::SetUsername(std::string username)
{
	this->_username = username;
	return ;
}

void	User::SetHostname(std::string hostname)
{
	this->_hostname = hostname;
	return ;
}

void	User::SetServername(std::string servername)
{
	this->_servername = servername;
	return ;
}

void User::SetRealname(std::vector<std::string> *paramCpy)
{
    if (paramCpy->size() < 4) // Vérifier si le vecteur a au moins 4 éléments
    {
        // Gérer l'erreur ou retourner si nécessaire
        return;
    }

    std::string forname = paramCpy->at(3); // le real name est le 3eme arg de NICK
    std::string realname = forname;
    size_t i = 4;

    if (realname.size() > 1 && realname.at(1) == ':') // Vérifier si la chaîne a au moins 2 caractères avant de vérifier le caractère à l'index 1
    {
        realname.erase(0, 1); // Supprimer les deux points
    }

    while (i < paramCpy->size())
    {
        realname += paramCpy->at(i); // Ajouter tous les noms qui suivent
        i++;                        // Car il est possible d'en avoir 1 ou plus de 2
    }

    this->_realname = realname;
}
bool	User::IsAvailableNickname(std::string nickname, Server *server)
{
	std::map<int, User*>::iterator	it = server->GetUsers().begin();
	std::map<int, User*>::iterator	ite = server->GetUsers().end();

	while (it != ite)
	{
		if (nickname == it->second->GetNickname())
			return (false);
		it++;
	}
	return (true);
}

void		User::SetNickname(std::string nickname, Server *server)
{
	if (IsAvailableNickname(nickname, server) == true)
	{
		this->_nickname = nickname;
	}
	else
	{ 
		this->_nickname = "";
	}
	return ;
}

void	User::SetFd(int fd)
{
	this->_fd = fd;
	return ;
}

void	User::SetValidity(bool status)
{
	this->_isValid = status;
}

void	User::SetAuth(bool status)
{
	this->_isAuth = status;
}

bool	User::GetValidity()
{
	return (this->_isValid);
}

bool	User::GetAuth()
{
	return (this->_isAuth);
}

int		User::GetFd()
{
	return (this->_fd);
}

std::string		User::GetNickname()
{
	return (this->_nickname);
}

std::string	User::GetUsername()
{
	return (this->_username);
}

std::string	User::GetHostname()
{
	return (this->_hostname);
}

std::string	User::GetServername()
{
	return (this->_servername);
}

std::string	User::GetRealname()
{
	return (this->_realname);
}

std::vector<Channel *>	&User::GetChannels()
{
	return (this->_channels);
}
