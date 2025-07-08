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

# Performance stuff
RAM usage in MB: ps -p <cearch-pid> -o rss=  | awk '{ printf "%.2f MB\n", $1 / 1024 }'

Query "Moby, Goethe"

100 Books
    RAM: 
    Time to index, sequential
    Time to index, async:
    Query response time: 

1000 Books
    RAM: 
    Time to index, sequential
    Time to index, async:
    Query response time: 

10000 Books
    RAM: 
    Time to index, sequential
    Time to index, async:
    Query response time: 