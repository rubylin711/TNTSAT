/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*!
@~english
@file mt_go_text.h
@brief  Describes the header file of the text module
@details

@~chinese
@file mt_go_text.h
@brief Text模块头文件
@details 矢量字体相关的接口与参数定义
*/

#ifndef __MT_GO_TEXT_H__
#define __MT_GO_TEXT_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Macro Definition ******************************/

/*************************** Structure Definition ****************************/
/*!
@~english the text layout
@~chinese 字体排版方式
*/
typedef enum {
    MTGO_LAYOUT_LEFT = 0x0001,         //!<@~english Horizontally left @~chinese 水平居左
    MTGO_LAYOUT_RIGHT = 0x0002,        //!<@~english Horizontally right @~chinese  水平居右
    MTGO_LAYOUT_HCENTER = 0x0004,      //!<@~english Horizontally right @~chinese  水平居中
    MTGO_LAYOUT_WRAP = 0x0008,         //!<@~english Wrap @~chinese 自动换行
    MTGO_LAYOUT_WORDELLIPSIS = 0x0010, //!<@~english Ellipsis format @~chinese 使用省略号风格
    MTGO_LAYOUT_TOP = 0x0100,          //!<@~english Vertically top @~chinese 垂直居顶
    MTGO_LAYOUT_BOTTOM = 0x0200,       //!<@~english Vertically bottom @~chinese 垂直居底
    MTGO_LAYOUT_VCENTER = 0x0400,      //!<@~english Vertically center @~chinese 垂直居中
    MTGO_LAYOUT_BUTT = 0x8000,
} MTGO_LAYOUT_E;

/*!
@~english the text style
@~chinese 字体风格
*/
typedef enum {
    MTGO_TEXT_STYLE_NORMAL = 0x00, //!<@~english Normal font @~chinese 正常字体
    MTGO_TEXT_STYLE_ITALIC = 0x01, //!<@~english Italic font @~chinese 斜体字体
    MTGO_TEXT_STYLE_BOLD = 0x02,   //!<@~english Bold font @~chinese 粗体字体
    MTGO_TEXT_STYLE_BUTT = 0x80,
} MTGO_TEXT_STYLE_E;

/*!
@~english the font file information
@~chinese 字库文件信息
*/
typedef struct
{
    mt_char *pFontFile; //!<@~english the  font file@~chinese 字库文件
    mt_u32 u32Size;           //!<@~english This value is ignored for the dot-matrix font. @~chinese 如果是点阵字体，则该值被忽略
} MTGO_TEXT_INFO_S;

/*!
@~english Font attributes
@~chinese 字体属性 
*/
typedef struct
{
    mt_u8 Height;   //!<@~english Font height @~chinese 字体高度
    mt_u8 MaxWidth; //!<@~english Maximum font width @~chinese 字体最大宽度
} MTGO_FONTATTR_S;

/*!
@~english Attributes of a text output object
@~chinese 文本输出对像属性 
*/
typedef struct
{
    MT_COLOR BgColor;         //!<@~english Background color @~chinese 背景色
    MT_COLOR FgColor;         //!<@~english Foreground color @~chinese 前景色
    MTGO_FONTATTR_S FontAttr; //!<@~english Font attributes of the character set @~chinese 字符集字体属性
} MTGO_TEXTOUTATTR_S;

/******************************* API declaration *****************************/
/*!
@~english
@addtogroup text_def Text
@{
@brief Text module API description 
*/

/*!
@~chinese
@addtogroup text_def Text
@{
@brief Text模块的API接口
*/

/*!
@~english
@brief call FT_Init_FreeType() to initialize FreeType library
@param N/A

@~chinese
@brief 调用API FT_Init_FreeType() 初始化FreeType 字库
@param 无

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE

@see \n
*/
mt_s32 MT_GO_InitText(mt_void);

/*!
@~english
@brief call FT_Done_FreeType() to clean up font library
@param N/A

@~chinese
@brief 调用FT_Done_FreeType() 清除FreeType 字库
@param 无

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE

@see \n
*/
mt_s32 MT_GO_DeinitText(mt_void);

/*!
@~english
@brief read the specified part of the text file, and copy this part into buffer
@param[in] pFileName, the handler of text file
@param[in] ReadOffset, the offset between text head and the specified part
@param[in] ReadSize, the byte bumber to read
@param[out] pBuf, the handler of text buffer

@~chinese
@brief 获取当前文本的字节数目
@param[in] pFileName, 文本句柄
@param[in] ReadOffset, 文本起始到读取部分的偏移
@param[in] ReadSize, 读取部分的字节数目
@param[out] pSize, 缓存句柄

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE

@see \n
*/
mt_s32 MT_GO_ReadFile(mt_char *pFileName, mt_char *pBuf, mt_u32 ReadOffset, mt_u32 ReadSize);

