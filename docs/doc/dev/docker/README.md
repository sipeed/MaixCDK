MaixCDK docker building environment
=====

## About Docker and install Docker

Docker is a tool, here we use it to create a clean Ubuntu environment to build MaixCDK.
Use docker you don't need to install dependencies manually, just create a docker container and then all ready.

Docker install doc see [Docker official doc](https://docs.docker.com/engine/install/ubuntu/)

After installation, you can use `docker --version` to check if it is installed successfully.

## Pull from docker hub (recommended), or build by yourself

```shell
docker pull sipeed/maixcdk-builder
```

> Or you can build from Dockerfile by yourself.
>
> ```shell
> docker build --network=host -t maixcdk-builder .
> ```
> You can also add proxy by add args:`--network=host --build-arg http_proxy=http://127.0.0.1:8123 --build-arg https_proxy=http://127.0.0.1:8123`


## Create and run container

The upper step we got a docker system image, now we create a container to run this image.

```shell
docker run -it --network=host --hostname maixcdk-env --name maixcdk-env --env USER=$USER --env UID=`id -u` --env GID=`id -g` --env MAIXCDK_PATH=/home/${USER}/MaixCDK -v /home/${USER}/MaixCDK:/home/${USER}/MaixCDK sipeed/maixcdk-builder
```
> !! DO NOT add `/bin/bash` to the end of the command.
> `--network=host` means use the same network as host PC.
> `--hostname` means set hostname of container to `maixcdk-env`.
> `--name` means set container name to `maixcdk-env`, then you can use `docker` command to control it.
> Then assign `USER`(your user name of host PC), user id, group id to use the same as host PC's user info in container to avoid permission problem.
> You can use `-v host_dir:container_dir` to map your data to container, e.g. `-v /home/${USER}/MaixCDK:/home/${USER}/MaixCDK -v /home/${USER}/projects:/home/${USER}/projects`.
> It's recommended to map your directories to the container's same directory, this is useful to see logs.
> `--env MAIXCDK_PATH=/home/${USER}/MaixCDK` arg to set `MAIXCDK_PATH` environment variable, so you can compile your project anywhere in container.
> `sipeed/maixcdk-builder` is the image name, if you build by yourself, use `maixcdk-builder` instead.


Then you can use shell command in container.
The user's password is `maixcdk` by default, if you want to change it, use `docker exec -it maixcdk-env passwd` to change it.

## Build MaixCDK projects or examples

When step into container, you can use shell command to build according to the MaixCDK document.

```shell
cd /MaixCDK/examples/hello_world
maixcdk build
maixcdk run

cd ~/projects/my_project
maixcdk build
maixcdk run
```

## Stop container

```shell
docker stop maixcdk-env
```

## RUN container

```shell
docker start maixcdk-env
docker attach maixcdk-env
```


## Execute command through host shell

```shell
docker exec -it --user $USER maixcdk-env "cd /MaixCDK/examples/hello_world && maixcdk build && maixcdk run"
```

```shell
docker exec -it --user $USER maixcdk-env  /bin/bash
```

## Remove container


```shell
docker stop maixcdk-env
docker rm maixcdk-env
```

