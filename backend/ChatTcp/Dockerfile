FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    locales \
    && rm -rf /var/lib/apt/lists/*

ENV LANG=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8

WORKDIR /app

COPY . /app

RUN g++ -std=c++17 -O2 -pthread -o server main.cpp Room.cpp

EXPOSE 8080

CMD ["./server"]