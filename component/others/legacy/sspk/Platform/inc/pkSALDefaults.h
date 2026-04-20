///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

/*
 *  SALDefs.h -- provide default no-op definitions of the SAL annotations so
 *               each platform does not need to provide them for us.  On platforms
 *               which have real SAL headers, those can be included by platBase.h,
 *               which is ahead of this file in pkPAL.h
 */

// Instead of using #pragma once, use #ifndef __PKSALDEFAULTS_H_. This way, a platform can suppress this
// file entirely via #define __PKSALDEFAULTS_H_ in their platBase.h.
#ifndef __PKSALDEFAULTS_H_
#define __PKSALDEFAULTS_H_

// Disable expansion of SAL macros

#ifndef _In_
#define _In_
#endif

#ifndef _In_opt_
#define _In_opt_
#endif

#ifndef _In_z_
#define _In_z_
#endif

#ifndef _In_opt_z_
#define _In_opt_z_
#endif

#ifndef _In_count_
#define _In_count_(size)
#endif

#ifndef _In_opt_count_
#define _In_opt_count_(size)
#endif

#ifndef _In_bytecount_
#define _In_bytecount_(size)
#endif

#ifndef _In_opt_bytecount_
#define _In_opt_bytecount_(size)
#endif


#ifndef _In_count_c_
#define _In_count_c_(size)
#endif

#ifndef _In_opt_count_c_
#define _In_opt_count_c_(size)
#endif

#ifndef _In_bytecount_c_
#define _In_bytecount_c_(size)
#endif

#ifndef _In_opt_bytecount_c_
#define _In_opt_bytecount_c_(size)
#endif


#ifndef _In_z_count_
#define _In_z_count_(size)
#endif

#ifndef _In_opt_z_count_
#define _In_opt_z_count_(size)
#endif

#ifndef _In_z_bytecount_
#define _In_z_bytecount_(size)
#endif

#ifndef _In_opt_z_bytecount_
#define _In_opt_z_bytecount_(size)
#endif


#ifndef _In_z_count_c_
#define _In_z_count_c_(size)
#endif

#ifndef _In_opt_z_count_c_
#define _In_opt_z_count_c_(size)
#endif

#ifndef _In_z_bytecount_c_
#define _In_z_bytecount_c_(size)
#endif

#ifndef _In_opt_z_bytecount_c_
#define _In_opt_z_bytecount_c_(size)
#endif


#ifndef _In_ptrdiff_count_
#define _In_ptrdiff_count_(size)
#endif

#ifndef _In_opt_ptrdiff_count_
#define _In_opt_ptrdiff_count_(size)
#endif


#ifndef _In_count_x_
#define _In_count_x_(size)
#endif

#ifndef _In_opt_count_x_
#define _In_opt_count_x_(size)
#endif

#ifndef _In_bytecount_x_
#define _In_bytecount_x_(size)
#endif

#ifndef _In_opt_bytecount_x_
#define _In_opt_bytecount_x_(size)
#endif


#ifndef _Out_
#define _Out_
#endif

#ifndef _Out_opt_
#define _Out_opt_
#endif


#ifndef _Out_cap_
#define _Out_cap_(size)
#endif

#ifndef _Out_opt_cap_
#define _Out_opt_cap_(size)
#endif

#ifndef _Out_bytecap_
#define _Out_bytecap_(size)
#endif

#ifndef _Out_opt_bytecap_
#define _Out_opt_bytecap_(size)
#endif


#ifndef _Out_cap_c_
#define _Out_cap_c_(size)
#endif

#ifndef _Out_opt_cap_c_
#define _Out_opt_cap_c_(size)
#endif

#ifndef _Out_bytecap_c_
#define _Out_bytecap_c_(size)
#endif

#ifndef _Out_opt_bytecap_c_
#define _Out_opt_bytecap_c_(size)
#endif


#ifndef _Out_cap_m_
#define _Out_cap_m_(mult,size)
#endif

#ifndef _Out_opt_cap_m_
#define _Out_opt_cap_m_(mult,size)
#endif

#ifndef _Out_z_cap_m_
#define _Out_z_cap_m_(mult,size)
#endif

#ifndef _Out_opt_z_cap_m_
#define _Out_opt_z_cap_m_(mult,size)
#endif


#ifndef _Out_ptrdiff_cap_
#define _Out_ptrdiff_cap_(size)
#endif

#ifndef _Out_opt_ptrdiff_cap_
#define _Out_opt_ptrdiff_cap_(size)
#endif


#ifndef _Out_cap_x_
#define _Out_cap_x_(size)
#endif

