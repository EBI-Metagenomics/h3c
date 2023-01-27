#!/bin/bash

pipx run h3daemon start "$1" --force --port 51371
pipx run h3daemon press "$1"
