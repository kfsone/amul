FROM alpine:latest
RUN apk update
RUN apk add clang clang-analyzer cmake ninja

ENTRYPOINT [ "/bin/ash" ]
