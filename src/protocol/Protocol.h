/*****************************************************************//**
 * \file   Protocol.h
 * \brief  Protocol Includes for Serverside
 *
 * \author Iridescence - Nathan Bourgeois <iridescentrosesfall@gmail.com>
 * \date   November 2020
 *********************************************************************/
#pragma once

constexpr auto PROTOCOL_VERSION = 1;

constexpr auto SERVER_VERSION_MAJOR = 0;
constexpr auto SERVER_VERSION_MINOR = 1;

constexpr auto SERVER_VERSION_REVISION = 1;

#include "ClientProtocol.h"
#include "ServerProtocol.h"