MaixCDK Docker 构建环境
=====

## 关于 Docker 和安装 Docker

Docker 是一种工具，我们在这里使用它来创建一个干净的 Ubuntu 环境用于构建 MaixCDK。
使用 Docker，你无需手动安装依赖项，只需创建一个 Docker 容器，一切环境就准备好了。

安装 Docker 的文档请参考 [Docker 官方文档](https://docs.docker.com/engine/install/ubuntu/)。

安装完成后，你可以使用 `docker --version` 检查是否安装成功。

## 从 Docker Hub 拉取（推荐），或自行构建

```shell
docker pull sipeed/maixcdk-builder
```

> 或者你也可以通过 Dockerfile 自行构建：
>
> ```shell
> docker build --network=host -t maixcdk-builder .
> ```
> 你也可以通过添加参数使用代理：
> `--network=host --build-arg http_proxy=http://127.0.0.1:8123 --build-arg https_proxy=http://127.0.0.1:8123`。

## 创建并运行容器

在上一步中我们得到了一个 Docker 系统镜像，现在我们创建一个容器来运行这个镜像。

```shell
docker run -it --network=host --hostname maixcdk-env --name maixcdk-env --env USER=$USER --env UID=`id -u` --env GID=`id -g` --env MAIXCDK_PATH=/home/${USER}/MaixCDK -v /home/${USER}/MaixCDK:/home/${USER}/MaixCDK sipeed/maixcdk-builder
```

> !! 不要在命令末尾添加 `/bin/bash`。
> `--network=host` 表示使用与主机相同的网络。
> `--hostname` 设置容器的主机名为 `maixcdk-env`。
> `--name` 设置容器名称为 `maixcdk-env`，方便使用 `docker` 命令进行控制。
> `USER`（主机的用户名）、用户 ID 和组 ID 将与主机的用户信息一致，避免权限问题。
> 你可以使用 `-v host_dir:container_dir` 将你的数据映射到容器中，例如：`-v /home/${USER}/MaixCDK:/home/${USER}/MaixCDK -v /home/${USER}/projects:/home/${USER}/projects`。
> 建议将主机目录映射到容器的相同目录，这样查看日志会更方便。
> 使用 `--env MAIXCDK_PATH=/home/${USER}/MaixCDK` 参数设置 `MAIXCDK_PATH` 环境变量，这样你可以在容器内任何位置编译项目。
> `sipeed/maixcdk-builder` 是镜像名称，如果你自行构建，请使用 `maixcdk-builder`。

接下来你可以在容器中使用 shell 命令。默认用户密码为 `maixcdk`，如果需要修改密码，可以使用以下命令：

```shell
docker exec -it maixcdk-env passwd
```

## 构建 MaixCDK 项目或示例

进入容器后，可以根据 MaixCDK 文档使用 shell 命令进行构建。

```shell
cd /MaixCDK/examples/hello_world
maixcdk build
maixcdk run

cd ~/projects/my_project
maixcdk build
maixcdk run
```

## 停止容器

```shell
docker stop maixcdk-env
```

## 启动容器

```shell
docker start maixcdk-env
docker attach maixcdk-env
```

## 从主机 shell 执行命令

```shell
docker exec -it --user $USER maixcdk-env "cd /MaixCDK/examples/hello_world && maixcdk build && maixcdk run"
```

```shell
docker exec -it --user $USER maixcdk-env /bin/bash
```

## 删除容器

```shell
docker stop maixcdk-env
docker rm maixcdk-env
```
