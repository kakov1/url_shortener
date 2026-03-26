FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    libboost-all-dev \
    nlohmann-json3-dev \
    libpqxx-dev \
    libxxhash-dev \                                          
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN cmake -S . -B build
RUN cmake --build build -j

CMD ["./build/shortener", "--port", "8080", "--threads", "4", "--db-host", "postgres", "--db-port", "5432"]