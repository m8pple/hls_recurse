
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
