FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    git \
    pkg-config \
    libboost-all-dev \
    nlohmann-json3-dev \
    libpqxx-dev \
    libxxhash-dev \
    libhiredis-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /tmp

RUN git clone --depth 1 https://github.com/sewenew/redis-plus-plus.git \
    && cd redis-plus-plus \
    && mkdir build \
    && cd build \
    && cmake .. \
        -DREDIS_PLUS_PLUS_BUILD_TEST=OFF \
        -DREDIS_PLUS_PLUS_BUILD_SHARED=ON \
        -DREDIS_PLUS_PLUS_BUILD_STATIC=OFF \
    && cmake --build . -j"$(nproc)" \
    && cmake --install . \
    && ldconfig

WORKDIR /app

COPY . .

RUN cmake -S . -B build
RUN cmake --build build -j"$(nproc)"

CMD ["./build/shortener", "--port", "8080", "--threads", "4", "--db-host", "postgres", "--db-port", "5432"]