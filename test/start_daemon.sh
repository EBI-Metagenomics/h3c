#!/bin/sh
set -e

test -e "$1.h3f" || pipx run --spec hmmer hmmpress "$1"
pipx run h3daemon start "$1" --port 51371
