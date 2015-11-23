sock2serial 用做socket与serial的双工通信桥

 *  Compile steps:
 *          1. Download Toolchain from http://downloads.openwrt.org/barrier_breaker/14.07/ar71xx/generic/OpenWrt-Toolchain-ar71xx-for-mips_34kc-gcc-4.8-linaro_uClibc-0.9.33.2.tar.bz2
 *          2. Extract it to your home
 *          3. Change OPENWRT_HOME in profile(at current dir) file if necessary
 *          4. Execute shell command: source ./profile
 *          5. make
 *  Usage:
 *          If compiled, you will get a sock2serial file, copy it your openwrt system.
 *          ./sock2serial  serial-dev-name        # maybe you will need to execute 'chmod +x ./sock2serial'
 *          e.g.
 *          ./sock2serial  /dev/ttyATH0