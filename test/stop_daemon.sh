#!/bin/bash

pipx run h3daemon stop "$(basename "$1")"
