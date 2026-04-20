##################################
# mtgo source
##################################
src_mtgo :=	\
		mtgo/src/adp_gfx.c			\
		mtgo/src/adp_jpeg.c			\
		mtgo/src/adp_layer.c		\
		mtgo/src/adp_mtfb.c			\
		mtgo/src/mt_go_bliter.c		\
		mtgo/src/mt_go_comm.c		\
		mtgo/src/mt_go_decoder.c	\
		mtgo/src/mt_go_gdev.c		\
		mtgo/src/mt_go_surface.c	\
		mtgo/src/mt_go_text.c		\
		mtgo/src/mtgo_blit.c		\
		mtgo/src/mtgo_decbmp.c		\
		mtgo/src/mtgo_decgif.c		\
		mtgo/src/mtgo_io.c			\
		mtgo/src/mtgo_memory.c		\
		mtgo/src/mtgo_memsurface.c	\
		mtgo/src/mtgo_surface.c		\
		mtgo/src/uni2gb.c

#		mtgo/src/adp_png.c			\
#		mtgo/src/mt_api_mmz.c		\

##################################
# mtgo include
##################################
MSP_INC += $(LOCAL_PATH)/mtgo/include
