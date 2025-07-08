## Cearch
Inspired by https://github.com/tsoding/seroost

tfidf full text search 

## Dependencies
- Boost Asio (libboost-all-dev)
- Pugixml (libpugixml-dev)
- poppler pdf (lib-poppler, lib-poppler-cpp-dev) 
- nlohmann json (nlohmann-json3-dev)
- openssl (libssl-dev)
- zlib (zlib1g-dev)

## Build the project
make

## Run Cearch
./cearch 8080 docs.gl index 1 10

# Container
## build container
docker build -t cearch .

## run container with output and mount with testdata
docker run --rm -p 8088:8088 -v ./testdata:/app/testdata cearch

# Performance stuff, index in memory
Test on:
Vendor ID:                ARM
  Model name:             Cortex-A72
    Model:                3
    Thread(s) per core:   1
    Core(s) per cluster:  4

RAM usage in MB: ps -p <cearch-pid> -o rss=  | awk '{ printf "%.2f MB\n", $1 / 1024 }'

Query "Moby, Goethe"

100 Books
    RAM: 120 MB
    Time to index, sequential: 11.0227 seconds
    Time to index, async: 
    Query time: 0.247146 milliseconds

1000 Books
    RAM: 1094.41 MB
    Time to index, sequential: 148.02 seconds
    Time to index, async:
    Query time: 1.51336 milliseconds
    Index FS size: 550M

5000 Books 
    RAM: 5465.75 MB
    Time to index, sequential: 741.212 seconds
    Time to index, async:
    Query time: 9.45767 milliseconds
    Index FS size: 2.8G