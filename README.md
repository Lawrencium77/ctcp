# ctcp

## How To Use This Code

### Development

First, install the [Dev Containers VSCode extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers). This will allow you attach a VSCode instance to a Docker container possessing the project code. See the [VSCode docs](https://code.visualstudio.com/docs/devcontainers/containers) for more detail.

Once you have the extension installed, run `Dev Containers: Rebuild Container` in VSCode. This will build the Docker image, launch client and server 
containers, and attach a VSCode instance to the server container. Volume mounts are used to sync the project code between the host and both containers.

### Testing

Once both containers are running, you can test the client-server interaction by doing:

```bash
tools/test/server.sh
```

to launch the server process. In a separate terminal, run the client process:

```bash
tools/test/client.sh
```

Note that, for a packet being transferred from client to server, the chance of successful transmission is:

```
(server-side packet loss) x (client-side packet loss)
```

and likewise for packet corruption. 