#!/bin/sh
echo "start"
echo $1
sed -i "s/HI_U64/mt_u64/g" `grep HI_U64 -rl $1`
sed -i "s/HI_U32/mt_u32/g" `grep HI_U32 -rl $1`
sed -i "s/HI_U8/mt_u8/g" `grep HI_U8 -rl $1`
sed -i "s/HI_U16/mt_u16/g" `grep HI_U16 -rl $1`

sed -i "s/HI_S64/mt_s64/g" `grep HI_S64 -rl $1`
sed -i "s/HI_S32/mt_s32/g" `grep HI_S32 -rl $1`
sed -i "s/HI_S16/mt_s16/g" `grep HI_S16 -rl $1`
sed -i "s/HI_S8/mt_s8/g" `grep HI_S8 -rl $1`

sed -i "s/HI_BOOL/MT_BOOL/g" `grep HI_BOOL -rl $1`
sed -i "s/HI_CHAR/MT_CHAR/g" `grep HI_CHAR -rl $1`
sed -i "s/HI_HANDLE/MT_HANDLE/g" `grep HI_HANDLE -rl $1`
sed -i "s/HI_VOID/mt_void/g" `grep HI_VOID -rl $1`

sed -i "s/HI_FALSE/MT_FALSE/g" `grep HI_FALSE -rl $1`
sed -i "s/HI_TRUE/MT_TRUE/g" `grep HI_TRUE -rl $1`

sed -i "s/HI_FAILURE/MT_FAILURE/g" `grep HI_FAILURE -rl $1`
sed -i "s/HI_SUCCESS/MT_SUCCESS/g" `grep HI_SUCCESS -rl $1`

sed -i "s/hi_/mt_/g" `grep hi_ -rl $1`
sed -i "s/HI_/MT_/g" `grep HI_ -rl $1`

sed -i "s/hiUNF_/mtUNF_/g" `grep hiUNF_ -rl $1`
sed -i "s/HIGO_/MTGO_/g" `grep HIGO_ -rl $1`
sed -i "s/HIADP_/MTADP_/g" `grep HIADP_ -rl $1`

sed -i "s/MT_CHAR/mt_char/g" `grep MT_CHAR -rl $1`
echo "finish"

