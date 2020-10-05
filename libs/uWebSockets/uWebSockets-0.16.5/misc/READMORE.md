## FAQ
You can read the frequently asked questions [here](FAQ.md).

## Motivation and goals
µWebSockets is a simple to use yet thoroughly optimized implementation of HTTP and WebSockets.
It comes with built-in pub/sub support, HTTP routing, TLS 1.3, IPv6, permessage-deflate and is battle tested as one of the most popular implementations, reaching many end-users daily.
Unlike other "pub/sub brokers", µWS does not assume or push any particular protocol but only operates over standard WebSockets.

The implementation is header-only C++17, cross-platform and compiles down to a tiny binary of a handful kilobytes.
It depends on µSockets, which is a standard C project for Linux, macOS & Windows.

Performance wise you can expect to outperform, or equal, just about anything similar out there, that's the fundamental goal of the project.
I can show certain cases where µWS with SSL significantly outperforms Golang servers running non-SSL. You get the SSL for free in a sense, well, in this particular case at least.

Another goal of the project is minimalism, simplicity and elegance.
Design wise it follows an ExpressJS-like interface where you attach callbacks to different URL routes.
This way you can easily build complete REST/WebSocket services in a few lines of code.

The project is async only and runs local to one thread. You scale it as individual threads much like Node.js scales as individual processes. That is, the implementation only sees a single thread and is not thread-safe. There are simple ways to do threading via async delegates though, if you really need to.

## Compiling
µWebSockets is 100% standard header-only C++17 - it compiles on any platform. However, it depends on µSockets in all cases, which is platform-specific C code that runs on Linux, Windows and macOS.

There are a few compilation flags for µSockets (see its documentation), but common between µSockets and µWebSockets flags are as follows:

* LIBUS_NO_SSL - disable OpenSSL dependency/functionality for uSockets and uWebSockets builds
* UWS_NO_ZLIB - disable Zlib dependency/functionality for uWebSockets

## Node.js
V8 addon is developed over at https://github.com/uNetworking/uWebSockets.js.

## User manual

### uWS::App & uWS::SSLApp
You begin your journey by constructing an "App". Either an SSL-app or a regular TCP-only App. The uWS::SSLApp constructor takes a struct holding SSL options such as cert and key. Interfaces for both apps are identical, so let's call them both "App" from now on.

Apps follow the builder pattern, member functions return the App so that you can chain calls.

### App.get, post, put, [...] and any routes
You attach behavior to "URL routes". A lambda is paired with a "method" (Http method that is) and a pattern (the URL matching pattern).

Methods are many, but most common are probably get & post. They all have the same signature, let's look at one example:

```c++
uWS::App().get("/hello", [](auto *res, auto *req) {
    res->end("Hello World!");
});
```

Important for all routes is that "req", the `uWS::HttpRequest *` dies with return. In other words, req is stack allocated so don't keep it in your pocket.

res, the `uWS::HttpResponse<SSL> *` will be alive and accessible until either its .onAborted callback emits, or you've responded to the request via res.end or res.tryEnd.

In other words, you either respond to the request immediately and return, or you attach lambdas to the res (which may hold captured data), and respond later on in some other async callback.

Data that you capture in a res follows RAII and is move-only so you can properly move-in for instance std::string buffers that you may use to, for instance, buffer upp streaming POST data. It's pretty cool, check out mutable lambdas with move-only captures.

The "any" route will match any method.

#### Pattern matching
* Static matches, think "/hello/this/is/static".
* Wildcard matches, think "/hello/*".
* Parameter matches, think "/candy/:kind", where value of :kind is retrieved by req.getParameter(0).

Routes are matched in order of insert. However, keep in mind routes are stored in a tree, where every URL segment is a node added in order. This means you should **add shorter routes first** in order to not be surprised with the matching order, caused by longer routes added before.

Example:

* Adding /a/:b/c as 1
* Adding /a/b as 2
* Adding /a/:b as 3

Now, even though route 2 looks to be prioritized before route 3, that's not the case. This because the longer route 1 already established all nodes of route 3, and thus already "tainted" the route order. Therefore make sure to add shorter routes before longer ones so that it becomes easier to spot their order.

#### Streaming data
You should never call res.end(huge buffer). res.end guarantees sending so backpressure will probably spike. Instead you should use res.tryEnd to stream huge data part by part. Use in combination with res.onWritable and res.onAborted callbacks.

Tip: Check out the JavaScript project, it has many useful examples of async streaming of huge data.

### The App.ws route
WebSocket "routes" are registered similarly, but not identically.

Every websocket route has the same pattern and pattern matching as for Http, but instead of one single callback you have a whole set of them, here's an example:

