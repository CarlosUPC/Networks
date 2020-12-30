#pragma once

// TODO(you): World state replication lab session

enum class ReplicationAction
{
	None, Create, Update, Input, Destroy
};

struct ReplicationCommand
{
	ReplicationAction action;
	uint32 networkId;

	uint32 inputFrontData = 0u;
};

class ReplicationManagerServer
{
public:
	void create(uint32 networkId);
	void update(uint32 networkId);
	void input(uint32 networkId, uint32 data = 0u);
	void destroy(uint32 networkId);

	void write(OutputMemoryStream& packet);

	bool isEmpty() const; 

private:
	std::unordered_map<uint32, ReplicationCommand> map;

	//More members...
};