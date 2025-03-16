FROM alpine:latest AS build

RUN apk update && \
    apk add --no-cache \
    boost-dev poppler-dev pugixml-dev clang clang-dev make

WORKDIR /cearch
COPY source/ ./source/
COPY Makefile .

RUN make

FROM alpine:latest

RUN apk update && \
    apk add --no-cache \
    poppler-dev pugixml-dev

RUN addgroup -S cearch && adduser -S cearch -G cearch
USER cearch

COPY --chown=cearch:cearch --from=build \
    ./cearch/cearch \
    ./app/

COPY --chown=cearch:cearch \
    web/ /web/

ENTRYPOINT /app/cearch 8088 /app/testdata /app/index 1 10
