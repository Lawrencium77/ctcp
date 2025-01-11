# ctcp

## How to Test

To build the Docker image and launch the server and client containers, do

```bash
tools/test/server.sh
```

In a separate terminal, then run the client process:

```bash
tools/test/client.sh
```

Note that, for a packet being transferred from client to server, the chance of successful transmission is:

```
(server-side packet loss) x (client-side packet loss)
```

To clear up, simply kill the `server.sh` process. This will call `docker compose down` to stop and remove running containers.