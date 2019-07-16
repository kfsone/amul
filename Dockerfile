FROM ubuntu:latest
RUN apt update
RUN apt install -qy build-essential gcc g++ clang cmake ninja-build
RUN apt install -qy vim valgrind gdb lldb

RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang 100
RUN update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 100

ENTRYPOINT [ "/bin/bash" ]
