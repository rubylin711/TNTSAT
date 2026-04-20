
export PLAYREADY_ROOT=$PWD
export PLAYREADY_DIR=$PLAYREADY_ROOT/source/linux
export PLAYREADY_PROFILE=drmprofilelinux.mk
export LINUX_BUILD=1
export _NTROOT=$PLAYREADY_ROOT
export PLAYREADY_BUILD_TYPE=FREE
export GCC_OPTIMIZATION_LEVEL=3
export DRM_BUILD_ARCH=MIPS
export DRM_BUILD_PROFILE=901
export PLAYREADY_GXX=/usr/local/crosstool-ng/gcc-9.3-glibc-2.28-mipsel-linux-gnu-rm2.0/bin/mipsel-linux-gnu-gcc
export LIBPATHS="-L/home/zchen/wanda_symphony/montage-tech/sdk/trunk/lib/concerto -L$PLAYREADY_ROOT/bin/lib"
#export PKG_CONFIG_PATH=$HOME/linux-sdk/vendor/montage/sdk/sdkproduct/middleware/opensource/curl-7.57.0/install/lib:$PKG_CONFIG_PATH
export PL_OBJ_DIR=$PLAYREADY_ROOT/pk4.0_out

if [ ! -d "./PK/msi" ];then
	mkdir -p PK
	ln -s $PLAYREADY_ROOT/source/linux ./PK/msi	
fi

