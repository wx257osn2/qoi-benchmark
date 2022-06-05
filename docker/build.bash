#!/bin/bash

here="$(realpath $(dirname ${BASH_SOURCE:-$0}))"

docker build -t qoi-benchmark:latest -f ${here}/Dockerfile.qoi-benchmark ${here}
