FROM ubuntu:22.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY server.cpp /app/server.cpp
COPY index.html /app/index.html
COPY app.js /app/app.js
COPY styles.css /app/styles.css

RUN g++ -O2 -std=c++17 server.cpp -o server

EXPOSE 8080

CMD ["./server"]
