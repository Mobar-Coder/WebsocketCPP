# WebsocketCpp
A Websocket Library for modern C++ (17) based on libwebsockets.

## Overview
The complete interface is similar to the Java-TCP Interface.
One major difference is that all communication is managed asynchronous
using callback-functors.

### Client
Only one class, `websocketcpp::WebSocketClient`. Create one per connection.

### Server
For every server create an instance of `websocketcpp::WebSocketServer`
for every connected client, this will create 
a new `websocketcpp::Connection`.

### Utility
The `Listener<Args...>` class implements a generic message system 
with arbitrary many receivers and usually one sender
`Args...` specifies the type of message. Clients can
subscribe by calling `Listener::subscribe`, the sender can send
a message by calling the `operator()` with the argument types
as specified by `Args...`.

## Installing
### Libwebsockets
First you need to install libssl, on ubuntu this can be done by running

```
sudo apt install libssl-dev
```
next clone libwebsockets from GitHub by running

```
git clone https://github.com/warmcat/libwebsockets.git
```

change into the cloned directory (`cd libwebsockets`) and create the Makefile by running:

```
cmake .
```

next compile the project by running the makefile

```
make
```

finally install the library by running

```
sudo make install
```

To be able to use the library without rebooting reload the linker cache by running

```
sudo ldconfig
```

### Installing WebsocketCpp
In the root directory of the  project run cmake to generate a Makefile by running
```
cmake .
```
next compile the program by running
```
make
```
finally install the program by running
```
sudo make install
```
the library can now be included using

```
#include <WebsocketCpp/Filename>
```
with `Filename` beeing one of the header files without their relative path.

and linked using

```
-lWebsocketCpp
```
