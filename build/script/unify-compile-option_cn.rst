编译和链接参数搭配
------------------

`arm nodebug`
	`architecture_arm + arm_mode + ssp + common_all + non-debug_all + sanitize_none`
`thumb nodebug`
	`architecture_arm + thumb_mode + ssp + common_all + non-debug_all + sanitize_none`
`aarch64 nodebug`
	`architecture_aarch64 + ssp + common_all + non-debug_all + non_debug_aarch64 + sanitize_none`
`mips nodebug`
	`architecture_mips + ssp + common_all + non-debug_all + sanitize_none`
`arm debug && perf`
	`architecture_arm + arm_mode + ssp + common_all + debug_all + debug_arm_mode + sanitize_none`
`thumb debug && perf`
	`architecture_arm + thumb_mode + ssp + common_all + debug_all + debug_thumb_mode + sanitize_none`
`aarch64 debug && perf`
	`architecture_aarch64 + ssp + common_all + debug_all + debug_aarch64 + sanitize_none`
`mips debug && perf`
	`architecture_mips + ssp + common_all + debug_all + sanitize_none`
`arm debug && sanitize`
	`architecture_arm + arm_mode + ssp + common_all + debug_all + debug_arm_mode + sanitize_asan`
`thumb debug && sanitize`
	`architecture_arm + thumb_mode + ssp + common_all + debug_all + debug_thumb_mode + sanitize_asan`
`aarch64 debug && sanitize`
	`architecture_aarch64 + ssp + common_all + debug_all + debug_aarch64 + sanitize_asan`
	或
	`architecture_aarch64 + ssp + common_all + debug_all + debug_aarch64 + sanitize_tag_asan`
	或
	`architecture_aarch64 + ssp + common_all + debug_all + debug_aarch64 + sanitize_tsan`
`mips debug && sanitize`
	`architecture_mips + ssp + common_all + debug_all + sanitize_asan`






具体编译参数
------------

`architecture_arm`::
	-march=armv7ve -mtune=cortex-a7 -mabi=aapcs-linux -mfloat-abi=hard -mfpu=neon-vfpv4 -mthumb-interwork -D_FILE_OFFSET_BITS=64
`arm_mode`::
	-marm
`thumb_mode`::
	-mthumb
`architecture_aarch64`::
	-march=armv8-a -mtune=cortex-a53 -mabi=lp64 -mfix-cortex-a53-835769 -mfix-cortex-a53-843419 -Wl,-Ttext-segment=0x100400000
`architecture_mips`::
	-msoft-float -EL -D_FILE_OFFSET_BITS=64


`ssp`::
	-fstack-protector-strong


`common_all`::
	-O2 -fno-common -fno-strict-aliasing -D_GNU_SOURCE -Werror=incompatible-pointer-types -Werror=uninitialized -Werror=int-to-pointer-cast -Werror=pointer-to-int-cast -Werror=int-conversion -Werror=missing-parameter-type -Werror=implicit-function-declaration -Werror=builtin-declaration-mismatch -Werror=aggressive-loop-optimizations -Wl,--build-id -Wl,-z,now -Wl,-z,relro


`non-debug_all`::
	-fomit-frame-pointer
`non_debug_aarch64`::
	-momit-leaf-frame-pointer


`debug_all`::
	-g -rdynamic -funwind-tables -fno-omit-frame-pointer -fno-optimize-sibling-calls
`debug_arm_mode`::
	-mapcs-frame
`debug_thumb_mode`::
	-mtpcs-frame -mtpcs-leaf-frame
`debug_aarch64`::
	-mno-omit-leaf-frame-pointer


`sanitize_none`::
	-D_FORTIFY_SOURCE=2 -ftree-vectorize
`sanitize_asan`::
	-D_FORTIFY_SOURCE=0 -fno-tree-vectorize -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=undefined -fno-sanitize=alignment
`sanitize_tag_asan`	只有aarch64才支持tag_asan::
	-D_FORTIFY_SOURCE=0 -fno-tree-vectorize -fsanitize=hwaddress -fsanitize-address-use-after-scope -fno-sanitize=alignment --param hwasan-instrument-allocas=0
`sanitize_tsan`		只有aarch64才支持tsan::
	-D_FORTIFY_SOURCE=0 -fno-tree-vectorize -fsanitize=thread



静态库编译参数和APP编译参数还需要添加::
	-ffunction-sections -fdata-sections
.. 动态库编译参数不要加
..	-ffunction-sections -fdata-sections
链接app需要添加::
	-Wl,--build-id -Wl,--gc-sections -Wl,-z,now -Wl,-z,relro
链接动态库需要添加::
	-Wl,--build-id -Wl,-z,now -Wl,-z,relro
.. 链接动态库不要加
..	-Wl,--gc-sections
开启sanitize时, 如果使用gcc而不是ld来链接应用程序或动态库, 必须添加参数::
	sanitize_asan 或 sanitize_tag_asan 或 sanitize_tsan





PIC配置
-------

编译动态库, 添加::
	-fPIC
.. 编译动态库, 不要加
..	-fPIE
.. 编译静态库和app, 不要加
..	-fPIC -fPIE


`debug && perf` 和 `debug && sanitize` 的strip参数要加::
	--strip-debug
.. nodebug不需要加任何strip参数


如果要使用 ``-O0``
删掉 ``-Os`` 和 ``-O2``, 加上 ``-O0``, 删掉 ``-ftree-vectorize``, 加上 ``-fno-tree-vectorize``, ``-D_FORTIFY_SOURCE=2`` 改为 ``-D_FORTIFY_SOURCE=0``









链接参数, 用于直接使用ld而不是gcc链接时, 如果采用gcc链接就只需要看上面的内容
----------------------------------------------------------------------------

`ld_arm`::
	--build-id -z,relro -z,now
`ld_aarch64`::
	-Ttext-segment=0x100400000 --build-id -z,relro -z,now
`ld_mips`::
	-EL --build-id -z,relro -z,now

链接成动态库使用 `ld_arm` 或 `ld_aarch64` 或 `ld_mips`

链接成app则还需要在 `ld_arm` 或 `ld_aarch64` 或 `ld_mips` 基础上添加::
	--gc-sections



使用sanitize需要链接系统库
`sanitize_asan`, 选用这种编译参数的, 链接参数添加
	`sanitize_asan_arm`
	或
	`sanitize_asan_aarch64`
	或
	`sanitize_asan_mips`
`sanitize_tag_asan`, 选用这种编译参数的, 链接参数添加::
	-lhwasan
`sanitize_tsan`, 选用这种编译参数的, 链接参数添加::
	-ltsan /usr/local/linaro/arm-gnu-toolchain-12.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/lib64/libtsan_preinit.o

`sanitize_asan_arm`::
	 -lasan -lubsan /usr/local/linaro/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/arm-linux-gnueabihf/lib/libasan_preinit.o
`sanitize_asan_mips`::
	-lasan -lubsan /usr/local/crosstool-ng/gcc-9.3-glibc-2.28-mipsel-linux-gnu-rm2.0/mipsel-linux-gnu/libc/lib/libasan_preinit.o
`sanitize_asan_aarch64`::
	-lasan -lubsan /usr/local/linaro/arm-gnu-toolchain-12.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/lib64/libasan_preinit.o
