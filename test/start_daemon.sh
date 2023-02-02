#!/bin/bash
set -e

pipx run --no-cache h3daemon press "$1"
pipx run h3daemon start "$1" --force --port 51371
