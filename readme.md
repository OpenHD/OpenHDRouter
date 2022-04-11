## Archieved, can be found in the main repo under openhd-telemetry

## OpenHDouter

This code is intended to act as a Mavlink router

A fork of the Mavlink library with custom messages and commands for OpenHD (not finalized, they're subject to change at any time) is included in this repo as a submodule, however the generated Mavlink headers are also included directly in the repo so you don't need to rebuild them.

----

## Dependencies

    apt install build-essential git python libboost-dev libboost-program-options-dev libboost-system-dev libasio-dev

----

## Build

    git clone https://github.com/OpenHD/OpenHDRouter.git
    cd OpenHDRouter
    git submodule update --init
    sudo make install

-----

## Use

There's a systemd service file for starting the router:

    systemctl start openhd_router

You can also run them directly for testing:

    openhd_router --tcp-port 5761 --serial-port /dev/openhd_microservice1

That will start the router on TCP port 5761 and connect to the local mavlink pipe wired to `tx_telemetry`
and `rx_rc_telemetry_buf`. The baud rate is always 115200.