#ifndef _Out_opt_cap_x_
#define _Out_opt_cap_x_(size)
#endif

#ifndef _Out_bytecap_x_
#define _Out_bytecap_x_(size)
#endif

#ifndef _Out_opt_bytecap_x_
#define _Out_opt_bytecap_x_(size)
#endif


#ifndef _Out_z_cap_
#define _Out_z_cap_(size)
#endif

#ifndef _Out_opt_z_cap_
#define _Out_opt_z_cap_(size)
#endif

#ifndef _Out_z_bytecap_
#define _Out_z_bytecap_(size)
#endif

#ifndef _Out_opt_z_bytecap_
#define _Out_opt_z_bytecap_(size)
#endif


#ifndef _Out_z_cap_c_
#define _Out_z_cap_c_(size)
#endif

#ifndef _Out_opt_z_cap_c_
#define _Out_opt_z_cap_c_(size)
#endif

#ifndef _Out_z_bytecap_c_
#define _Out_z_bytecap_c_(size)
#endif

#ifndef _Out_opt_z_bytecap_c_
#define _Out_opt_z_bytecap_c_(size)
#endif


#ifndef _Out_z_cap_x_
#define _Out_z_cap_x_(size)
#endif

#ifndef _Out_opt_z_cap_x_
#define _Out_opt_z_cap_x_(size)
#endif

#ifndef _Out_z_bytecap_x_
#define _Out_z_bytecap_x_(size)
#endif

#ifndef _Out_opt_z_bytecap_x_
#define _Out_opt_z_bytecap_x_(size)
#endif


#ifndef _Out_cap_post_count_
#define _Out_cap_post_count_(cap,count)
#endif

#ifndef _Out_opt_cap_post_count_
#define _Out_opt_cap_post_count_(cap,count)
#endif

#ifndef _Out_bytecap_post_bytecount_
#define _Out_bytecap_post_bytecount_(cap,count)
#endif

#ifndef _Out_opt_bytecap_post_bytecount_
#define _Out_opt_bytecap_post_bytecount_(cap,count)
#endif


#ifndef _Out_z_cap_post_count_
#define _Out_z_cap_post_count_(cap,count)
#endif

#ifndef _Out_opt_z_cap_post_count_
#define _Out_opt_z_cap_post_count_(cap,count)
#endif

#ifndef _Out_z_bytecap_post_bytecount_
#define _Out_z_bytecap_post_bytecount_(cap,count)
#endif

#ifndef _Out_opt_z_bytecap_post_bytecount_
#define _Out_opt_z_bytecap_post_bytecount_(cap,count)
#endif


#ifndef _Out_capcount_
#define _Out_capcount_(capcount)
#endif

#ifndef _Out_opt_capcount_
#define _Out_opt_capcount_(capcount)
#endif

#ifndef _Out_bytecapcount_
#define _Out_bytecapcount_(capcount)
#endif

#ifndef _Out_opt_bytecapcount_
#define _Out_opt_bytecapcount_(capcount)
#endif


#ifndef _Out_capcount_x_
#define _Out_capcount_x_(capcount)
#endif

#ifndef _Out_opt_capcount_x_
#define _Out_opt_capcount_x_(capcount)
#endif

#ifndef _Out_bytecapcount_x_
#define _Out_bytecapcount_x_(capcount)
#endif

#ifndef _Out_opt_bytecapcount_x_
#define _Out_opt_bytecapcount_x_(capcount)
#endif


#ifndef _Out_z_capcount_
#define _Out_z_capcount_(capcount)
#endif

#ifndef _Out_opt_z_capcount_
#define _Out_opt_z_capcount_(capcount)
#endif

#ifndef _Out_z_bytecapcount_
#define _Out_z_bytecapcount_(capcount)
#endif

#ifndef _Out_opt_z_bytecapcount_
#define _Out_opt_z_bytecapcount_(capcount)
#endif


#ifndef _Inout_
#define _Inout_
#endif

#ifndef _Inout_opt_
#define _Inout_opt_
#endif


#ifndef _Inout_z_
#define _Inout_z_
#endif

#ifndef _Inout_opt_z_
#define _Inout_opt_z_
#endif


#ifndef _Inout_count_
#define _Inout_count_(size)
#endif

#ifndef _Inout_opt_count_
#define _Inout_opt_count_(size)
#endif

#ifndef _Inout_bytecount_
#define _Inout_bytecount_(size)
#endif

#ifndef _Inout_opt_bytecount_
#define _Inout_opt_bytecount_(size)
#endif


