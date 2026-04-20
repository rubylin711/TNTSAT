#!/bin/sh
echo "start"
echo $1
sed -i "s/mt_mt_u64/mt_u64/g" `grep mt_u64 -rl $1`
sed -i "s/mt_mt_u32/mt_u32/g" `grep mt_u32 -rl $1`
sed -i "s/mt_mt_u8/mt_u8/g" `grep mt_u8 -rl $1`
sed -i "s/mt_mt_u16/mt_u16/g" `grep mt_u16 -rl $1`

sed -i "s/mt_mt_s64/mt_s64/g" `grep mt_s64 -rl $1`
sed -i "s/mt_mt_s32/mt_s32/g" `grep mt_s32 -rl $1`
sed -i "s/mt_mt_s16/mt_s16/g" `grep mt_s16 -rl $1`
sed -i "s/mt_mt_s8/mt_s8/g" `grep mt_s8 -rl $1`

sed -i "s/MT_MT_BOOL/MT_BOOL/g" `grep MT_BOOL -rl $1`
sed -i "s/mt_char/mt_char/g" `grep mt_char -rl $1`
sed -i "s/MT_HANDLE/MT_HANDLE/g" `grep MT_HANDLE -rl $1`
sed -i "s/mt_mt_void/mt_void/g" `grep mt_void -rl $1`

sed -i "s/MT_FALSE/MT_FALSE/g" `grep MT_FALSE -rl $1`
sed -i "s/MT_TRUE/MT_TRUE/g" `grep MT_TRUE -rl $1`

sed -i "s/MT_FAILURE/MT_FAILURE/g" `grep MT_FAILURE -rl $1`
sed -i "s/MT_SUCCESS/MT_SUCCESS/g" `grep MT_SUCCESS -rl $1`

sed -i "s/mt_/mt_/g" `grep mt_ -rl $1`
sed -i "s/MT_/MT_/g" `grep MT_ -rl $1`

sed -i "s/mtUNF_/mtUNF_/g" `grep mtUNF_ -rl $1`
sed -i "s/MTGO_/MTGO_/g" `grep MTGO_ -rl $1`
sed -i "s/MTADP_/MTADP_/g" `grep MTADP_ -rl $1`

sed -i "s/mt_char/mt_char/g" `grep mt_char -rl $1`

sed -i "s/FALSE/MT_FALSE/g" `grep mt_char -rl $1`
sed -i "s/TRUE/MT_TRUE/g" `grep mt_char -rl $1`
sed -i "s/RET_CODE/mt_u32/g" `grep mt_char -rl $1`

echo "finish"

