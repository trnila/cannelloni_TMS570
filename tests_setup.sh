#!/bin/bash
set -ex

sudo ip link set can0 down
sudo ip link set can0 type can bitrate 500000
sudo ip link set can0 up

create_vcan() {
  sudo ip link add name "$1" type vcan || true
  sudo ip link set "$1" up
}

create_vcan can-0-0
create_vcan can-0-1
create_vcan can-0-2
create_vcan can-0-3

./bridge/cannelloni_bridge \
  can-0-0:10.0.0.1:20000 \
  can-0-1:10.0.0.1:20001 \
  can-0-2:10.0.0.1:20002 \
  can-0-3:10.0.0.1:20003
