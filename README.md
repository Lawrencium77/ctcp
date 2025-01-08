# ctcp

## How to Use

To build the Docker image and run two containers, do

```bash
docker-compose up
```

In separate terminals, then do:

```bash
docker-compose exec server bash
```

```bash
docker-compose exec client bash
```

and launch the server and client respectively. To clear up, do

```bash
docker-compose down
```
