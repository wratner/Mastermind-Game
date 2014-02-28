William Ratner
wratner2
CS 438

Mastermind README

To start the server, enter the following in the terminal:

./server <port number>

The server must be started first.

When the server is started, you will need to either type "random" to generate 
and random 4 digit number or you need to type in the 4 digit number manually. 

To start the client, enter the following in the terminal:

./client <hostname> <port number>

Enter your guesses based on the hints you recieve. 

If you guess correctly within 8 guesses, you will recieve a message
telling you that you won and the client will disconnect from the server. 

If you use up all 8 guesses, you will recieve a message telling you that
you lost and the client will disconnect from the server. 