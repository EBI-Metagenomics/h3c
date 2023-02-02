#!/bin/bash
set -e

pipx run h3daemon stop "$(basename "$1")"
