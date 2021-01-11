#include "Client.h"
#include "protocol/Protocol.h"
#include <zlib/zlib.h>

Client::Client(Network::Connection* con) : safe_conn(*con) {
	conn = std::unique_ptr<Network::Connection>{ con };
	connected = true;
	compression = true;
}

Client::~Client() {
#ifndef SINGLE_THREADED
	thr.join();
#endif
}

auto Client::receivePacket() -> bool {
	auto pack = safe_conn->Recv(false);

	if (pack != nullptr) {
		receivedQueue->emplace(pack);
		return true;
	}

	return false;
}

auto Client::addPacket(Network::PacketOut* pack) -> void {
	packetQueue->emplace(pack);
}

auto Client::sendPacket() -> void {
	size_t len = packetQueue->size();

	for (int i = 0; i < len; i++) {
		size_t packetLength;
		packetLength = packetQueue->front()->buffer->GetUsedSpace() + 1;

		Network::ByteBuffer* bbuf = new Network::ByteBuffer(packetLength + 5); //512 KB

		if (compression) {
			if (packetLength > 256) {
				bbuf->WriteVarInt32(static_cast<u32>(packetLength));


				Network::ByteBuffer* bbuf2 = new Network::ByteBuffer(packetLength + 5);
				bbuf2->WriteBEUInt8(static_cast<u8>(packetQueue->front()->ID));
				for (int i = 0; i < packetQueue->front()->buffer->GetUsedSpace(); i++) {
					uint8_t temp;
					packetQueue->front()->buffer->ReadBEUInt8(temp);
					bbuf2->WriteBEUInt8(temp);
				}

				//COMPRESS DATA!
				u8* byteBuffer = new u8[200 KiB];

				uLongf csize = compressBound(static_cast<uLong>(packetLength));
				if (csize > 200 KiB) {
					throw std::runtime_error("FATAL! COMPRESSED SIZE TOO LARGE: " + std::to_string(csize));
				}

				int32_t stat = compress2((Bytef*)byteBuffer, &csize, (const Bytef*)bbuf2->m_Buffer, static_cast<uLong>(bbuf2->GetUsedSpace()), Z_DEFAULT_COMPRESSION);

				if (stat != Z_OK) {
					throw std::runtime_error("FAILED TO COMPRESS: " + std::to_string(stat));
				}

				for (size_t i = 0; i < csize; i++) {
					bbuf->WriteBEUInt8(byteBuffer[i]);
				}

				delete[] byteBuffer;

			}
			else {
				bbuf->WriteVarInt32(0);
				bbuf->WriteBEUInt8(static_cast<u8>(packetQueue->front()->ID));

				packetQueue->front()->buffer->ResetRead();
				//Add body
				for (int i = 0; i < packetQueue->front()->buffer->GetUsedSpace(); i++) {
					uint8_t temp;
					packetQueue->front()->buffer->ReadBEUInt8(temp);
					bbuf->WriteBEUInt8(temp);
				}
			}

			Network::ByteBuffer* bbuf2 = new Network::ByteBuffer(bbuf->GetUsedSpace() + 5);
			bbuf2->WriteVarInt32(static_cast<u32>(bbuf->GetUsedSpace()));

			for (int i = 0; i < bbuf->GetUsedSpace(); i++) {
				uint8_t temp;
				bbuf->ReadBEUInt8(temp);
				bbuf2->WriteBEUInt8(temp);
			}

			//Send over socket
			safe_conn->Send(bbuf2->GetUsedSpace(), bbuf2->m_Buffer);

			delete bbuf2;
		}
		else {
			//Header
			bbuf->WriteVarInt32(static_cast<u32>(packetLength));

			bbuf->WriteBEUInt8(static_cast<u8>(packetQueue->front()->ID));

			packetQueue->front()->buffer->ResetRead();
			//Add body
			for (int i = 0; i < packetQueue->front()->buffer->GetUsedSpace(); i++) {
				uint8_t temp;
				packetQueue->front()->buffer->ReadBEUInt8(temp);
				bbuf->WriteBEUInt8(temp);
			}

			//Send over socket
			safe_conn->Send(bbuf->GetUsedSpace(), bbuf->m_Buffer);
		}

		delete bbuf;
		delete packetQueue->front()->buffer;
		delete packetQueue->front();
		packetQueue->pop();
	}
}

#include <iostream>
auto Client::handlePackets() -> void {

	size_t len = receivedQueue->size();
	for (size_t i = 0; i < len; i++) {
		auto packet = receivedQueue->front();
		receivedQueue->pop();

		switch (static_cast<cbPacketName>(packet->ID)) {
		case cbPacketName::Disconnect: {
			Utilities::app_Logger->debug("Client Sent Disconnect!");
			connected = false;

			break;
		}

		case cbPacketName::Handshake: {
			auto data = decodeSbHandshake(packet);

			Utilities::app_Logger->trace("User: "  + data.username + " is trying to join!");
			Utilities::app_Logger->trace("PROTOCOL ID: " + std::to_string(data.protocolID));

			if (data.protocolID != PROTOCOL_VERSION) {
				Utilities::app_Logger->warn("Outdated Protocol ID!");
				connected = false;

				auto p = makeCbDisconnect("VERSION TOO OLD!");
				addPacket(p.release());
				sendPacket();
			}
			else {
				auto p = makeCbHandshake({ PROTOCOL_VERSION, SERVER_VERSION_MAJOR, SERVER_VERSION_MINOR, SERVER_VERSION_REVISION });
				addPacket(p.release());
				sendPacket();
			}
			
			break;
		}

		default: {
			Utilities::app_Logger->warn("UNKNOWN PACKET: " + std::to_string(packet->ID));

			break;
		}
		}

		delete packet;
	}
}


void Client::start() {
#ifndef SINGLE_THREADED
	thr = std::thread(Client::run, this);
#else
	Client::run(this);
#endif
}

auto Client::run(Client* self) -> void {

	//Loop
	while (self->safe_conn->isAlive() && self->connected) {

		//Receive up to 50 packets at once
		int packetCounter = 0;
		while (self->receivePacket() && packetCounter < 50) {
			packetCounter++;
		}

		self->handlePackets();

		self->sendPacket();

		//Sleep for a tick
		Platform::delayForMS(50);
	}

	self->connected = false;
}
