compile and link option collocation
-----------------------------------

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
	or
	`architecture_aarch64 + ssp + common_all + debug_all + debug_aarch64 + sanitize_tag_asan`
	or
	`architecture_aarch64 + ssp + common_all + debug_all + debug_aarch64 + sanitize_tsan`
`mips debug && sanitize`
	`architecture_mips + ssp + common_all + debug_all + sanitize_asan`






detail of compile option
------------------------

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
`sanitize_tag_asan`	only aarch64 support tag_asan::
	-D_FORTIFY_SOURCE=0 -fno-tree-vectorize -fsanitize=hwaddress -fsanitize-address-use-after-scope -fno-sanitize=alignment --param hwasan-instrument-allocas=0
`sanitize_tsan`		only aarch64 support tsan::
	-D_FORTIFY_SOURCE=0 -fno-tree-vectorize -fsanitize=thread



static lib compile option and application compile option should add::
	-ffunction-sections -fdata-sections
.. dynamic lib compile option not use
..	-ffunction-sections -fdata-sections
link application should add::
	-Wl,--build-id -Wl,--gc-sections -Wl,-z,now -Wl,-z,relro
link dynamic lib should add::
	-Wl,--build-id -Wl,-z,now -Wl,-z,relro
.. link dynamic lib not use
..	-Wl,--gc-sections
when enable sanitize, if link application or dynamic lib via gcc but not ld, must use option::
	sanitize_asan or sanitize_tag_asan or sanitize_tsan





PIC option
----------

compile dynamic lib, add::
	-fPIC
.. compile dynamic lib, not use
..	-fPIE
.. compile static lib and application, not use
..	-fPIC -fPIE


`debug && perf` and `debug && sanitize`, strip option need::
	--strip-debug
.. nodebug not use any strip option


if you want to use ``-O0``
delete ``-Os`` and ``-O2``, add ``-O0``, delete ``-ftree-vectorize``, add ``-fno-tree-vectorize``, ``-D_FORTIFY_SOURCE=2`` change to ``-D_FORTIFY_SOURCE=0``









link option for ld but not gcc, you only need see the above content if you link by gcc but not ld
-------------------------------------------------------------------------------------------------

`ld_arm`::
	--build-id -z,relro -z,now
`ld_aarch64`::
	-Ttext-segment=0x100400000 --build-id -z,relro -z,now
`ld_mips`::
	-EL --build-id -z,relro -z,now

link dynamic lib, use `ld_arm` or `ld_aarch64` or `ld_mips`

link appliaction need add gc option base on `ld_arm` or `ld_aarch64` or `ld_mips`::
	--gc-sections



sanitize should link system lib
`sanitize_asan`, if you chose this compile option, link option add
	`sanitize_asan_arm`
	or
	`sanitize_asan_aarch64`
	or
	`sanitize_asan_mips`
`sanitize_tag_asan`, if you chose this compile option, link option add::
	-lhwasan
`sanitize_tsan`, if you chose this compile option, link option add::
	-ltsan /usr/local/linaro/arm-gnu-toolchain-12.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/lib64/libtsan_preinit.o

`sanitize_asan_arm`::
	-lasan -lubsan /usr/local/linaro/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/arm-linux-gnueabihf/lib/libasan_preinit.o
`sanitize_asan_mips`::
	-lasan -lubsan /usr/local/crosstool-ng/gcc-9.3-glibc-2.28-mipsel-linux-gnu-rm2.0/mipsel-linux-gnu/libc/lib/libasan_preinit.o
`sanitize_asan_aarch64`::
	-lasan -lubsan /usr/local/linaro/arm-gnu-toolchain-12.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/lib64/libasan_preinit.o
