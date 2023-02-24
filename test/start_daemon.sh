#!/bin/sh
set -e

echo "Ponto 1"
test -e "$1.h3f" || pipx run --spec hmmer hmmpress "$1"
echo "Ponto 2"
pipx run h3daemon start "$1" --port 51371
echo "Ponto 3"
pipx run h3daemon port "$1"
echo "Ponto 4"
