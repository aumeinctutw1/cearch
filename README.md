## Cearch
Inspired by https://github.com/tsoding/seroost

tfidf full text search 

## Dependencies
- Pugixml (libpugixml-dev)
- Boost Asio (libboost-all-dev)
- poppler (lib-poppler)

## Build the project
make

## Run Cearch
./cearch 8080 docs.gl index 1 10

# Container
## build container
docker build -t cearch .

## run container with output and mount with testdata
docker run --rm -p 8088:8088 -v ./testdata:/app/testdata cearch