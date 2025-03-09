ARG CMAKE_VERSION=3.31.3

FROM ubuntu:24.04 AS builder

ARG CMAKE_VERSION

RUN --mount=type=cache,target=/var/lib/apt,sharing=locked \
    --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=bind,src=installer,target=/installer \
    /installer/ubuntu2404_prelude.bash

RUN --mount=type=cache,target=/var/lib/apt,sharing=locked \
    --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=bind,src=installer,target=/installer \
    /installer/ubuntu2404_gcc14.bash

RUN --mount=type=cache,target=/var/lib/apt,sharing=locked \
    --mount=type=cache,target=/var/cache/apt,sharing=locked \
    apt-get install -y --no-install-recommends make

RUN --mount=type=cache,target=/var/lib/apt,sharing=locked \
    --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=bind,src=installer,target=/installer \
    /installer/ubuntu2404_clang20.bash

RUN --mount=type=cache,target=/var/lib/apt,sharing=locked \
    --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=bind,src=installer,target=/installer \
    /installer/ubuntu2404_clang-format18.bash

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

FROM ubuntu:24.04 AS final

COPY --from=builder /usr /usr
COPY --from=builder /opt /opt
COPY --from=builder /etc/alternatives /etc/alternatives
COPY --from=builder /etc/ssl /etc/ssl

RUN chmod a+w /opt/rustup /opt/cargo

ARG CMAKE_VERSION

ENV RUSTUP_HOME=/opt/rustup \
    CARGO_HOME=/opt/cargo \
    PATH=${PATH}:/opt/cmake-${CMAKE_VERSION}/bin