#ifndef _Inout_count_c_
#define _Inout_count_c_(size)
#endif

#ifndef _Inout_opt_count_c_
#define _Inout_opt_count_c_(size)
#endif

#ifndef _Inout_bytecount_c_
#define _Inout_bytecount_c_(size)
#endif

#ifndef _Inout_opt_bytecount_c_
#define _Inout_opt_bytecount_c_(size)
#endif


#ifndef _Inout_z_count_
#define _Inout_z_count_(size)
#endif

#ifndef _Inout_opt_z_count_
#define _Inout_opt_z_count_(size)
#endif

#ifndef _Inout_z_bytecount_
#define _Inout_z_bytecount_(size)
#endif

#ifndef _Inout_opt_z_bytecount_
#define _Inout_opt_z_bytecount_(size)
#endif


#ifndef _Inout_z_count_c_
#define _Inout_z_count_c_(size)
#endif

#ifndef _Inout_opt_z_count_c_
#define _Inout_opt_z_count_c_(size)
#endif

#ifndef _Inout_z_bytecount_c_
#define _Inout_z_bytecount_c_(size)
#endif

#ifndef _Inout_opt_z_bytecount_c_
#define _Inout_opt_z_bytecount_c_(size)
#endif


#ifndef _Inout_ptrdiff_count_
#define _Inout_ptrdiff_count_(size)
#endif

#ifndef _Inout_opt_ptrdiff_count_
#define _Inout_opt_ptrdiff_count_(size)
#endif


#ifndef _Inout_count_x_
#define _Inout_count_x_(size)
#endif

#ifndef _Inout_opt_count_x_
#define _Inout_opt_count_x_(size)
#endif

#ifndef _Inout_bytecount_x_
#define _Inout_bytecount_x_(size)
#endif

#ifndef _Inout_opt_bytecount_x_
#define _Inout_opt_bytecount_x_(size)
#endif


#ifndef _Inout_cap_
#define _Inout_cap_(size)
#endif

#ifndef _Inout_opt_cap_
#define _Inout_opt_cap_(size)
#endif

#ifndef _Inout_bytecap_
#define _Inout_bytecap_(size)
#endif

#ifndef _Inout_opt_bytecap_
#define _Inout_opt_bytecap_(size)
#endif


#ifndef _Inout_cap_c_
#define _Inout_cap_c_(size)
#endif

#ifndef _Inout_opt_cap_c_
#define _Inout_opt_cap_c_(size)
#endif

#ifndef _Inout_bytecap_c_
#define _Inout_bytecap_c_(size)
#endif

#ifndef _Inout_opt_bytecap_c_
#define _Inout_opt_bytecap_c_(size)
#endif


#ifndef _Inout_cap_x_
#define _Inout_cap_x_(size)
#endif

#ifndef _Inout_opt_cap_x_
#define _Inout_opt_cap_x_(size)
#endif

#ifndef _Inout_bytecap_x_
#define _Inout_bytecap_x_(size)
#endif

#ifndef _Inout_opt_bytecap_x_
#define _Inout_opt_bytecap_x_(size)
#endif


#ifndef _Inout_z_cap_
#define _Inout_z_cap_(size)
#endif

#ifndef _Inout_opt_z_cap_
#define _Inout_opt_z_cap_(size)
#endif

#ifndef _Inout_z_bytecap_
#define _Inout_z_bytecap_(size)
#endif

#ifndef _Inout_opt_z_bytecap_
#define _Inout_opt_z_bytecap_(size)
#endif


#ifndef _Inout_z_cap_c_
#define _Inout_z_cap_c_(size)
#endif

#ifndef _Inout_opt_z_cap_c_
#define _Inout_opt_z_cap_c_(size)
#endif

#ifndef _Inout_z_bytecap_c_
#define _Inout_z_bytecap_c_(size)
#endif

#ifndef _Inout_opt_z_bytecap_c_
#define _Inout_opt_z_bytecap_c_(size)
#endif


#ifndef _Inout_z_cap_x_
#define _Inout_z_cap_x_(size)
#endif

#ifndef _Inout_opt_z_cap_x_
#define _Inout_opt_z_cap_x_(size)
#endif

#ifndef _Inout_z_bytecap_x_
#define _Inout_z_bytecap_x_(size)
#endif

#ifndef _Inout_opt_z_bytecap_x_
#define _Inout_opt_z_bytecap_x_(size)
#endif


#ifndef _Ret_
#define _Ret_
#endif

#ifndef _Ret_opt_
#define _Ret_opt_
#endif