```c++
uWS::App().ws<PerSocketData>("/*", {
    /* Settings */
    .compression = uWS::SHARED_COMPRESSOR,
    .maxPayloadLength = 16 * 1024,
    .idleTimeout = 10,
    /* Handlers */
    .open = [](auto *ws, auto *req) {
        /* Here you can use req just like as for Http */
    },
    .message = [](auto *ws, std::string_view message, uWS::OpCode opCode) {
        ws->send(message, opCode);
    },
    .drain = [](auto *ws) {
        /* Check getBufferedAmount here */
    },
    .ping = [](auto *ws) {

    },
    .pong = [](auto *ws) {

    },
    .close = [](auto *ws, int code, std::string_view message) {

    }
});
```

WebSocket routes specify a user data type that should be used to keep per-websocket data. Many times people tend to attach user data
which should belong to the websocket by putting the pointer and the user data in a std::map. That's wrong! Don't do that!

#### Use the WebSocket.getUserData() feature
You should use the provided user data feature to store and attach any per-socket user data. Going from user data to WebSocket is possible if you make your user data hold a pointer to WebSocket, and hook things up in the WebSocket open handler. Your user data memory is valid while your WebSocket is.

If you want to create something more elaborate you could have the user data hold a pointer to some dynamically allocated memory block that keeps a boolean whether the WebSocket is still valid or not. Sky is the limit here, you should never need any std::map for this.

#### WebSockets are valid from open to close
All given WebSocket pointers are guaranteed to live from open event (where you got your WebSocket) until close event is called. So is the user data memory. One open event will always end in exactly one close event, they are 1-to-1 and will always be balanced no matter what. Use them to drive your RAII data types, they can be seen as constructor and destructor.

Message events will never emit outside of open/close. Calling WebSocket.close or WebSocket.end will immediately call the close handler.

#### Backpressure in websockets
Similarly to for Http, methods such as ws.send(...) can cause backpressure. Make sure to check ws.getBufferedAmount() before sending, and check the return value of ws.send before sending any more data. WebSockets do not have .onWritable, but instead make use of the .drain handler of the websocket route handler.

Inside of .drain event you should check ws.getBufferedAmount(), it might have drained, or even increased. Most likely drained but don't assume that it has, .drain event is only a hint that it has changed.

#### Settings
Compression (permessage-deflate) has three options, uWS::DISABLED, uWS::SHARED_COMPRESSOR and uWS::DEDICATED_COMPRESSOR. Disabled and shared options require no memory, while dedicated compressor requires somewhere close to 300kb per socket, a very significant cost.

Compressing using shared means that every WebSocket message is an isolated compression stream, it does not have a sliding compression window, kept between multiple send calls.

Shared compression is my personal favorite, since it doesn't change memory usage while still provide decent compression, especially for larger messages.

* idleTimeout is roughly the amount of seconds that may pass between messages. Being idle for more than this, and the connection is severed. This means you should make your clients send small ping messages every now and then, to keep the connection alive. You can also make the server send ping messages but I would definitely put that labor on the client side.

### Listening on a port
Once you have defined your routes and their behavior, it is time to start listening for new connections. You do this by calling

```c++
App.listen(port, [](auto *listenSocket) {
    /* listenSocket is either nullptr or us_listen_socket */
})
```

Cancelling listenning is done with the uSockets function call `us_listen_socket_close`.

### App.run and fallthrough
When you are done and want to enter the event loop, you call, once and only once, App.run.
This will block the calling thread until "fallthrough". The event loop will block until no more async work is scheduled, just like for Node.js.

Many users ask how they should stop the event loop. That's not how it is done, you never stop it, you let it fall through. By closing all sockets, stopping the listen socket, removing any timers, etc, the loop will automatically cause App.run to return gracefully, with no memory leaks.

Because the App itself is under RAII control, once the blocking .run call returns and the App goes out of scope, all memory will gracefully be deleted.

### Putting it all toghether

```c++
int main() {
    uWS::App().get("/*", [](auto *res, auto *req) {
        res->end("Hello World!");
    }).listen(9001, [](auto *listenSocket) {
        if (listenSocket) {
            std::cout << "Listening for connections..." << std::endl;
        }
    }).run();

    std::cout << "Shoot! We failed to listen and the App fell through, exiting now!" << std::endl;
}
```

### Scaling up

One event-loop per thread, isolated and without shared data. That's the design here. Just like Node.js, but instead of per-process, it's per thread (well, obviously you can do it per-process also).

If you want to, you can simply take the previous example, put it inside of a few `std::thread` and listen to separate ports, or share the same port (works on Linux). More features like these will probably come, such as master/slave set-ups but it really isn't that hard to understand the concept - keep things isolated and spawn multiple instances of whatever code you have.
