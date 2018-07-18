FROM artprod.dev.bloomberg.com/devx/rhel-build-base

RUN apt-get update && \
    apt-get install --force-yes -y \
    builddeb \
    clang \
    clangtools-meta \
    libclangmetatool-clang-5.0-dev \
    libtinfo-dev \
    libgtest-dev

COPY . source
WORKDIR source
RUN mkdir build -p && \
    cd build && \
    export LDFLAGS=-L/opt/bb/lib64 && \
    export CXXFLAGS=-I/opt/bb/include && \
    cmake -DClang_DIR=/opt/bb/lib/llvm-5.0/lib64/cmake/clang/ \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    ..

RUN make -C build && \
    make CTEST_OUTPUT_ON_FAILURE=1 -C build test

