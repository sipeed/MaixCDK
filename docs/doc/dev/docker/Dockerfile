FROM ubuntu:20.04

ENV DEBIAN_FRONTEND noninteractive

EXPOSE 8888
EXPOSE 2333

RUN dpkg --add-architecture i386 \
    && apt-get -o APT::Retries=3 update -y

RUN apt-get update \
    && apt-get install -y software-properties-common \
    && add-apt-repository -y ppa:deadsnakes/ppa\
    && apt-get update \
	&& apt-get install build-essential vim \
        git libncurses5-dev zlib1g-dev gawk \
        libssl-dev unzip lib32z1 lib32z1-dev lib32stdc++6 libstdc++6 \
        ca-certificates file g++-multilib libc6:i386 locales \
        python3 python3-pip rsync shellcheck \
        libopencv-dev libopencv-contrib-dev \
        libsdl2-dev wget sudo -y \
        && wget -q https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh -O miniconda.sh \
        && bash miniconda.sh -b -p /opt/miniconda \
        && rm miniconda.sh \
        && /opt/miniconda/bin/conda tos accept --override-channels --channel https://repo.anaconda.com/pkgs/main \
        && /opt/miniconda/bin/conda tos accept --override-channels --channel https://repo.anaconda.com/pkgs/r \
        && apt-get purge -yq software-properties-common \
        && apt-get autoremove -yq --purge \
        && apt-get clean \
        && rm -rf /var/lib/apt/lists/* \
        && rm -rf /tmp/* 


ENV PATH="/opt/miniconda/bin:$PATH"
RUN conda init bash \
    && conda init zsh
COPY requirements.txt /tmp/requirements.txt
COPY teedoc_requirements.txt /tmp/teedoc_requirements.txt

RUN conda create -n maixcam2 python=3.13 -y
RUN conda run -n maixcam2 pip install -r /tmp/requirements.txt && \
    conda run -n maixcam2 pip install -r /tmp/teedoc_requirements.txt


RUN conda create -n maixcam python=3.11 -y

RUN conda run -n maixcam2 pip install -r /tmp/requirements.txt && \
    conda run -n maixcam2 pip install -r /tmp/teedoc_requirements.txt

COPY switch_user.sh /usr/local/bin/switch_user.sh

RUN echo "conda activate maixcam2" >> ~/.bashrc
RUN chmod +x /usr/local/bin/switch_user.sh
WORKDIR /maix
ENTRYPOINT ["/usr/local/bin/switch_user.sh"]

