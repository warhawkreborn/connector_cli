For the built-in Web Server I think the most important information would be the following:

    List of servers being broadcasted (for playing)
    List of servers being published (for hosting)
    IP addresses

Also it might be a good idea to add the following:

    Viewing log output
    Option to force refresh server list
    Option to force add server even if it is not detected
    Option to broadcast a server not in the official list (ip + port field)

As it will be the primary solution for people hosting, we might also add some more advanced features in the future:

    Log of incoming user connections
    Option to drop individual connections
    Option to block specific ips from connecting (white/black listing)
    Option to dump packets for each connection to individual files (if possible as a pcap)
    Option to automatically upload those to the server (opt in)

While the first 3 features reason should be obvious (spam protection) the last two have a pretty simple reason as well.
We would like to decipher the protocol to implement features like

    Username banning
    Statistics (per User/Total)
    Username + Password auth
    Cheat protection (stuff like reload hacks, wall hacks, etc)

However to do so we need to understand (and intercept the encryption of the protocol) and having captures of the data going back and forth would really help here. They of course require intercepting the traffic, but if the server is running 24/7 it should not be a problem to do so. That said it should of course be optional.
