## TCP hole punch

Based on the article `https://bford.info/pub/net/p2pnat/index.html`, this repository is a `C` implementation of the concept explained in the article, more specifically the `4.4 Simultaneous TCP Open`. This repository is just a proof of concept and therefore is public domain.

## Compiling

### Linux

- With Makefile

```bash
make all
```

- Directly using gcc command line

```bash
gcc -g server.c -o server -pthread
gcc -g client.c -o client -pthread
```

## Testing

Start the Rendezvous server which will listen on port 7777 (defined in the code). To connect peers across different NAT's this server must have a public IP address.

```bash
./server
```

Connect two clients in the same server IP, they can be across different NAT's or in the same, both will work.

```bash
# the ip here must be the rendezvous server ip.
./client 192.168.1.2
```

After a few seconds the clients will print the message `"connected to peer"` and will begin listening for messages coming from each others `stdin` in another thread.