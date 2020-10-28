FROM alpine:3.12 AS build
RUN apk add --no-cache build-base cmake protobuf-dev protoc libexecinfo-dev

COPY . /usr/app/builder

WORKDIR /usr/app/builder

RUN find ./ -type f -print | xargs chmod 777 \
    &&mkdir build \
    &&cd build \
    &&cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    && make install -j

FROM alpine:3.12 AS runtime
COPY --from=build /usr/local/bin/jumanpp /usr/local/bin/jumanpp
COPY --from=build /usr/local/libexec/jumanpp /usr/local/libexec/jumanpp