#ifndef _Deref_out_
#define _Deref_out_
#endif

#ifndef _Deref_out_opt_
#define _Deref_out_opt_
#endif

#ifndef _Deref_opt_out_
#define _Deref_opt_out_
#endif

#ifndef _Deref_opt_out_opt_
#define _Deref_opt_out_opt_
#endif


#ifndef _Deref_out_z_
#define _Deref_out_z_
#endif

#ifndef _Deref_out_opt_z_
#define _Deref_out_opt_z_
#endif

#ifndef _Deref_opt_out_z_
#define _Deref_opt_out_z_
#endif

#ifndef _Deref_opt_out_opt_z_
#define _Deref_opt_out_opt_z_
#endif


#ifndef _Check_return_
#define _Check_return_
#endif


#ifndef _Printf_format_string_
#define _Printf_format_string_
#endif

#ifndef _Scanf_format_string_
#define _Scanf_format_string_
#endif

#ifndef _Scanf_s_format_string_
#define _Scanf_s_format_string_
#endif

#ifndef _FormatMessage_format_string_
#define _FormatMessage_format_string_
#endif


#ifndef _Success_
#define _Success_(expr)
#endif


#ifndef _In_bound_
#define _In_bound_
#endif

#ifndef _Out_bound_
#define _Out_bound_
#endif

#ifndef _Ret_bound_
#define _Ret_bound_
#endif

#ifndef _Deref_in_bound_
#define _Deref_in_bound_
#endif

#ifndef _Deref_out_bound_
#define _Deref_out_bound_
#endif

#ifndef _Deref_inout_bound_
#define _Deref_inout_bound_
#endif

#ifndef _Deref_ret_bound_
#define _Deref_ret_bound_
#endif


#ifndef _In_range_
#define _In_range_(lb,ub)
#endif

#ifndef _Out_range_
#define _Out_range_(lb,ub)
#endif

#ifndef _Ret_range_
#define _Ret_range_(lb,ub)
#endif

#ifndef _Deref_in_range_
#define _Deref_in_range_(lb,ub)
#endif

#ifndef _Deref_out_range_
#define _Deref_out_range_(lb,ub)
#endif

#ifndef _Deref_ret_range_
#define _Deref_ret_range_(lb,ub)
#endif


#ifndef _Ret_z_
#define _Ret_z_
#endif

#ifndef _Ret_opt_z_
#define _Ret_opt_z_
#endif


#ifndef _Ret_cap_
#define _Ret_cap_(size)
#endif

#ifndef _Ret_opt_cap_
#define _Ret_opt_cap_(size)
#endif

#ifndef _Ret_bytecap_
#define _Ret_bytecap_(size)
#endif

#ifndef _Ret_opt_bytecap_
#define _Ret_opt_bytecap_(size)
#endif


#ifndef _Ret_cap_c_
#define _Ret_cap_c_(size)
#endif

#ifndef _Ret_opt_cap_c_
#define _Ret_opt_cap_c_(size)
#endif

#ifndef _Ret_bytecap_c_
#define _Ret_bytecap_c_(size)
#endif

#ifndef _Ret_opt_bytecap_c_
#define _Ret_opt_bytecap_c_(size)
#endif


#ifndef _Ret_cap_x_
#define _Ret_cap_x_(size)
#endif

#ifndef _Ret_opt_cap_x_
#define _Ret_opt_cap_x_(size)
#endif

#ifndef _Ret_bytecap_x_
#define _Ret_bytecap_x_(size)
#endif

#ifndef _Ret_opt_bytecap_x_
#define _Ret_opt_bytecap_x_(size)
#endif


#ifndef _Ret_z_cap_
#define _Ret_z_cap_(size)
#endif

#ifndef _Ret_opt_z_cap_
#define _Ret_opt_z_cap_(size)
#endif

#ifndef _Ret_z_bytecap_
#define _Ret_z_bytecap_(size)
#endif

#ifndef _Ret_opt_z_bytecap_
#define _Ret_opt_z_bytecap_(size)
#endif


#ifndef _Ret_count_
#define _Ret_count_(size)
#endif

#ifndef _Ret_opt_count_
#define _Ret_opt_count_(size)
#endif

#ifndef _Ret_bytecount_
#define _Ret_bytecount_(size)
#endif

#ifndef _Ret_opt_bytecount_
#define _Ret_opt_bytecount_(size)
#endif


#ifndef _Ret_count_c_
#define _Ret_count_c_(size)
#endif

#ifndef _Ret_opt_count_c_
#define _Ret_opt_count_c_(size)
#endif

