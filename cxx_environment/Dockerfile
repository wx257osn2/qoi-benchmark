ARG GCC_VERSION=16.2.0
ARG CMAKE_VERSION=4.4.3

FROM ubuntu:26.04 AS builder

ARG GCC_VERSION
ARG CMAKE_VERSION
ARG DEBIAN_FRONTEND=noninteractive

RUN --mount=type=cache,target=/var/lib/apt,sharing=locked \
    --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=bind,src=installer,target=/installer \
    /installer/ubuntu$(bash -c 'source /etc/os-release; echo ${VERSION_ID//./}')_prelude.bash

RUN --mount=type=cache,target=/var/lib/apt,sharing=locked \
    --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=bind,src=installer,target=/installer \
    /installer/ubuntu_gcc.bash ${GCC_VERSION}

RUN --mount=type=cache,target=/var/lib/apt,sharing=locked \
    --mount=type=cache,target=/var/cache/apt,sharing=locked \
    apt-get install -y --no-install-recommends make

RUN --mount=type=cache,target=/var/lib/apt,sharing=locked \
    --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=bind,src=installer,target=/installer \
    /installer/ubuntu_clang.bash 23 /opt/gcc-${GCC_VERSION}

RUN --mount=type=bind,src=installer,target=/installer \
    /installer/cmake.bash ${CMAKE_VERSION}

ENV RUSTUP_HOME=/opt/rustup \
    CARGO_HOME=/opt/cargo

RUN --mount=type=bind,src=installer,target=/installer \
    /installer/rustup.bash

RUN --mount=type=cache,target=/var/lib/apt,sharing=locked \
    --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=bind,src=installer,target=/installer \
    /installer/ubuntu2404_postlude.bash

FROM ubuntu:26.04 AS final

COPY --from=builder /usr /usr
COPY --from=builder /opt /opt
COPY --from=builder /etc/alternatives /etc/alternatives
COPY --from=builder /etc/ssl /etc/ssl

RUN chmod a+w /opt/rustup /opt/cargo

ARG GCC_VERSION
ARG CMAKE_VERSION

ENV RUSTUP_HOME=/opt/rustup \
    CARGO_HOME=/opt/cargo \
    PATH=${PATH}:/opt/cmake-${CMAKE_VERSION}/bin:/opt/gcc-${GCC_VERSION}/bin
