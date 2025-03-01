#!/bin/bash

set -euo pipefail

curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --profile default
chmod -R a+w ${RUSTUP_HOME} ${CARGO_HOME}
