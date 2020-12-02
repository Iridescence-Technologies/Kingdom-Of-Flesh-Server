/*****************************************************************//**
 * \file   Server.h
 * \brief  The server main
 *
 * \author Iridescence - Nathan Bourgeois <iridescentrosesfall@gmail.com>
 * \date   November 2020
 *********************************************************************/
#pragma once
#include "Client.h"
#include <Network/NetworkDriver.h>
#include "Types.h"
#include <vector>
#include <memory>
//#include "world/World.h"

using namespace Stardust;

/**
 * Describes our server.
 * Simply a socket that handles new connections.
 * On new connection, it spawns a Client and splits it off into a thread.
 */
class Server : public NonCopyable, NonMovable {
	std::unique_ptr<Network::ServerSocket> socket;
	std::vector<std::shared_ptr<Client>> clients;
	bool running;

	//World* world;

	/**
	 * Garbage collects our old disconnected clients.
	 *
	 */
	void collectDead();
public:

	/**
	 * Starts the server.
	 *
	 * \param port - The server starting port.
	 */
	Server(u16 port);
	~Server();

	/**
	 * Listens for new connections
	 *
	 */
	auto listen() -> void;

	/**
	 * Is the server running?
	 *
	 * \return - Running Status
	 */
	inline auto isRunning() {
		return running;
	}
};