#include "Networks.h"

// TODO(you): Reliability on top of UDP lab session

//#include "DeliveryManager.h"


Delivery* DeliveryManager::writeSequenceNumber(OutputMemoryStream& packet)
{
	packet << nextOutSeqNumber;

	Delivery* delivery = new Delivery();
	delivery->sequenceNumber = nextOutSeqNumber;
	delivery->dispatchTime = Time.time;

	delivery->delegate = new DeliveryDelegateDummy();

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
				//pendingDeliveries.erase(it);
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
}

void DeliveryDelegateDummy::OnDeliverySuccess(DeliveryManager* delManager)
{
	//Packet delivered
}

void DeliveryDelegateDummy::OnDeliveryFailure(DeliveryManager* delManager)
{
	//Packet dropped
}