This was a school project, and some of the code was given to me.
what i wrote is: the client class, incoder-decode class (server-> src/main...-> api -> massagingEncoderDecoderImpl), the protocol class (server-> src/main...-> api -> stompProtocolImpl),
the client data class(server-> src/main...-> api -> clientData), connections class (server-> src/main...-> srv -> connectionImpl) and the main functions of the server client.
beside those, i was assigned to change the reactor, base server, blocking connection handler and the non-blocking connection handler.


How to Use:

Server (Java):
Build the server by running: mvn compile
Run the server with: mvn exec:java -Dexec.mainClass="bgu.spl.net.impl.stomp.StompServer" -Dexec.args="<port> <tpc/reactor>"
Example: mvn exec:java -Dexec.mainClass="bgu.spl.net.impl.stomp.StompServer" -Dexec.args="7777 tpc"

Client (C++):
Build the client using: make
Run the client with: ./bin/client
Use the following commands inside the client:

login <host:port> <username> <password>
Connect to the server.

join <channel>
Subscribe to a topic (e.g., fire_dept).

exit <channel>
Unsubscribe from a topic.

report <file.json>
Read emergency events from a JSON file and send them to the appropriate channel.

summary <channel> <user> <output_file>
Save all events reported by a specific user in a channel to a file, in a readable summary format.

logout
Disconnect from the server (sends a DISCONNECT frame and waits for a RECEIPT).


Additional Info:
The server supports both Thread-per-client and Reactor modes.
The client uses multithreading to handle both server messages and user input.
Communication is done using standard STOMP frames (CONNECT, SUBSCRIBE, SEND, etc.).
All emergency events must follow the required JSON format and include event name, city, time, status, and description.
Project is designed to compile and run on Linux lab machines or Docker environments.





