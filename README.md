# TMS570 IPv6-only CAN-UDP bridge via cannelloni protocol
TMS570 cluster of 3x4 CAN interfaces bridged over UDP via [cannelloni protocol](https://github.com/mguentner/cannelloni).
Automatic Configuration is accomplished by using IPv6 and mDNS Discovery.

## Building and flashing
Set the environment variable `TI_CGT_ROOT` to the location of the TI-CGT compiler, and then run the `make` command to initiate the build process. Additionally, consult the provided build [pipeline](.github/workflows/build.yml) for detailed steps.

You can flash the entire cluster by using `make flash`, or you can flash individual cores with `make flash_0` target.
Make sure that Uniflash is added to your system `PATH`.

## Running `cannelloni_bridge`
Automatically discovers TMS570 devices via mDNS and sets up bridges between UDP and virtual CAN interfaces.

```shell-session
$ make -C bridge
$ ./bridge/cannelloni_bridge
```

## Testing

```shell-session
$ ./tests_setup.sh
$ CAN_RX=can0 CAN_TX=can-0-0 pytest
```
