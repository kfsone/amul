FROM ubuntu:latest

COPY sample/02proxy /etc/apt/apt.conf.d
RUN if ! getent hosts wafer.lan >/dev/null ; then \
		rm /etc/apt/apt.conf.d/02proxy ; \
	fi

RUN apt update
RUN apt install -qy build-essential gcc g++ clang cmake ninja-build
RUN apt install -qy vim valgrind gdb lldb
RUN apt install -qy clang-tidy cppcheck iwyu

RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang 100
RUN update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 100

ENTRYPOINT [ "/bin/bash" ]
