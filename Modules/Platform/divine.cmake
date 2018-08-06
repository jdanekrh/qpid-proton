SET(CMAKE_C_OUTPUT_EXTENSION .bc)
SET(CMAKE_CXX_OUTPUT_EXTENSION .bc)

SET(CMAKE_STATIC_LIBRARY_SUFFIX_C .a)
SET(CMAKE_STATIC_LIBRARY_SUFFIX_CXX .a)

find_program(compiler "compiler.py")
set(CMAKE_C_COMPILER ${compiler})
set(CMAKE_CXX_COMPILER ${compiler})
# https://stackoverflow.com/questions/1867745/cmake-use-a-custom-linker
set(CMAKE_CXX_LINK_EXECUTABLE "${compiler} <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

# 2 jdanek@nixos ~/Work/repos/qpid-proton/divine (git)-[jd_fuzz_use_static_lib] % cmake .. -DCMAKE_MODULE_PATH=~/Work/repos/qpid-proton/Modules -DCMAKE_SYSTEM_NAME=divine -DLIB_SUFFIX="64" -DCMAKE_TOOLCHAIN_FILE=/home/jdanek/Work/repos/qpid-proton/divine.cmake --debug-trycompile

#../minisat/utils/ParseUtils.h:27:10: fatal error: 'zlib.h' file not found
##include <zlib.h>
#         ^~~~~~~~
#
#-- Performing Test CXX_SUPPORTS_NO_NESTED_ANON_TYPES_FLAG
#-- Performing Test CXX_SUPPORTS_NO_NESTED_ANON_TYPES_FLAG - Success
#-- Found Perl: /usr/bin/perl (found version "5.26.2")
#-- Could NOT find ODBC (missing: ODBC_CONFIG_EXECUTABLE ODBC_INCLUDE_DIRS_)
#-- Found PkgConfig: /usr/bin/pkg-config (found version "1.4.2")
#-- Could NOT find Z3 (missing: Z3_LIBRARY Z3_INCLUDE_DIR)
#-- Found Curses: /usr/lib64/libcurses.so
#-- Performing Test BRICKS_HAVE_DIRENT_D_TYPE
#-- Performing Test BRICKS_HAVE_DIRENT_D_TYPE - Success
#-- Checking for module 'valgrind'
#--   Package 'valgrind', required by 'virtual:world', not found
#
#
#make static
#-- Performing Test LLVM_NO_OLD_LIBSTDCXX
#-- Performing Test LLVM_NO_OLD_LIBSTDCXX - Failed
#CMake Error at llvm/cmake/modules/CheckCompilerVersion.cmake:38 (message):
#  Host Clang must be able to find libstdc++4.8 or newer!
#Call Stack (most recent call first):
#  llvm/cmake/config-ix.cmake:14 (include)
#  CMakeLists.txt:103 (include)
#  CMakeLists.txt:133 (build_llvm)
#
# libstdc++-static                                x86_64                                8.1.1-5.fc28                                   updates                                614 k
#
#install clang
#
#-- Configuring incomplete, errors occurred!
#See also "/mnt/divine/_build.static/CMakeFiles/CMakeOutput.log".
#See also "/mnt/divine/_build.static/CMakeFiles/CMakeError.log".
#make[2]: *** [Makefile:93: config] Error 1
#make[1]: *** [Makefile:107: static-divine] Error 2
#make: *** [Makefile:72: static] Error 2
#
#
#Ubuntu Note, selecting 'zlib1g-dev' instead of 'libz-dev'
#and build-essentials
#
#
#-- Detecting CXX compile features - done
#-- Performing Test LLVM_NO_OLD_LIBSTDCXX
#-- Performing Test LLVM_NO_OLD_LIBSTDCXX - Failed
#CMake Error at llvm/cmake/modules/CheckCompilerVersion.cmake:38 (message):
#  Host Clang must be able to find libstdc++4.8 or newer!
#Call Stack (most recent call first):
#  llvm/cmake/config-ix.cmake:14 (include)
#  CMakeLists.txt:103 (include)
#  CMakeLists.txt:133 (build_llvm)
#
#
#-- Configuring incomplete, errors occurred!
#See also "/mnt/divine/_build.static/CMakeFiles/CMakeOutput.log".
#See also "/mnt/divine/_build.static/CMakeFiles/CMakeError.log".
#[root@383592df1d09 _build.static]# /mnt/divine/_build.toolchain/clang//bin/clang ../../
#.ansible/        .bash_profile    .config/         .kube/           .pki/            .viminfo         get-pip.py       tc-bazel/
#.bash_history    .bashrc          .cshrc           .lesshst         .ssh/            check.cpp        original-ks.cfg
#.bash_logout     .cache/          .docker/         .local/          .tcshrc          divine/          qpid-proton/
#[root@383592df1d09 _build.static]# /mnt/divine/_build.toolchain/clang//bin/clang ../
#.authorspellings      _build.release/       a.out                 cryptoms/             doc/                  minisat/              tools/
#.darcsignore          _build.static/        bricks/               dios/                 external/             releng/
#CMakeLists.txt        _build.toolchain/     clang/                divine/               lart/                 stp/
#Makefile              _darcs/               compiler-rt/          divine-4.1.11.tar.gz  llvm/                 test/
#[root@383592df1d09 _build.static]# /mnt/divine/_build.toolchain/clang//bin/clang ../../
#.ansible/        .bash_profile    .config/         .kube/           .pki/            .viminfo         get-pip.py       tc-bazel/
#.bash_history    .bashrc          .cshrc           .lesshst         .ssh/            check.cpp        original-ks.cfg
#.bash_logout     .cache/          .docker/         .local/          .tcshrc          divine/          qpid-proton/
#[root@383592df1d09 _build.static]# /mnt/divine/_build.toolchain/clang//bin/clang ../../check.cpp
#In file included from ../../check.cpp:1:
#In file included from /usr/lib/gcc/x86_64-redhat-linux/8/../../../../include/c++/8/atomic:38:
#/usr/lib/gcc/x86_64-redhat-linux/8/../../../../include/c++/8/bits/c++0x_warning.h:32:2: error: This file requires compiler and library support for the ISO C++ 2011 standard. This
#      support must be enabled with the -std=c++11 or -std=gnu++11 compiler options.
##error This file requires compiler and library support \
# ^
#../../check.cpp:2:1: error: use of undeclared identifier 'std'
#std::atomic<float> x(0.0f);
#^
#2 errors generated.
#[root@383592df1d09 _build.static]# cat ../../check.cpp
##include <atomic>
#std::atomic<float> x(0.0f);
#int main() { return (float)x; }
#[root@383592df1d09 _build.static]#
#
#
#
#^
#In file included from /mnt/divine_u/dios/sys/dios.cpp:15:
#In file included from /mnt/divine_u/dios/sys/config.hpp:13:
#In file included from /mnt/divine_u/dios/vfs/passthru.h:176:
#/mnt/divine_u/dios/include/sys/syscall.def:50:1: error: use of undeclared identifier '_HOST_SYS_symlinkat'
#SYSCALL( symlinkat,   CONTINUE, int,     ( MEM _1, int _2, MEM _3 ) )
#^
#sudo apt-get install linux-headers-generic
#
#
#pacman -sy grep cmake make
#
#
#-- Could NOT find Z3 (missing: Z3_LIBRARY Z3_INCLUDE_DIR)
#-- Found Curses: /usr/lib/libcurses.so
#CMake Error at CMakeLists.txt:188 (message):
#  libedit is required for 'divine sim' (-DOPT_SIM=ON)
#
#
#-- Performing Test BRICKS_HAVE_DIRENT_D_TYPE
#
#
#LIBEDIT
#    linked by target "divine-sim" in directory /mnt/divine_a/divine
#LIBTINFO
#    linked by target "divine-sim" in directory /mnt/divine_a/divine
#
#
#
#
#
#checking whether getpwnam_r and getpwuid_r are posix _draft_ like... no
#checking that generated files are newer than configure... done
#configure: creating ./config.status
#config.status: creating Makefile
#./config.status: line 1298: awk: command not found
#config.status: error: could not create Makefile
#==> ERROR: A failure occurred in build().
#    Aborting...
#[jdanek@f999375f1e86 tinfo]$ sudo pacman -S awk
#bash: sudo: command not found
#[jdanek@f999375f1e86 tinfo]$ exit
#[root@f999375f1e86 divine_a]# pacman -S awk
#resolving dependencies...
#looking for conflicting packages...
#
#Packages (1) gawk-4.2.1-1
#
#Total Download Size:   1.02 MiB
#Total Installed Size:  2.47 MiB
#
#:: Proceed with installation? [Y/n] y
#warning: no /var/cache/pacman/pkg/ cache exists, creating...
#:: Retrieving packages...
# gawk-4.2.1-1-x86_64                                                              1047.3 KiB  1999K/s 00:01 [################################################################] 100%
#(1/1) checking keys in keyring                                                                              [################################################################] 100%
#(1/1) checking package integrity                                                                            [################################################################] 100%
#(1/1) loading package files                                                                                 [################################################################] 100%
#(1/1) checking for file conflicts                                                                           [################################################################] 100%
#(1/1) checking available disk space                                                                         [################################################################] 100%
#:: Processing package changes...
#(1/1) installing gawk                                                                                       [################################################################] 100%
#:: Running post-transaction hooks...
#(1/3) Cleaning up package cache...
#(2/3) Arming ConditionNeedsUpdate...
#
#/usr/sbin/makepkg: line 184: fakeroot: command not found
#
#
#
#
#ninja: warning: multiple rules generate divine/version-generated.cpp. builds involving this target will not be correct; continuing anyway [-w dupbuild=warn]
#[215/5274] Linking CXX executable llvm/bin/llvm-tblgen
#llvm/lib/libLLVMSupport.a(Path.cpp.o): In function `llvm::sys::fs::real_path(llvm::Twine const&, llvm::SmallVectorImpl<char>&, bool)':
#../llvm/lib/Support/Path.cpp:(.text._ZN4llvm3sys2fs9real_pathERKNS_5TwineERNS_15SmallVectorImplIcEEb+0x210): warning: Using 'getpwnam' in statically linked applications requires at runtime the shared libraries from the glibc version used for linking
#llvm/lib/libLLVMSupport.a(Path.cpp.o): In function `llvm::sys::path::native(llvm::SmallVectorImpl<char>&, llvm::sys::path::Style)':
#../llvm/lib/Support/Path.cpp:(.text._ZN4llvm3sys4path6nativeERNS_15SmallVectorImplIcEENS1_5StyleE+0x419): warning: Using 'getpwuid' in statically linked applications requires at runtime the shared libraries from the glibc version used for linking
#
#
#






#In file included from /usr/include/features.h:452:
#/usr/include/gnu/stubs.h:7:11: fatal error: 'gnu/stubs-32.h' file not found
## include <gnu/stubs-32.h>
#lib32-glibc


#[root@f999375f1e86 divine]# pkgfile --update
#:: Updating 3 repos...
#\  download complete: core                 [   751.2 KiB   751K/s  2 remaining]
#  download complete: extra                [     7.1 MiB  4.67M/s  1 remaining]
#  download complete: community            [    17.5 MiB  6.96M/s  0 remaining]
#:: download complete in 2.51s             <    25.4 MiB  10.1M/s  3 files    >
#:: waiting for 1 process to finish repacking repos...
#[root@f999375f1e86 divine]# pkgfile sys/epoll.h
#[root@f999375f1e86 divine]# pkgfile epoll.h
#core/glibc
#core/linux-headers
#core/linux-lts-headers
#extra/linux-hardened-headers
#extra/linux-zen-headers
#community/aarch64-linux-gnu-glibc
#community/dietlibc
#community/emscripten
#community/musl
#community/riscv64-linux-gnu-glibc
#[root@f999375f1e86 divine]# pacman -S core/glibc core/linux-headers
#warning: glibc-2.27-3 is up to date -- reinstalling
#resolving dependencies...
#looking for conflicting packages...
#
#Packages (2) glibc-2.27-3  linux-headers-4.17.11-1




#dnf install -y ninja-build
#dnf install -y zlib-devel
# dnf install -y gcc gcc-c++ make ninja libedit-devel
# cmake                                                                        x86_64                                                 3.11.2-1.fc28                                                              updates                                                 7.7 M
# gcc-c++                                                                      x86_64                                                 8.1.1-5.fc28                                                               updates                                                  12 M
# libedit-devel                                                                x86_64                                                 3.1-23.20170329cvs.fc28                                                    fedora                                                   43 k
# ncurses-devel                                                                x86_64                                                 6.1-5.20180224.fc28                                                        updates                                                 527 k
# ninja-build                                                                  x86_64                                                 1.8.2-1.fc28                                                               fedora                                                  127 k
# perl                                                                         x86_64                                                 4:5.26.2-412.fc28                                                          updates                                                  70 k
# python2                                                                      x86_64                                                 2.7.15-2.fc28                                                              updates                                                 101 k
#Upgrading:
# libgcc                                                                       x86_64                                                 8.1.1-5.fc28                                                               updates                                                  95 k
# ncurses-base                                                                 noarch                                                 6.1-5.20180224.fc28                                                        updates                                                  80 k
# ncurses-libs

# [nix-shell:~/Work/repos/qpid-proton/divine]$ CFLAGS="-I/nix/store/ygx5qy4z8y85ccq68aizjkqn31hpiazg-libc++-6.0.0/include -I/nix/store/7801g3g7s312ply7idnhbhfss341amfc-libc++abi-6.0.0/include -I/nix/store/4gkzvw5sg95h50qldknxf5x2q76p3fm8-compiler-rt-6.0.0-dev/include -I/nix/store/ld95lwk51n982l0vgfqwdh7v445989fk-krb5-1.15.2-dev/include -I/nix/store/fxx4w3v47xq6gdpqy45ln6d09gbx3qn0-gss-1.0.3/include -I/nix/store/z2hnyx3nwaqqfzxkb9clj66sd7xnwiv8-libuv-1.20.3/include -I/nix/store/h7nzh4lj2zy81qqp4jqkrly4wnrdnpja-util-linux-2.32-dev/include -I/nix/store/q239yikz665n4a5rff7rg2vc7jpay6xb-openssl-1.0.2o-dev/include -I/nix/store/f3svdpbdciz3c91nc8bj8qi0adc6p5zl-ruby-2.4.4/include -I/nix/store/d3aa78jf7pm9lk6ad67x4f21a068dxn6-python-2.7.15/include -I/nix/store/6njy58687zgcyyd8r12s3ar39gr3w7z6-cyrus-sasl-2.1.26-dev/include -I/nix/store/y8cfvcvya61l260jil989lcmkia5b5gh-zlib-1.2.11-dev/include -I/nix/store/dibg3zh2jacak3vxdfgcipygp51a2a0l-boost-1.67_0-dev/include -I/nix/store/wmbkg83fsylyk8rqsp663bb2axgmyvl8-clang-wrapper-6.0.0/resource-root/include -I/nix/store/lyd89mv72m8a0aw1a4idfimyi0rb2b13-glibc-2.27-dev/include" cmake .. -DCMAKE_MODULE_PATH=~/Work/repos/qpid-proton/Modules -DCMAKE_SYSTEM_NAME=divine -DLIB_SUFFIX="64" -DCMAKE_TOOLCHAIN_FILE=/home/jdanek/Work/repos/qpid-proton/divine.cmake --debug-trycompile

#this will find epoll
# CFLAGS="-I/nix/store/ygx5qy4z8y85ccq68aizjkqn31hpiazg-libc++-6.0.0/include -I/nix/store/7801g3g7s312ply7idnhbhfss341amfc-libc++abi-6.0.0/include -I/nix/store/4gkzvw5sg95h50qldknxf5x2q76p3fm8-compiler-rt-6.0.0-dev/include -I/nix/store/ld95lwk51n982l0vgfqwdh7v445989fk-krb5-1.15.2-dev/include -I/nix/store/fxx4w3v47xq6gdpqy45ln6d09gbx3qn0-gss-1.0.3/include -I/nix/store/z2hnyx3nwaqqfzxkb9clj66sd7xnwiv8-libuv-1.20.3/include -I/nix/store/h7nzh4lj2zy81qqp4jqkrly4wnrdnpja-util-linux-2.32-dev/include -I/nix/store/q239yikz665n4a5rff7rg2vc7jpay6xb-openssl-1.0.2o-dev/include -I/nix/store/f3svdpbdciz3c91nc8bj8qi0adc6p5zl-ruby-2.4.4/include -I/nix/store/d3aa78jf7pm9lk6ad67x4f21a068dxn6-python-2.7.15/include -I/nix/store/6njy58687zgcyyd8r12s3ar39gr3w7z6-cyrus-sasl-2.1.26-dev/include -I/nix/store/y8cfvcvya61l260jil989lcmkia5b5gh-zlib-1.2.11-dev/include -I/nix/store/dibg3zh2jacak3vxdfgcipygp51a2a0l-boost-1.67_0-dev/include -I/nix/store/wmbkg83fsylyk8rqsp663bb2axgmyvl8-clang-wrapper-6.0.0/resource-root/include -I/nix/store/lyd89mv72m8a0aw1a4idfimyi0rb2b13-glibc-2.27-dev/include -I/nix/store/lyd89mv72m8a0aw1a4idfimyi0rb2b13-glibc-2.27-dev/include -I/nix/store/m3djksc96pkjm84x56nxw7illn7m72cj-glibc-2.27-dev/include" cmake .. -DCMAKE_MODULE_PATH=~/Work/repos/qpid-proton/Modules -DCMAKE_SYSTEM_NAME=divine -DLIB_SUFFIX="64" -DCMAKE_TOOLCHAIN_FILE=/home/jdanek/Work/repos/qpid-proton/divine.cmake
#--debug-trycompile

#static
# compiling CMakeFiles/qpid-proton.dir/src/core/object/object.c.bc
# error: unknown argument: '-shared'


# ssl and crypto must go
# ~/Work/repos/qpid-proton/divine/c/examples]$ /home/jdanek/Work/repos/qpid-proton/compiler.bash -I/nix/store/ygx5qy4z8y85ccq68aizjkqn31hpiazg-libc++-6.0.0/include -I/nix/store/7801g3g7s312ply7idnhbhfss341amfc-libc++abi-6.0.0/include -I/nix/store/4gkzvw5sg95h50qldknxf5x2q76p3fm8-compiler-rt-6.0.0-dev/include -I/nix/store/ld95lwk51n982l0vgfqwdh7v445989fk-krb5-1.15.2-dev/include -I/nix/store/fxx4w3v47xq6gdpqy45ln6d09gbx3qn0-gss-1.0.3/include -I/nix/store/z2hnyx3nwaqqfzxkb9clj66sd7xnwiv8-libuv-1.20.3/include -I/nix/store/h7nzh4lj2zy81qqp4jqkrly4wnrdnpja-util-linux-2.32-dev/include -I/nix/store/q239yikz665n4a5rff7rg2vc7jpay6xb-openssl-1.0.2o-dev/include -I/nix/store/f3svdpbdciz3c91nc8bj8qi0adc6p5zl-ruby-2.4.4/include -I/nix/store/d3aa78jf7pm9lk6ad67x4f21a068dxn6-python-2.7.15/include -I/nix/store/6njy58687zgcyyd8r12s3ar39gr3w7z6-cyrus-sasl-2.1.26-dev/include -I/nix/store/y8cfvcvya61l260jil989lcmkia5b5gh-zlib-1.2.11-dev/include -I/nix/store/dibg3zh2jacak3vxdfgcipygp51a2a0l-boost-1.67_0-dev/include -I/nix/store/wmbkg83fsylyk8rqsp663bb2axgmyvl8-clang-wrapper-6.0.0/resource-root/include -I/nix/store/lyd89mv72m8a0aw1a4idfimyi0rb2b13-glibc-2.27-dev/include -I/nix/store/lyd89mv72m8a0aw1a4idfimyi0rb2b13-glibc-2.27-dev/include -I/nix/store/m3djksc96pkjm84x56nxw7illn7m72cj-glibc-2.27-dev/include   CMakeFiles/c-send.dir/send.c.bc  -o send ../../c/CMakeFiles/qpid-proton-proactor.dir/src/proactor/epoll.c.bc ../../c/CMakeFiles/qpid-proton-proactor.dir/src/proactor/proactor-internal.c.bc $(find ../../c/CMakeFiles/qpid-proton-core.dir/src -name "*.c.bc")
# /nix/store/adf048j1l1adwx35rd3acpalmjbkv1mk-openssl-1.0.2o/lib/libssl.so /nix/store/adf048j1l1adwx35rd3acpalmjbkv1mk-openssl-1.0.2o/lib/libcrypto.so /nix/store/k9ijaa1xd9fhjaxj4aifzjg1n0cbl04x-cyrus-sasl-2.1.26/lib/libsasl2.so
