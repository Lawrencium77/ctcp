# ctcp

A C implementation of UDP and TCP protocols over IP.

### Current Status

* **UDP**: Single-threaded server and client implementation are working (see setup below).
* **TCP**: Not yet implemented.

## How To Use This Code

### Development

First, install the [Dev Containers VSCode extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers). This will allow you attach a VSCode instance to a Docker container possessing the project code. See the [VSCode docs](https://code.visualstudio.com/docs/devcontainers/containers) for more detail.

Once you have the extension installed, run `Dev Containers: Rebuild Container` in VSCode. This will build the Docker image, launch client and server 
containers, and attach a VSCode instance to the server container. Volume mounts are used to sync the project code between the host and both containers.

### Testing

Once both containers are running, you can test the client-server interaction by doing:

```bash
tools/test/set_conditions.sh
```

to set packet loss and corrupt  ion rates. Next, run

```bash
tools/test/daemon.sh
```

to launch the UDP daemon process. This handles the raw IP socket and implements UDP's two main services:

* Checksum validation
* Demultiplexing.

In a separate terminal, then run a UDP server process:

```bash
tools/test/server.sh
```

Then run a UDP client process to send data to the UDP server:

```bash
tools/test/client.sh
```

Finally, we can reset the network conditions by running:

```bash
tools/test/set_conditions.sh --reset
```

Note that, for a packet being transferred from client to server, the chance of successful transmission is:

```
(server-side packet loss) x (client-side packet loss)
```

and likewise for packet corruption. Note that packet corruption can cause packet loss even without explicit loss settings, since IP drops any packets with corrupted headers.