/*!
@~english
@brief get the byte size of this text file
@param[in] pFileName, the handler of text file
@param[out] pSize, pointer to byte number

@~chinese
@brief 获取当前文本的字节数目
@param[in] pFileName, 文本句柄
@param[out] pSize, 指向字节数的指针

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE

@see \n
*/
mt_s32 MT_GO_GetFileSize(mt_char *pFileName, mt_u32 *pSize);

/*!
@~english
@brief Creates a text output object.
@details If the received file name is in vector font, the font height is set based on the 22-dot array. 

@param[in] pFontFile Font file, the value can't be empty.
@param[out]  pTextOut Handle of a text output object.

@~chinese
@brief 创建文本输出对象
@details 如果传入的是矢量字体文件名,则字体高度按22点阵创建
@param[in] pFontFile 字体文件，不能为空
@param[out]  pTextOut 文本输出对象句柄

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE

@see \n
::MT_GO_DestroyText
*/
mt_s32 MT_GO_CreateText(mt_char *pFontFile, mt_handle *pTextOut);

/*!
@~english
@brief Destroys a text output object.
@param[in] TextOut Handle of a text output object.

@~chinese
@brief 销毁文本输出对象
@param[in] TextOut 文本输出对象句柄

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE

@see \n
::MT_GO_CreateText
*/
mt_s32 MT_GO_DestroyText(mt_handle TextOut);

/*!
@~english
@brief Set the size for each font, in pixel unit.
@param[in] TextOut Handle of a text output object.
@param[in] width Font width.
@param[in] height Font height.

@~chinese
@brief 设置字体高度，以像素为单位
@param[in] TextOut 文本输出对象句柄
@param[in] width 字体宽度.
@param[in] height 字体高度.

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
mt_s32 MT_GO_SetPixelSize(mt_handle TextOut, mt_u32 width, mt_u32 height);

/*!
@~english
@brief Get the size for each font, in pixel unit.
@param[in] TextOut Handle of a text output object.
@param[in] width Font width.
@param[in] height Font height.

@~chinese
@brief 获取字体高度，以像素为单位
@param[in] TextOut 文本输出对象句柄
@param[in] width 字体宽度.
@param[in] height 字体高度.

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
mt_s32 MT_GO_GetPixelSize(mt_handle TextOut, mt_u32 *pWidth, mt_u32 *pHeight);

/*!
@~english
@brief Obtains the attributes of a text output object.
@param[in] TextOut Handle of a text output object.
@param[out] pTextOutAttr  Attributes of a text output object.

@~chinese
@brief 获取文本输出对象属性
@param[in] TextOut 文本输出对象句柄
@param[out] pTextOutAttr 文本输出对象属性

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
mt_s32 MT_GO_GetTextAttr(mt_handle TextOut, MTGO_TEXTOUTATTR_S *pTextOutAttr);

/*!
@~english
@brief Obtains the width and height of text contents
@param[in] TextOut Handle of a text output object
@param[in] pText Text contents
@param[out] pWidth Width of text contents. It cannot be empty
@param[out] pHeight Height of text contents. It cannot be empty

@~chinese
@brief 获取文本内容的宽高
@param[in] TextOut 文本输出对象句柄
@param[in] pText 文本内容
@param[out] pWidth 文本内容宽度，不可为空
@param[out] pHeight 文本内容高度，不可为空

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
mt_s32 MT_GO_GetTextExtent(mt_handle TextOut, const mt_char *pText, mt_s32 *pWidth, mt_s32 *pHeight);

/*!
@~english
@brief Sets to display the background color of a text output object.
@param[in] TextOut Handle of a text output object.
@param[in] bTransparent  Whether to display the background color of a text output object. \n
If the value is set to MT_TRUE, the background color is not displayed. The default value is MT_TRUE.

@~chinese
@brief 设置文本输出对象的背景色是否显示
@param[in] TextOut 文本输出对象句柄
@param[in] bTransparent  文本输出对象背景色是否显示\n
为MT_TRUE, 则不显示，默认为MT_TRUE

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE

@see \n
::MT_GO_SetTextBGColor
*/
mt_s32 MT_GO_SetTextBGTransparent(mt_handle TextOut, MT_BOOL bTransparent);

/*!
@~english
@brief Sets the background color of a text output object.
@details If the target surface is a palette, the color is the palette index.
@param[in] TextOut Handle of a text output object
@param[in] Color  Background color of a text output object

@~chinese
@brief 设置文本输出对象的背景色
@details 如果目标surface是调色板，则Color是调色板索引
@param[in] TextOut 文本输出对象句柄
@param[in] Color  文本输出对象背景色，

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE

@see \n
::MT_GO_SetTextBGTransparent
*/
mt_s32 MT_GO_SetTextBGColor(mt_handle TextOut, MT_COLOR Color);

