FROM alpine:3.12 AS build-jumanpp
RUN apk add --no-cache build-base cmake protobuf-dev protoc libexecinfo-dev
 
COPY . /usr/app/builder

WORKDIR /usr/app/builder

RUN find ./ -type f -print | xargs chmod 777 \
    &&mkdir build \
    &&cd build \
    &&cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    && make install -j