/*****************************************************************//**
 * \file   main.cpp
 * \brief  The Main File
 *
 * \author Iridescence - Nathan Bourgeois <iridescentrosesfall@gmail.com>
 * \date   November 2020
 *********************************************************************/
#include <iostream>
#include <Utilities/Logger.h>
#include "Server.h"

using namespace Stardust;

inline auto setup_logger() -> void {
	Utilities::app_Logger->autoFlush = true;
	Utilities::app_Logger->toConsole = true;
	Utilities::detail::core_Logger->autoFlush = true;
	Utilities::detail::core_Logger->toConsole = true;

	Utilities::app_Logger->currentLevel = Utilities::LOGGER_LEVEL_TRACE;
	Utilities::detail::core_Logger->currentLevel = Utilities::LOGGER_LEVEL_TRACE;
	Utilities::app_Logger->info("KOF Server Start!");
}

auto main() -> int {
	Platform::initPlatform("headless");
	setup_logger();

	auto srv = std::make_unique<Server>('K' * 'O' + 'F');
	while (srv->isRunning()) {
		srv->listen();

		Platform::platformUpdate();
	}

	Platform::exitPlatform();
	return 0;
}