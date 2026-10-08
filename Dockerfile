FROM ubuntu:24.04

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    gdb \
    valgrind \
    netcat-openbsd \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . /app

RUN cmake -S . -B build -G Ninja && \
    cmake --build build -j$(nproc)

EXPOSE 8080

CMD ["./build/main"]