/*!
@~english
@brief Sets the font color of a text output object.
@detaisl If the target surface is a palette, the color is the palette index.
@param[in] TextOut Handle of a text output object.
@param[in] Color  Font color of a text output object.

@~chinese
@brief 设置文本输出对象的字体颜色
@details 如果目标surface是调色板，则Color是调色板索引
@param[in] TextOut 文本输出对象句柄
@param[in] Color  文本输出对象字体颜色

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
mt_s32 MT_GO_SetTextColor(mt_handle TextOut, MT_COLOR Color);

/*!
@~english
@brief Outputs text contents to a specified surface.
@param[in] TextOut Handle of a text output object.
@param[in] Surface Surface handle.
@param[in] pText  Text contents.
@param[in] pRect Text output region. If the value is empty, it indicates that text contents are output from the 
surface origin.

@~chinese
@brief 输出文本内容到指定surface

@param[in] TextOut 文本输出对象句柄
@param[in] Surface Surface句柄
@param[in] pText 文本内容
@param[in] pRect 文本输出区域，为空表示从surface原点输出

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
mt_s32 MT_GO_TextOut(mt_handle TextOut, mt_handle Surface, const mt_char *pText,
                     const MT_RECT *pRect);

/*!
@~english
@brief Outputs texts. This API is an extended API. You can call this API to output contents based on customized 
styles and formatting.
@param[in] hTextOut Text output handle.
@param[in] hSurface Surface handle.
@param[in] pText    Text contents ending with /0.
@param[in] pRect    Text output region.
@param[in] Style    Styles and formatting including LAYOUT_LEFT, LAYOUT_RIGHT, and LAYOUT_HCENTER.

@~chinese
@brief 文本输出扩展接口,输出内容可以按照用户指定的排版进行输出。
@param[in] hTextOut 文本输出句柄
@param[in] hSurface Surface句柄
@param[in] pText 以/0结尾的文本内容
@param[in] pRect 文本输出区域
@param[in] Style 排版格式，如LAYOUT_LEFT、LAYOUT_RIGHT、LAYOUT_HCENTER等

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
mt_s32 MT_GO_TextOutEx(mt_handle hTextOut, mt_handle hSurface,
                       const mt_char *pText, const MT_RECT *pRect,
                       mt_u32 Style);

/*!
@~english
@brief Creates a font. an extended api for MT_GO_CreateText, it can create font based on customized font size.
@param[in] phText Text output handle.
@param[in] pInfo  Text information.

@~chinese
@brief 创建字体,MT_GO_CreateText的扩展，可以按用户指定的字体大小进行创建

@param[in] phText CNcomment:文本输出句柄
@param[in] pInfo  文件信息

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
mt_s32 MT_GO_CreateTextEx(const MTGO_TEXT_INFO_S *pInfo, mt_handle *phText);

/*!
@~english
@brief display the specified text and auto-wrap it
@param[in] hTextOut, handler of the text object
@param[in] hSurface, handler of the surface object
@param[in] pText, pointer to the text that is going to display
@param[in] pRect, pointer to the rectangle region that is going to display
@param[in] Style, text display style
@param[in] LineWidth, line width restriction
@param[in] PreLoadByteNum, pre-loadeded text content
@param[out] pByteNum, pointer to the byte number displayed within this line

@~chinese
@brief 显示当前文本并自动换行
@param[in] hTextOut, 文本对象句柄
@param[in] hSurface, Surface 对象句柄
@param[in] pText, 指向将被显示文本的指针
@param[in] pRect, 指向将被显示区域块的指针
@param[in] Style, 文本显示风格
@param[in] LineWidth, 行长限制
@param[in] PreLoadByteNum, 预加载的文本字节数目
@param[out] pByteNum, 已被显示的当前行文本字节数目

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE

@see \n
*/
mt_s32 MT_GO_TextOutEx_Clip(mt_handle hTextOut, mt_handle hSurface,
                            mt_u8 *pText, const MT_RECT *pRect,
                            mt_u32 Style, mt_u32 LineWidth,
                            mt_u32 PreLoadByteNum, mt_u32 *pByteNum);
mt_s32 MT_GO_TextOut_Clip(mt_handle TextOut, mt_handle Surface, const mt_u8 *pText,
                          const MT_RECT *pRect, mt_u32 LineWidth, mt_u32 PreLoadByteNum, mt_u32 *pByteNum);

