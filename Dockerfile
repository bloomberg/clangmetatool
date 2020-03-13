ARG BASE_IMAGE=ubuntu:18.04

FROM $BASE_IMAGE

ARG TARGET_LLVM_VERSION=8
ARG IMAGE_REPO=bionic

# Depenedencies to fetch, build llvm and clang
RUN apt-get update && apt-get install -y \
        wget \
        gnupg2 \
        lsb-release \
        software-properties-common

RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
    # The repository name used here MUST be kept in sync with the base image version
    add-apt-repository "deb http://apt.llvm.org/$IMAGE_REPO/ llvm-toolchain-$IMAGE_REPO-$TARGET_LLVM_VERSION   main"

RUN apt-get update

RUN apt-get install -y \
        # Build toolchains that we are targeting for compatibility with
        gcc-7 \
        g++-7 \
        cmake \
        # clangmetatool uses gtest
        libgtest-dev \
        # Depenedencies for our perl scripts
        libfile-spec-native-perl

# Set up LLVM packages with depenedencies
RUN apt-get install -y \
        # LLVM
        llvm-"$TARGET_LLVM_VERSION" \
        libllvm"$TARGET_LLVM_VERSION" \
        llvm-"$TARGET_LLVM_VERSION"-dev \
        # Clang & friends
        clang-"$TARGET_LLVM_VERSION" \
        libclang-common-"$TARGET_LLVM_VERSION"-dev \
        libclang-"$TARGET_LLVM_VERSION"-dev \
        # libc++
        libc++-"$TARGET_LLVM_VERSION"-dev

# Set up build environment
ENV CC=/usr/bin/gcc-7 \
    CXX=/usr/bin/g++-7 \
    MAKEFLAGS="-j4" \
    CMAKE_BUILD_PARALLEL_LEVEL=4

# Install gtest as they recommend to, for 1.8.x
RUN cd /usr/src/gtest && \
    cmake . && \
    make && \
    mv libg* /usr/lib

COPY . clangmetatool/
WORKDIR clangmetatool

# Build tool, run tests, and do a test install
RUN cmake \
        -DClang_DIR="$(llvm-config-$TARGET_LLVM_VERSION --cmakedir)"/../clang \
        -DLLVM_DIR="$(llvm-config-$TARGET_LLVM_VERSION --cmakedir)" \
        -Bbuild \
        -H.
RUN cmake --build build/ --target all
RUN cmake --build build/ --target test -- ARGS="--output-on-failure"
RUN cmake --build build/ --target install

# Fix includes for clangmetatool (due to ubuntu debian's clang)
RUN ln -s /usr/lib/llvm-8/include/clangmetatool /usr/include/clangmetatool

# Build skeleton
RUN mkdir skeleton/build && cd skeleton/build && \
    cmake -DClang_DIR="$(llvm-config-$TARGET_LLVM_VERSION --cmakedir)"/../clang \
          -Dclangmetatool_DIR="$(llvm-config-$TARGET_LLVM_VERSION --cmakedir)"/../clang .. && \
    cmake --build . --target all && \
    cmake --build . --target install && \
    cd - && rm -rf skeleton/build

# Run the tool on itself
RUN yourtoolname $(find src skeleton -name '*.cpp') -- -std=gnu++14 -I$(llvm-config-$TARGET_LLVM_VERSION --includedir)
