# ctcp

## How to Use

To build the Docker image and run two containers, do

```bash
docker compose up
```

In separate terminals, then do:

```bash
docker compose exec server tc qdisc add dev eth0 root netem loss 50%   # Adjust packet loss as needed
docker compose exec server tc qdisc add dev eth0 root netem corrupt 50%   # Adjust packet corruption as needed
docker compose exec server bash
```

```bash
docker compose exec client tc qdisc add dev eth0 root netem loss 50%    # Adjust packet loss as you wish
docker compose exec client tc qdisc add dev eth0 root netem corrupt 50%   # Adjust packet corruption as needed
docker compose exec client bash
```

To launch the server process from inside the container, do:

```bash
build/server
```

To run the client, run `nslookup server` to obtain the IP address of the server container. Then do

```
build/server <ip_addr> <message>
```

Note that, for a packet being transferred from client to server, the chance of successful transmission is:

```
(server-side packet loss) x (client-side packet loss)
```

To clear up, do

```bash
docker compose down
```
