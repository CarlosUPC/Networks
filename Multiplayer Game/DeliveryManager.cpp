#include "Networks.h"
#include "DeliveryManager.h"

// TODO(you): Reliability on top of UDP lab session

//#include "DeliveryManager.h"


DeliveryManager::DeliveryManager()
{
}

DeliveryManager::~DeliveryManager()
{
	clear();
}

Delivery* DeliveryManager::writeSequenceNumber(OutputMemoryStream& packet)
{
	packet << nextOutSeqNumber;

	Delivery* delivery = new Delivery();
	delivery->sequenceNumber = nextOutSeqNumber;
	delivery->dispatchTime = Time.time;

	//delivery->delegate = new DeliveryDelegateServer();

	pendingDeliveries.push_back(delivery);

	nextOutSeqNumber++;

	return delivery;
}

bool DeliveryManager::processSequenceNumber(const InputMemoryStream& packet)
{
	uint32 seqNumber;
	packet >> seqNumber;

	if (seqNumber >= nextInSeqNumber)
	{
		nextInSeqNumber = seqNumber + 1;
		pendingAcks.push_back(seqNumber);
		return true;
	}
	else
	{
		//pendingAcks.push_back(seqNumber);
		return false;
	}
}

bool DeliveryManager::hasSequenceNumberPendingAck() const
{
	return !pendingAcks.empty();
}

void DeliveryManager::writeSequenceNumbersPendingAck(OutputMemoryStream& packet)
{
	uint32 size = pendingAcks.size();
	packet << size;

	for (uint32 seqNumber : pendingAcks)
		packet << seqNumber;

	pendingAcks.clear();
}

void DeliveryManager::processAckdSequenceNumbers(const InputMemoryStream& packet)
{
	uint32 acksSize;
	packet >> acksSize;

	for (int i = 0; i < acksSize; ++i)
	{
		uint32 seqNumber;
		packet >> seqNumber;

		for (std::list<Delivery*>::iterator it = pendingDeliveries.begin(); it != pendingDeliveries.end(); ++it)
		{
			Delivery* delivery = *it;
			if (delivery->sequenceNumber == seqNumber)
			{
				if (delivery->delegate)
					delivery->delegate->OnDeliverySuccess(this);

				pendingDeliveries.remove(delivery);
				delete delivery->delegate;
				delete delivery;
				
				break;
			}
		}
	}
}

void DeliveryManager::processTimedOutPackets()
{
	
	std::list<std::list<Delivery*>::iterator> toDelete;

	for (std::list<Delivery*>::iterator it = pendingDeliveries.begin(); it != pendingDeliveries.end(); ++it)
	{
		if (Time.time - (*it)->dispatchTime >= PACKET_DELIVERY_TIMEOUT_SECONDS)
		{
			(*it)->delegate->OnDeliveryFailure(this);
			toDelete.push_back(it);
			delete (*it)->delegate;
			delete* it;
		}
	}

	for (auto mode : toDelete)
	{
		pendingDeliveries.erase(mode);
	}
}

void DeliveryManager::clear()
{
	for (Delivery* delivery : pendingDeliveries)
		delete delivery;

	pendingDeliveries.clear();
	pendingAcks.clear();

	nextInSeqNumber = 0;
	nextOutSeqNumber = 0;
}


DeliveryDelegateServer::DeliveryDelegateServer(ReplicationManagerServer* replicationServer) 
	: server(replicationServer), map(replicationServer->GetReplicationData())
{
	
	for (std::unordered_map<uint32, ReplicationCommand>::iterator it = map.begin(); it != map.end(); ++it)
	{
		commands.push_back(it->second);
	}
}

void DeliveryDelegateServer::OnDeliverySuccess(DeliveryManager* delManager)
{
	//Packet delivered
	for (std::vector<ReplicationCommand>::iterator it = commands.begin(); it != commands.end(); ++it)
	{
		switch ((*it).action)
		{
		case ReplicationAction::Create:
			
			break;
		case ReplicationAction::Update:
			
			break;
		case ReplicationAction::Input:
			
			break;
		case ReplicationAction::Destroy:
			
			break;
		default:
			break;
		}
	}
}

void DeliveryDelegateServer::OnDeliveryFailure(DeliveryManager* delManager)
{
	//Packet previously dropped. Re-update client state
	for (std::vector<ReplicationCommand>::iterator it = commands.begin(); it != commands.end(); ++it)
	{
		switch ((*it).action)
		{
		case ReplicationAction::Create:
			if(App->modLinkingContext->getNetworkGameObject((*it).networkId) != nullptr)
				server->create((*it).networkId);
			break;
		case ReplicationAction::Update:
			if (App->modLinkingContext->getNetworkGameObject((*it).networkId) != nullptr)
				server->update((*it).networkId);
			break;
		case ReplicationAction::Input:
			if (App->modLinkingContext->getNetworkGameObject((*it).networkId) != nullptr)
				server->input((*it).networkId, (*it).inputFrontData);
			break;
		case ReplicationAction::Destroy:
			if (App->modLinkingContext->getNetworkGameObject((*it).networkId) != nullptr)
				server->destroy((*it).networkId);
			break;
		default:
			break;
		}
	}
}