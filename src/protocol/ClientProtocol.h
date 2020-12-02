/*****************************************************************//**
 * \file   ClientBoundProtocol.h
 * \brief  Client Bound Packet Names and Structs
 *
 * \author Iridescence - Nathan Bourgeois <iridescentrosesfall@gmail.com>
 * \date   November 2020
 *********************************************************************/
#pragma once
#include <string>
#include "../Types.h"

enum class cbPacketName {
	Disconnect = 0x00,
	Handshake = 0xFF,
};

/**
 * Client-Bound Disconnect.
 *
 * No fields - causes immediate disconnection.
 */
struct CbDisconnect {
	std::string reason;
};

inline auto makeCbDisconnect(std::string reason) -> std::unique_ptr<Network::PacketOut>{
	auto res = std::make_unique<Network::PacketOut>(sizeof(CbDisconnect) + 64);

	res->ID = 0x00;
	res->buffer->WriteVarUTF8String(reason);

	return res;
}

/**
 * Client-Bound handshake.
 *
 * This sends the current protocol ID,
 * The server version major,
 * The server version minor,
 * The server version revision.
 */
struct CbHandshake {
	uint8_t protocolID;
	uint8_t verMaj;
	uint8_t verMin;
	uint8_t verRev;
};

inline auto makeCbHandshake(CbHandshake d) -> std::unique_ptr<Network::PacketOut> {
	auto res = std::make_unique<Network::PacketOut>(sizeof(CbHandshake));

	res->ID = 0xFF;
	res->buffer->WriteBEUInt8(d.protocolID);
	res->buffer->WriteBEUInt8(d.verMaj);
	res->buffer->WriteBEUInt8(d.verMin);
	res->buffer->WriteBEUInt8(d.verRev);

	return res;
}