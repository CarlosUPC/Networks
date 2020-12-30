# BattleShip Royale
BattleShip Royale is an amazing multiplayer game where you pilot a spaceship and have to defeat all the foes you will encounter in order to keep alive as long as possible.

## Members

* Carlos Peña Hernando
* Alexandru Cercel Mihai


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
    
* Alexandru:

    * game accepts client connections
    * game handles join/leave events
* Carlos:

## Technical Features

 ### World state replication system 
  * **Contributor**: Carlos 
  * **Completeness degree**: Archived with known issue
  * **Result accomplished**: the world state replication system uses a map which stores a network id with his proper action (Create, Update, Input , Destroy). The system works 
        well and replicates the current state of the server to its connected clients but i realized there is an issue when some values are modified like the life or kills in
        Behaviour code and they have to be replicated to the clients. Sometimes it happens that replication of that changed variables is updated correctly to all the clients but         often is only well replicated within the client application. This issue is fixed sending new packet type with that changed value from server to appropriate client. It           works perfectly but i think is not a good practise because of packet dropping. 
         
 ### Redundant sending of input packets 
 * **Contributor**: Carlos
 * **Completeness degree**: Completely Archived
 * **Result accomplished**: 
 
 ### Delivery Manager - Completely Archived
 * **Contributor**: Carlos
 * **Completeness degree**: Completely Archived
 * **Result accomplished**: 
 
 ### Client-side prediction with server reconciliation
 * **Contributor**: Carlos
 * **Completeness degree**: Completely Archived
 * **Result accomplished**: 
 
 ### Entity Interpolation 
 * **Contributor**: Carlos
 * **Completeness degree**: Completely Archived
 * **Result accomplished**: 
    
    

## Bugs / Issues

  * A we deal with techniques to compensate for lag, and bandwidth usage is a real concern, we send input notification data from server with every replication packet. 
  That means a slower response to have all the clients updated.
  * There is a loss of input data with high latency simulation
    




