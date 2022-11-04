#!/bin/bash

setup() {
    bats_load_library bats-support
    bats_load_library bats-assert
    bats_load_library bats-file
    PATH="$BATS_TEST_DIRNAME/..:$PATH"

    pipx install pooch-cli >/dev/null
    pooch https://pub.danilohorta.me/deciphon/ross.1.hmm --hash 12ba4be00070134ec2e77696eb88263ac8e8d0cd30375670d461ba6361a574e9

    curl -sOL https://app.danilohorta.me/h3daemon
    chmod +x h3daemon

    ./h3daemon start ross.1.hmm --name=h3daemon --yes
}

teardown() {
    ./h3daemon stop --name=h3daemon
    rm -f h3daemon ross.1.hmm
}

request() {
    cat <<EndOfMessage
@file output.mpk
>tr|Q949S7|Q949S7_ARATH NAD(P)-binding Rossmann-fold superfamily protein OS=Arabidopsis thaliana OX=3702 GN=At5g15910 PE=1 SV=1
MLRSLIWKRSQAYSSVVTMSSISQRGNERLLSEVAGSHSRDNKILVLGGNGYVGSHICKE
ALRQGFSVSSLSRSGRSSLHDSWVDDVTWHQGDLLSPDSLKPALEGITSVISCVGGFGSN
SQMVRINGTANINAVKAAAEQGVKRFVYISAADFGVINNLIRGYFEGKRATEAEILDKFG
NRGSVLRPGFIHGTRQVGSIKLPLSLIGAPLEMVLKLLPKEVTKIPVIGPLLIPPVNVKS
VAATAVKAAVDPEFASGVIDVYRILQHGH
//
EndOfMessage
}

@test "first" {
    request | h3client
}
