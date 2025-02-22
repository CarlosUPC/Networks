# BattleShip Royale
BattleShip Royale is an amazing multiplayer game where you pilot a spaceship and have to defeat all the foes you will encounter in order to keep alive as long as possible.

## Members

* Carlos Peña Hernando [CarlosUPC](https://github.com/CarlosUPC)
* Alexandru Cercel Mihai [AlexandruC5](https://github.com/AlexandruC5)


## Description

Run a server if there is not one created. Once the server is created you can join as a client selecting the proper adress and port.
Besides you can insert a name among a type of ship and you will be ready to enter the battle royale game.

Start destroying the foes you will encounter by shooting with your basic laser attack. You will be able to activate your ultimate laser attack when
his cooldown is done that would be useful in situations when you could be surrounded by several enemies.

Keep your spaceship alive as long as you can while destroying other ships to become the most skilled player in the ranking list of the game. 



## Instructions

   * A: Rotate left
   * D: Rotate right
   * Down Arrow: Move forward
   * Left Arrow: Basic shoot
   * Right Arrow: Ultimate shoot


## Technical Features

### Game accepts client connections 

* **Contributor**: Alexandru
* **Completeness degree**: Completely Archived
* **Result accomplished**: The game accepts clients and assignes them an ID that is handled by the server then assignes them the corresponding data.

### Game handles join/leave events
* **Contributor**: Alexandru
* **Completeness degree**: Completely Archived
* **Result accomplished**: Clients can disconnect and connect at freewill. At the beginning we had an issue related with replication manager since the clients don't 
replicate properly in the create action when they were disconnected and reconnected again. It was solved declaring the manager at OnStart() function of the client Network module


 ### World state replication system 
  * **Contributor**: Carlos 
  * **Completeness degree**: Archived with known issue
  * **Result accomplished**: the world state replication system uses a map which stores a network id with his proper action (Create, Update, Input , Destroy). The system works 
        well and replicates the current state of the server to its connected clients but i realized there is an issue when some values are modified like the life or kills in
        Behaviour code and they have to be replicated to the clients. Sometimes it happens that replication of that changed variables is updated correctly to all the clients but         often is only well replicated within the client application. This issue is fixed sending new packet type with that changed value from server to appropriate client. It           works perfectly but i think is not a good practise because of packet dropping. 
         
 ### Redundant sending of input packets 
 * **Contributor**: Carlos
 * **Completeness degree**: Completely Archived
 * **Result accomplished**:  As we deal with techniques to compensate for lag, and bandwidth usage is a real concern, we send input notification data from server with every                                    replication packet. That means a slower response to have all the clients updated but we optimize it in terms of bandwidth streaming. My first approach was sending the next expected input sequence after each replication packet so each client had to read all the previous replication data to reach the redudancy input so 
i improved the replication system creating a new action called input as a notification that stores that next sequence input number. This decrease a litle bit tha latency generated by the process of read unnecessary data and update the inputdatafront in order to have space for more input controllers.
 
 ### Delivery Manager
 * **Contributor**: Carlos
 * **Completeness degree**: Completely Archived
 * **Result accomplished**: Delivery manager takes care of all packet received from server to all connected clients. It just send a sequence number that is compared by client delivery manager sequence number so if the values are not the same the client doesn't process the read of that packet. That happens in every replication interval within a replication packet with a heap allocated delegate that has a replication server reference to resend the current state of the server according to the missed actions if any packet was not received to the clients. With packet drop checkbox activated, it's perceived a loss of data but delegate works well in order to resend the proper state when there is a failure.
 
 
 ### Client-side prediction with server reconciliation
 * **Contributor**: Carlos
 * **Completeness degree**: Completely Archived
 * **Result accomplished**: The client-side prediction is performed after the client recieves the replication packet with his last processed input updated from the server state. After
 received it, we iterate the input array in case of inputdataback was incremented, so there are inputs which need to be re-applied since they are not yet processed by the server in order to eliminate visible desynchronization issue.
 
 ### Entity Interpolation 
 * **Contributor**: Carlos
 * **Completeness degree**: Completely Archived with known issue
 * **Result accomplished**: The interpolation works well and there is no kind of flickering or teleportation of positions and angle. I don't know if I'm crazy but it gives me the feeling that it is not as immediate as it looks on the server, maybe it is because all the input is processed on the server and there is a small latency since we first send the inputs from the client, the server runs them and then send the replication to update the client. Also it can be by the ratio/percentage that is calculated in the local interpolation. You will see.
 
### Lag compensation
* **Contributor**: Alexandru
* **Completeness degree**: Tried but not archived. Tried but dissmies due that couldn't make it work
* **Result accomplished**: Tried to create a system that keeps a history of all recent player positions for one second so the system can estimate at what time the commando was created,
			   the math would be: CommandExeTime = CurrentServTime - Packet Latency - Client View 
			   This would move all other players back to where they were at the command execution time, then the user command would be exectued and finaly after the command is
			   processed they players would revert to their original position.       


## Gameplay Features

* Alexandru
	* Kill & Ultimate UI
	* Ranking list UI
	* Cooldown
* Carlos
	* Local lifebar
	* Ultimate shoot attack
	* sorting proxy clients for the ranking list

    




