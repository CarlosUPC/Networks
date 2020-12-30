#pragma once

// TODO(you): World state replication lab session
class ModuleNetworkingClient;
class ReplicationManagerClient
{
public:
	void read(const InputMemoryStream& packet, ModuleNetworkingClient* client);
};
