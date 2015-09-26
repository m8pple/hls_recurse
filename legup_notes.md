
Install 32-bit c++ libraries
----------------------------

Legup wants to use 32-bit (-m32), so make sure
that 32-bit libraries are included:

    sudo apt-get install g++-multilib

Symptoms are:

    vagrant@vagrant-ubuntu-trusty-64:/mnt/_dt10_/documents/papers/working/ideas/hls_recursion/src$ clang++ -std=c++11 state_machine_self_recursion_test.cpp  -I ../include -emit-llvm -c  -fno-builtin -I /home/vagrant/legup/examples/lib/include/  -m32
    In file included from state_machine_self_recursion_test.cpp:3:
    /usr/bin/../lib/gcc/x86_64-linux-gnu/4.8/../../../../include/c++/4.8/cstdio:41:10: fatal error:
          'bits/c++config.h' file not found
    #include <bits/c++config.h>
             ^
    1 error generated.

Patch GenerateRTL.cpp
---------------------

Apply src/eval/legup/GenerateRTL.patch to legup/llvm/lib/Target/Verilog/GenerateRTL.cpp

This avoids situations where legup tries to create dot files with
pathnames that are too long, and then fails when it can't write
to them.

Setup full modelsim
---------------------

The output of recursive programs will take forever
to simulate due to auto-slowdown for large programs
in the modelsim starter edition

Update cmake
------------

The default cmake on the legupUbuntu  vm is too old to support certain
things like c++11 features for compilers. It needs to be updated to
something more recent. e.g. http://askubuntu.com/q/610291. I used:

    sudo add-apt-repository ppa:george-edison55/cmake-3.x
    sudo apt-get update
    sudo apt-get upgrade

This has the side effect of upgrading a number of other things,
but doesn't seem to affect the integrity of the legup build.

While you're at it (or ideally before), the VM can be cleaned up a bit:

    sudo apt-get remove --purge libreoffice*
    sudo apt-get remove --purge thunderbird*
    sudo apt-get clean
    sudo apt-get autoremove

Not that it really matters that much given how big quartus
and modelsim are.
