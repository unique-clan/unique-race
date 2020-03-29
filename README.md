Unique Race
===========

The Unique Race modification of DDNet

Building
--------

To compile Unique Race, execute the following commands in the source root:

    mkdir build
    cd build
    cmake -DCLIENT=OFF -DMYSQL=ON ..
    make -j$(nproc) DDNet-Server

On Arch Linux the MariaDB C++ connector is available as the AUR package `mysql-connector-c++`.
