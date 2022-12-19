FROM ubuntu:20.04 AS base

RUN useradd omicsds && usermod -aG sudo omicsds && passwd -d omicsds
WORKDIR /home/omicsds

FROM base as developer

ARG DEBIAN_FRONTEND=noninteractive
# Library build requirements
RUN apt update && apt install -y cmake lcov zlib1g-dev libssl-dev uuid-dev libcurl4-openssl-dev zstd sudo git make g++ pkg-config autoconf automake patch gzip tar

# Documentation build requirements
RUN apt install -y --no-install-recommends python3 python3-venv python3-pip doxygen

# R requirements
RUN apt install -y --no-install-recommends r-base

# Other items (clang-format, catch2)
RUN apt install -y --no-install-recommends clang-format

# Final apt cleanup
RUN rm -rf /var/lib/apt/lists/*

# User setup
COPY --chown=omicsds:omicsds .github/scripts/install_catch2.sh install_catch2.sh
RUN chown omicsds:omicsds /home/omicsds
USER omicsds
RUN python3 -m venv .env && . .env/bin/activate && pip3 install sphinx breathe
RUN echo "source .env/bin/activate" > .bashrc
RUN INSTALL_DIR=/home/omicsds/catch2-install CATCH2_VER=v2.13.9 bash install_catch2.sh

FROM developer as builder
COPY --chown=omicsds:omicsds . /home/omicsds/OmicsDS

RUN "/home/omicsds/OmicsDS/.github/scripts/readme_test.sh"

FROM base as consumer

COPY --from=builder /home/omicsds/OmicsDS/install /home/omicsds/omicsds-libs
COPY --from=builder /home/omicsds/R-libs /home/omicsds/R-libs
COPY --from=builder /home/omicsds/OmicsDS/build/docs/sphinx /home/omicsds/docs

ENTRYPOINT [ "/bin/bash" ]
