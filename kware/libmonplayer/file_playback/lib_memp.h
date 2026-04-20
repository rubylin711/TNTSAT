/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __LIB_MEMP_H__
#define __LIB_MEMP_H__

#define FUNC_REDEF(func)         fp##_##func
#define VARI_REDEF(variable)     fp##_##variable

#define memp_link_t              VARI_REDEF(memp_link_t)
#define memp_cb_t                VARI_REDEF(memp_cb_t)
#define lib_memp_t               VARI_REDEF(lib_memp_t)

#define lib_memp_create          FUNC_REDEF(lib_memp_create)
#define lib_memp_destroy         FUNC_REDEF(lib_memp_destroy)
#define lib_memp_alloc           FUNC_REDEF(lib_memp_alloc)
#define lib_memp_free            FUNC_REDEF(lib_memp_free)
#define lib_memp_resize          FUNC_REDEF(lib_memp_resize)
#define lib_memp_align_alloc     FUNC_REDEF(lib_memp_align_alloc)
#define lib_memp_align_free      FUNC_REDEF(lib_memp_align_free)
/*!
   The macro is defined to enable debug print information or NOT.
  */
#define MEMP_DEBUG    0

/*!
   The structure is defined for memp link information.
  */
struct memp_link;

/*!
   The structure is defined for memp link information.
  */
typedef struct memp_link {
    /*!
       The next node.
      */
    struct memp_link *p_next;
    /*!
       The size of this node.
      */
    size_t size;
} memp_link_t;

/*!
   The structure is defined for memp internal information.
  */
typedef struct {
    /*!
       The piece, points to memp_link.
      */
    memp_link_t *p_piece;
#if MEMP_DEBUG
  /*!
     The address of this piece.
    */
  unsigned long address;
  /*!
     The size of this piece.
    */
  int size;
#endif
} memp_cb_t;

/*!
   The macro is defined to specifies the minimum size of a piece.
  */
#define MEMP_MIN_SIZE             (16)

/*!
   The macro is defined for calculate the size of header of memp.
  */
#define MEMP_HEADER_SIZE          sizeof(memp_cb_t)

/*!
   The macro is defined for calculate the size of a picec header of memp.
  */
#define MEMP_PIECE_HEADER_SIZE    sizeof(memp_link_t)

/*!
   The macro is MEMP object size.
  */
#define LIB_MEMP_SIZE           (MEMP_HEADER_SIZE / sizeof(u32))
/*!
   The macro is MEMP slice size for aligning.
  */
#define LIB_MEMP_SLICE_ALIGN    (16)

/*!
   Define memory partition control data with all internal
   information hidden.
  */
typedef struct {
    /*!
       The words of object to hide the internal information
      */
    u32 words[LIB_MEMP_SIZE];
} lib_memp_t;


/*!
   Initialize a memory partition

   \param[out] p_memp Points to the MEMF object.
   \param[in] buffer Specifies the address of the partition buffer.
   \param[in] size Specifies the size of the partition buffer.

   \return if successful, it return SUCCESS, Otherwise returns ERR_PARAM.
  */
RET_CODE lib_memp_create(lib_memp_t *p_memp, void *buffer, size_t size);

/*!
   Destroy a memory partition

   \param[in] p_memp Points to the MEMF object.

   \return if successful, it return SUCCESS, Otherwise returns ERR_PARAM.
  */
RET_CODE lib_memp_destroy(lib_memp_t *p_memp);

/*!
   Allocate buffer from memory partition

   \param[in] p_memp Points to the MEMF object.
   \param[in] size Specifies the size of the buffer to be allocated.

   \return if successful, it return a pointer to allocated buffer,
           Otherwise returns NULL.
  */
void *lib_memp_alloc(lib_memp_t *p_memp, size_t size);

/*!
   Free buffer to memory partition

   \param[in] p_memp Points to the MEMF object.
   \param[in] buffer Points to the buffer to be freed.
  */
RET_CODE lib_memp_free(lib_memp_t *p_memp, void *p_piece);
#endif