#ifndef _Ret_bytecount_c_
#define _Ret_bytecount_c_(size)
#endif

#ifndef _Ret_opt_bytecount_c_
#define _Ret_opt_bytecount_c_(size)
#endif


#ifndef _Ret_count_x_
#define _Ret_count_x_(size)
#endif

#ifndef _Ret_opt_count_x_
#define _Ret_opt_count_x_(size)
#endif

#ifndef _Ret_bytecount_x_
#define _Ret_bytecount_x_(size)
#endif

#ifndef _Ret_opt_bytecount_x_
#define _Ret_opt_bytecount_x_(size)
#endif


#ifndef _Ret_z_count_
#define _Ret_z_count_(size)
#endif

#ifndef _Ret_opt_z_count_
#define _Ret_opt_z_count_(size)
#endif

#ifndef _Ret_z_bytecount_
#define _Ret_z_bytecount_(size)
#endif

#ifndef _Ret_opt_z_bytecount_
#define _Ret_opt_z_bytecount_(size)
#endif


#ifndef _Ret_valid_
#define _Ret_valid_
#endif

#ifndef _Ret_opt_valid_
#define _Ret_opt_valid_
#endif


#ifndef _Ret_notnull_
#define _Ret_notnull_
#endif

#ifndef _Ret_maybenull_
#define _Ret_maybenull_
#endif

#ifndef _Ret_null_
#define _Ret_null_
#endif


#ifndef _Deref_ret_z_
#define _Deref_ret_z_
#endif

#ifndef _Deref_ret_opt_z_
#define _Deref_ret_opt_z_
#endif


#ifndef _Deref_out_z_cap_c_
#define _Deref_out_z_cap_c_(size)
#endif

#ifndef _Deref_inout_z_cap_c_
#define _Deref_inout_z_cap_c_(size)
#endif

#ifndef _Deref_out_z_bytecap_c_
#define _Deref_out_z_bytecap_c_(size)
#endif

#ifndef _Deref_inout_z_bytecap_c_
#define _Deref_inout_z_bytecap_c_(size)
#endif

#ifndef _Deref_inout_z_
#define _Deref_inout_z_
#endif


// TODO: the following legacy SAL macros are deprecated
//       and should be removed when components code has been updated

#ifndef  __struct_bcount
#define  __struct_bcount(len)
#endif

#ifndef  __field_ecount
#define  __field_ecount(len)
#endif

#ifndef  __field_bcount
#define  __field_bcount(len)
#endif

#ifndef  __out_ecount
#define  __out_ecount(len)
#endif

#ifndef  __out_bcount
#define  __out_bcount(len)
#endif

#ifndef  __in_bcount
#define  __in_bcount(len)
#endif

#ifndef  __in_ecount
#define  __in_ecount(len)
#endif

#ifndef  __inout_bcount
#define  __inout_bcount(len)
#endif

#ifndef  __inout_ecount
#define  __inout_ecount(len)
#endif

#ifndef __out_bcount_full
#define __out_bcount_full(len)
#endif

#ifndef __out_ecount_full
#define __out_ecount_full(len)
#endif

#ifndef __in_bcount_full
#define __in_bcount_full(len)
#endif

#ifndef __in_ecount_full
#define __in_ecount_full(len)
#endif

#ifndef __out_ecount_opt
#define __out_ecount_opt(len)
#endif

#ifndef __out_bcount_opt
#define __out_bcount_opt(len)
#endif

#ifndef __in_ecount_opt
#define __in_ecount_opt(len)
#endif

#ifndef __in_bcount_opt
#define __in_bcount_opt(len)
#endif

#ifndef __in_opt
#define __in_opt
#endif

#ifndef __out_opt
#define __out_opt
#endif

#ifndef __out
#define __out
#endif

#ifndef __in
#define __in
#endif

#ifndef __inout
#define __inout
#endif

#ifndef __format_string
#define __format_string
#endif

#ifndef __deref_inout
#define __deref_inout
#endif

#ifndef __nullterminated
#define __nullterminated
#endif

#ifndef __deref_out
#define __deref_out
#endif

#ifndef __deref_out_opt
#define __deref_out_opt
#endif

#ifndef __deref_opt_out
#define __deref_opt_out
#endif

#ifndef __deref_out_bcount
#define __deref_out_bcount(len)
#endif

#ifndef __inout_opt
#define __inout_opt
#endif

#ifndef __analysis_assume
#define __analysis_assume(len)
#endif

#ifndef __fallthrough
#define __fallthrough
#endif

#endif // __PKSALDEFAULTS_H_
