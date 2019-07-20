# kfsone/amul: Builds an environment for building & testing my retrofit of my
# 90's Amiga MUd Language: amul.
#
# Installs the latest ubuntu with a self-hosted CMake 13.15 (vs 13.10)
#
# Also:
#  gcc-7  (at time of writing)
#  clang-8
#  gdb, lldb
#  cppcheck, include-what-you-use, ninja build, python3 and ipython
#
# Mount your amul source folder as /amul and see amul/etc/Makefile for
# wrapping the ninja commands.
#
# I typically build into /amul/build or /amul/linux if I'm using wsl
# and want to build windows and wsl at the same time.

FROM ubuntu:latest

# On my home network, I have "wafer.lan" as my apt cache.
# Maybe I should cname that to something other people would be ok using :)
COPY etc/02proxy /etc/apt/apt.conf.d
RUN if ! getent hosts wafer.lan >/dev/null ; then \
		rm /etc/apt/apt.conf.d/02proxy ; \
	fi

# Baseline packages
ARG DEBIAN_FRONTEND=noninteractive
RUN apt update && apt install -qy apt-utils && apt -qy upgrade

#TODO: iwyu, ctags, git-core, git-extras, gcovr, lcov
RUN apt install -qy vim less cppcheck ninja-build make \
					python3 python3-pip python3-virtualenv virtualenv
RUN apt install -qy	gcc g++ gdb valgrind
RUN apt install -qy clang-8 clang-8-doc lldb-8 libfuzzer-8-dev clang-tidy-8

RUN pip3 install --user ipython

# I want gcc and clang to be the default compilers
RUN \
	update-alternatives --install /usr/bin/cc cc /usr/bin/clang-8 100 && \
	update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-8 100

# Build our version of CMake
COPY etc/vimrc ~/.vimrc
COPY etc/cmake* /tmp/
RUN chmod +x /tmp/cmake.build.sh
RUN /bin/bash /tmp/cmake.build.sh

ENTRYPOINT [ "/bin/bash" ]
