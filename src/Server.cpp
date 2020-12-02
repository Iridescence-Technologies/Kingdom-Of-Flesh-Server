#include "Server.h"

Server::Server(u16 port) {
	Network::g_NetworkDriver.Init();

	running = true;
	socket = std::make_unique<Network::ServerSocket>(port);

	//Setup world
}

Server::~Server() {
	socket->Close();
	Network::g_NetworkDriver.Cleanup();
}

void Server::collectDead() {
	for (auto i = 0; i < clients.size(); i++) {
		if (clients[i]->isClosed()) {
			clients[i] = nullptr;
		}
	}

	std::vector<std::shared_ptr<Client>> temp;
	for (auto i = 0; i < clients.size(); i++) {
		if (clients[i] != nullptr) {
			temp.push_back(std::move(clients[i]));
		}
	}

	clients.clear();
	for (auto& cl : temp) {
		clients.push_back(std::move(cl));
	}
}

std::shared_ptr<Client> cl;
void Server::listen() {
	//Wait for new connection
	auto conn = socket->ListenState();

	//Create new ptr
	auto cl = std::make_shared<Client>(conn);

	//Start the thread
	cl->start();

	//Move the ptr to the list
	clients.push_back(std::move(cl));
	
	//cl is nullptr

	//Garbage collect
	collectDead();
}