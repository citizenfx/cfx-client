#pragma once

namespace fx
{
	namespace ServerDecorators
	{
		const fwRefContainer<fx::GameServer>& WithEndPoints(const fwRefContainer<fx::GameServer>& server)
		{
			static std::shared_ptr<ConsoleCommand> cmd;

			server->OnAttached.Connect([=](fx::ServerInstanceBase* instance)
			{
				cmd = std::move(instance->AddCommand("endpoint_add_udp", [=](const std::string& endPoint)
				{
					// parse the endpoint to a peer address
					boost::optional<net::PeerAddress> peerAddress = net::PeerAddress::FromString(endPoint);

					// if a peer address is set
					if (peerAddress.is_initialized())
					{
						// create an ENet host
						ENetAddress addr = GetENetAddress(*peerAddress);
						ENetHost* host = enet_host_create(&addr, 64, 2, 0, 0);

						// ensure the host exists
						if (!host)
						{
							trace("Could not bind on %s - is this address valid and not already in use?\n", endPoint);
							return;
						}

						// register the global host
						g_hostInstances[host] = server.GetRef();

						server->hosts.push_back(fx::GameServer::THostPtr{ host });

						server->OnHostRegistered(host);
					}
				}));
			});

			return server;
		}
	}
}
