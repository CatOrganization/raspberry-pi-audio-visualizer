FROM arm32v7/ubuntu

RUN apt-get update
RUN apt-get -y install build-essential
RUN apt-get -y install \
	libasound2-dev \
	git

WORKDIR /opt/src/

RUN git clone https://github.com/raysan5/raylib.git

RUN echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections

RUN apt-get install -y \
	libxcursor-dev \
	libxinerama-dev \
	libxrandr-dev \
	libxi-dev \
	mesa-common-dev \
	libgl1-mesa-dev \
	libx11-dev

RUN apt-get install -y xorg-dev

RUN apt-get install -y \
	--no-install-recommends \
	gvfs

RUN cd raylib && git fetch && git checkout 3.0.0
RUN make -C raylib/src/ PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=SHARED
RUN make -C raylib/src/ install PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=SHARED

COPY . /opt/src/c-viz

RUN make -C c-viz PLATFORM=PLATFORM_DESKTOP
