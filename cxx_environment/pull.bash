#!/bin/bash

here="$(realpath $(dirname ${BASH_SOURCE:-$0}))"

if [ ! "${SINGULARITY:+true}" ]; then
  if [ `which singularity` ]; then
    SINGULARITY=singularity
  elif [ `which apptainer` ]; then
    SINGULARITY=apptainer
  fi
fi

set -euo pipefail

yasuno () {
  printf "${1} [y/N]: "
  exec < /dev/tty
  read yn
  case $yn in
    [yY]*) return 0 ;;
        *) return 1 ;;
  esac
}

version=${1:-"image-v20261012"}
arch=$(uname -m)
uri=${2:-"https://github.com/wx257osn2/qoi-benchmark/releases/download/{}"}

if [ -f ${here}/cxx-${arch}.sif ]; then
  yasuno "${here}/cxx-${arch}.sif already exists. overwrite?"
  ret=$?
  if [ $ret -eq 1 ]; then
    exit $ret
  fi
fi

if [[ ${uri} == oras://* ]]; then
  ${SINGULARITY} pull $(echo ${uri} | sed "s/{}/${version}/g")-${arch}
  filename=$(echo ${uri##*/} | sed "s/:{}/_${version}-${arch}/g").sif
  if [ ${filename} != "cxx-${arch}.sif" ]; then
    mv ${filename} ${here}/cxx-${arch}.sif
  fi
else
  curl -sSL -o ${here}/cxx-${arch}.sif $(echo ${uri} | sed "s/{}/${version}/g")/${3:-cxx}-${arch}.sif
fi
