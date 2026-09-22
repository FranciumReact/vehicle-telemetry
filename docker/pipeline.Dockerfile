# Build stage: the full compiler toolchain, used only to produce the binary.
FROM ubuntu:24.04 AS build
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake libpqxx-dev \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY CMakeLists.txt .
COPY include/ include/
COPY src/ src/
COPY tests/ tests/
RUN cmake -S . -B build && cmake --build build -j

# Runtime stage: only the binary and the shared library it links against.
# The compiler never ships.
FROM ubuntu:24.04
RUN apt-get update && apt-get install -y --no-install-recommends \
        libpqxx-7.8t64 \
    && rm -rf /var/lib/apt/lists/*
COPY --from=build /src/build/encode_test /usr/local/bin/pipeline
CMD ["pipeline"]