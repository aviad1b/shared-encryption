/*********************************************************************
 * \file   ConnectedClientHandler.cpp
 * \brief  Implementation of ConnectedClientHandler class.
 * 
 * \author aviad1b
 * \date   December 2025, Kislev 5786
 *********************************************************************/

#include "ConnectedClientHandler.hpp"

namespace senc::server
{
	ConnectedClientHandler::ConnectedClientHandler(utils::Socket& sock, const std::string& username)
		: _sock(sock), _username(username) { }

	void ConnectedClientHandler::loop()
	{
		// TODO: Implement
	}
}
