#!/bin/bash
set -ex

sudo ip link set can0 down
sudo ip link set can0 type can bitrate 500000
sudo ip link set can0 up

create_vcan() {
  sudo ip link add name "$1" type vcan || true
  sudo ip link set "$1" up
}

for node in 0 1 2; do
  create_vcan can-$node-0
  create_vcan can-$node-1
  create_vcan can-$node-2
  create_vcan can-$node-3
done

make -C bridge
./bridge/cannelloni_bridge