/*! 
@~english
@brief Sets the style of a font, such as bold, italic, or normal;\n
MTGO_LAYOUT_WRAP and MTGO_LAYOUT_WORDELLIPSIS not supported yet.

@param[in] hTextOut Text output handle.
@param[in] eStyle   Font style.

@~chinese
@brief 设置字体的风格，比如:粗体，斜体，正常体\n
MTGO_LAYOUT_WRAP and MTGO_LAYOUT_WORDELLIPSIS目前不支持

@param[in] hTextOut 文本输出句柄
@param[in] eStyle 字体风格

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
mt_s32 MT_GO_SetTextStyle(mt_handle hTextOut, MTGO_TEXT_STYLE_E eStyle);

/*! 
@~english
@brief Sets the speed and rounds for text scrolling 

@param[in] TextOut Text output handle.
@param[in] rounds Scroll rounds, if set to 0, the scroll will not stop after creating until the destroy api is called.
@param[in] speed Scroll speed, the text will move (speed) pixels per 40ms, 2 or 3 recommanded.

@~chinese
@brief 设置滚动字幕的循环次数与移动速度

@param[in] TextOut 文本输出句柄
@param[in] rounds 滚动循环次数，如果设为0，在创建后将一直滚动，除非调用销毁接口
@param[in] speed 滚动速度，每40ms移动speed个像素点，推荐设为2或3

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
mt_s32 MT_GO_SetScroll(mt_handle TextOut, mt_u32 rounds, mt_u32 speed);

/*! 
@~english
@brief Create scrolling text  

@param[in] hTextOut Text output handle.
@param[in] Surface Surface handle.
@param[in] pText    Text contents ending with /0.
@param[in] pRect    Scrolling text output region.

@~chinese
@brief 创建滚动字幕

@param[in] hTextOut 文本输出句柄
@param[in] Surface Surface句柄
@param[in] pText 以/0结尾的文本内容
@param[in] pRect 滚动字幕输出区域

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
mt_s32 MT_GO_CreateTextScroll(mt_handle hTextOut, mt_handle hSurface, const mt_char *pText, const MT_RECT *pRect);

/*! 
@~english
@brief Destory scrolling text  

@param[in] hTextOut Text output handle.
@param[in] Surface Surface handle.

@~chinese
@brief 销毁滚动字幕

@param[in] hTextOut 文本输出句柄
@param[in] Surface Surface句柄

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
mt_s32 MT_GO_DestroyTextScroll(mt_handle hTextOut, mt_handle hSurface);

/*!
@~english
@brief Set the horizontal spacing between displayed characters, that is, the horizontal spacing between character 
lines.
@param[in] hTextOut Text output handle.
@param[in] u32Distance Horizontal spacing.

@~chinese
@brief 设置字符显示水平间距，两个字符的水平间距
@param[in] hTextOut 文本输出句柄
@param[in] u32Distance 水平间距

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
mt_s32 MT_GO_SetCharExtra(mt_handle hTextOut, mt_u32 u32Distance);

/*!
@~english
@brief Get the horizontal spacing between displayed characters, that is, the horizontal spacing between character 
lines.
@param[in] hTextOut Text output handle.
@param[in] u32Distance Horizontal spacing.

@~chinese
@brief 获取字符显示水平间距，两个字符的水平间距
@param[in] hTextOut 文本输出句柄
@param[in] u32Distance 水平间距

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
mt_s32 MT_GO_GetCharExtra(mt_handle hTextOut, mt_u32 *pDistance);

/*!
@~english
@brief Set the vertical spacing between displayed characters, that is, the vertical spacing between character 
lines.
@param[in] hTextOut Text output handle.
@param[in] u32Distance Vertical spacing.

@~chinese
@brief 设置字符显示垂直间距，两个字符行的垂直间距
@param[in] hTextOut 文本输出句柄
@param[in] u32Distance 垂直间距

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
mt_s32 MT_GO_SetLineExtra(mt_handle hTextOut, mt_u32 u32Distance);

/*!
@~english
@brief Get the vertical spacing between displayed characters, that is, the vertical spacing between character 
lines.
@param[in] hTextOut Text output handle.
@param[in] u32Distance Vertical spacing.

@~chinese
@brief 获取字符显示垂直间距，两个字符行的垂直间距
@param[in] hTextOut 文本输出句柄
@param[in] u32Distance 垂直间距

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
mt_s32 MT_GO_GetLineExtra(mt_handle hTextOut, mt_u32 *pDistance);

/*!
@~english
@brief Initializes the character module.
@param N/A.

@~chinese
@brief 初始化字符模块
@param 无

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
//mt_s32 MT_GO_InitText();

/*!
@~english
@brief Deinitializes the character module.
@param N/A.

@~chinese
@brief 去初始化字符模块
@param 无

@~
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
//mt_s32 MT_GO_DeinitText();

/*!
@}
*/

mt_s32 MT_GO_DestroyAllTextScrolls(void);

#ifdef __cplusplus
}
#endif
#endif /* __MT_GO_TEXT_H__ */
