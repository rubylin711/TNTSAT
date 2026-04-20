/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/**
 \file mt_svr_assa.h
 \brief ASS subtitle parsing
 \author Montage Co., Ltd.
 \date 2008-2018
 \version 1.0
 \author
 \date 2017-11-10
 */

#ifndef _MT_SVR_ASSA_H_
#define _MT_SVR_ASSA_H_

#include <stdio.h>
#include "mt_type.h"
#include "mt_unf_subtouput.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/***************************** Structs Definition ******************************/
/** \addtogroup     Suplayer */
/** @{ */ /** <!--[Suplayer] */

/***************************** Macro Definition ******************************/

/** Number of Styles fields */
/** CNcomment:Styles字段个数 */
#define STYLEMATCH_MAX (24)

/** Number of Events fields */
/** CNcomment:Events字段个数 */
#define EVENTMATCH_MAX (10)

/** Length of the key field in the Format field */
/** CNcomment:出现在format字段内的关键字段长度 */
#define FORMAT_FIELD_LEN (256)

/** Length of the key field in the Events field */
/** CNcomment:出现在Events字段内的关键字长度 */
#define EVENT_FIELD_LEN (128)

/** Maximum length of the Dialog field */
/** CNcomment:Dialog字段的最大长度 */
#define DIALOG_FIELD_LEN (2048)

/**
*When the data structure is extended, ensure that the new members are 4-byte aligned and new members are placed after the existing members.
* Field information structure in the script information
*/
/**
*CNcomment:扩展数据结构的时候请注意,新增成员最好采用等宽字节的成员且放随其后
* Script Info 里的字段信息结构 CNend
*/
typedef struct mtSO_SUBTITLE_ASS_SCRIPTINFO_S
{
    MT_BOOL *pbFlag; /**<Whether the field is a Script Info field.*/ /**< CNcomment:是否为Script Info字段 */

    MT_CHAR *pszTitle; /**<Title (subtitle description)*/ /**< CNcomment:标题,对字幕的描述 */

    MT_CHAR *pszScriptAuthor; /**<Subtitle creator*/ /**< CNcomment:创建字幕的作者 */

    MT_CHAR *pszTranslate; /**<Initial subtitle translator*/ /**< CNcomment:最初翻译字幕的人 */

    MT_CHAR *pszEdition; /**<Initial subtitle editor*/ /**< CNcomment:最初编辑字幕的人 */

    MT_CHAR *pszTimedAuthor; /**<Time line creator*/ /**< CNcomment:最初的时间轴人员 */

    MT_CHAR *pszSynchPoint; /**<Start time of loading subtitles*/ /**< CNcomment:开始加载字幕的时间点 */

    MT_CHAR *pszUpdateAuthor; /**<Subtitle updater*/ /**< CNcomment:对字幕进行更新的人员 */

    MT_CHAR *pszUpdateDetails; /**<Update details*/ /**< CNcomment:更新的具体信息 */

    MT_CHAR *pszScriptType; /**<Version description: v4+/v4*/ /**< CNcomment:版本说明:v4+/v4 */

    MT_CHAR *pszCollisions; /**<How to move subtitles when two subtitles overlap*/ /**< CNcomment:当两条字幕重叠时,如何进行相对移动 */

    MT_CHAR *pszPlayResY; /**<Video height reference standard of the file*/ /**< CNcomment:文件所使用的视频高度参考标准 */

    MT_CHAR *pszPlayResX; /**<Video width reference standard of the file*/ /**< CNcomment:文件所使用的视频宽度参考标准 */

    MT_CHAR *pszPlayDepth; /**<Color depth*/

    MT_CHAR *pszTimer; /**<Speed timer for loading subtitles (in percentage)*/

    MT_CHAR *pszWrapStyle; /**<Defines the default wrapping style.*/

} MT_SO_SUBTITLE_ASS_SCRIPTINFO_S;

/**
*Information structure of the fields in Styles
*The fields must be arranged, and newly added fields must be placed after existing fields.
*The following are supported fields:
* Name, Fontname, Fontsize, PrimaryColour, SecondaryColour,
* TertiaryColour, BackColour, Bold, Italic, Underline,
* StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle,
* Outline, Shadow, Alignment, MarginL, MarginR, MarginV, AlphaLevel, Encoding
*/
/**
* CNcomment:Styles 里面的各个字段信息结构
* 必须按顺序排列各个字段,且在增加扩展字段时,只能放在后面
* 支持的字段:
* Name, Fontname, Fontsize, PrimaryColour, SecondaryColour,
* TertiaryColour, BackColour, Bold, Italic, Underline,
* StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle,
* Outline, Shadow, Alignment, MarginL, MarginR, MarginV, AlphaLevel, Encoding CNend
*/
typedef struct mtSO_SUBTITLE_ASS_STYLE_S
{
    MT_CHAR *pszName;     /**<Style name, case-sensitive*/
    MT_CHAR *pszFontName; /**<Font name in Windows, case-sensitive*/
    MT_CHAR *pszFontSize; /**<Font size*/

    /**The color format contains the alpha channel. (AABBGGRR) */
    MT_CHAR *pszPrimaryColour; /**<Primary color*/           /**< CNcomment:主要颜色 */
    MT_CHAR *pszSecondaryColour; /**<Secondary color*/       /**< CNcomment:次要颜色 */
    MT_CHAR *pszTertiaryColour; /**<Subtitle outline color*/ /**< CNcomment:轮廓颜色 */
    MT_CHAR *pszBackColour; /**<Background color*/           /**< CNcomment:阴影颜色 */

    /** bool value is -1/0=true/false change to 1/0 */
    MT_CHAR *pszBold; /**<-1 indicates bold, and 0 indicate regular.*/      /**< CNcomment:-1 为粗体 , 0 为常规 */
    MT_CHAR *pszItalic; /**<-1 indicates italic, and 0 indicates regular.*/ /**< CNcomment:-1 为斜体 , 0 为常规 */
    MT_CHAR *pszUnderline; /**<[-1 or 0] underscore*/                       /**< CNcomment:[-1 或者 0] 下划线 */
    MT_CHAR *pszStrikeOut; /**< [-1 or 0] strike or strikeout*/             /**< CNcomment:[-1 或者 0] 中划线/删除线 */

    /**Variables values can be floating-point numbers.*/
    MT_CHAR *pszScaleX; /**<Text scaling width (in percentage)*/                                                                                    /**< CNcomment:修改文字的宽度.为百分数 */
    MT_CHAR *pszScaleY; /**<Text scaling text (in percentage)*/                                                                                     /**< CNcomment:修改文字的高度.为百分数 */
    MT_CHAR *pszSpacing; /**<Text spacing (in pixel)*/                                                                                              /**< CNcomment:文字间的额外间隙.为像素数 */
    MT_CHAR *pszAngle; /**<Rotation angle based on the Z axis. The angle can be a fractional number. The origin is defined by alignment variable.*/ /**< CNcomment:按 Z 轴进行旋转的度数,原点由alignment进行了定义.可以为小数 */
    MT_CHAR *pszBorderStyle; /**< 1 = border+shadow, 3 = background with a single color*/                                                           /**< CNcomment:1=边框+阴影,3=纯色背景. */
    MT_CHAR *pszOutline; /**<Text outline width (in pixel)*/                                                                                        /**< CNcomment:文字轮廓宽度,为像素数 */
    MT_CHAR *pszShadow; /**<Shadow depth (in pixel)*/                                                                                               /**< CNcomment:阴影的深度 , 为像素数 */
    MT_CHAR *pszAlignment; /**<Define the subtitle position.*/                                                                                      /**< CNcomment:定义字幕的位置 */
    MT_CHAR *pszMarginL; /**<Margin between the subtitle area and the left border (in pixel)*/                                                      /**< CNcomment:字幕可出现区域与左边缘的距离,为像素数 */
    MT_CHAR *pszMarginR; /**<Margin between the subtitle area and the right border (in pixel)*/                                                     /**< CNcomment:字幕可出现区域与右边缘的距离,为像素数 */
    MT_CHAR *pszMarginV; /**<Vertical margin*/                                                                                                      /**< CNcomment:垂直距离 */
    MT_CHAR *pszAlphaLevel; /**<This variable is used to define the transparency of ASS subtitles. It is not present in ASS.*/                      /**< CNcomment:SSA 字幕用来定义透明度,Not present in ASS */
    MT_CHAR *pszEncoding; /**<Specify the character set or encoding mode of the font.*/                                                             /**< CNcomment:指明字体的字符集或编码方式 */

    struct mtSO_SUBTITLE_ASS_STYLE_S *pstNext; /**<Point to the next Styles information.*/ /**< CNcomment:指向下一种Styles信息 */

} MT_SO_SUBTITLE_ASS_STYLE_S;

/**Styles field administrative information that is obtained by parsing the ASS subtitle extra data*/
/** CNcomment:ass字幕扩展数据中解析得到的styles字段管理信息 */
typedef struct mtSO_SUBTITLE_ASS_STYLESV4_S
{
    MT_BOOL bVersionIsV4; /**<Whether the version is V4 or V4+. If not, an error occurs.*/ /**< CNcomment:是否为V4或V4+,否则出错 */

    mt_s32 s32StyleMatchTotal; /**<Number of identified file fields*/ /**< CNcomment:文件里被识别的字段个数 */

    mt_s32 s32StyleMatchIndex[STYLEMATCH_MAX]; /**<Information about all fields supported by Styles*/ /**< CNcomment:styles所支持的所有字段信息 */

    MT_SO_SUBTITLE_ASS_STYLE_S *pstStyle; /**<Information about the supported fields in the storage file*/ /**< CNcomment:存储文件中被支持的字段信息 */

} MT_SO_SUBTITLE_ASS_STYLESV4_S;

/**Description of all fields in Dialogue*/
/** CNcomment:Dialogue 里各个字段的描述信息 */
typedef struct mtSO_SUBTITLE_ASS_DIALOG_S
{
    MT_CHAR szMarked[EVENT_FIELD_LEN]; /**<Identifier line or graphics layer*/ /**< CNcomment:标识行/或者为图层 */

    MT_CHAR szStart[EVENT_FIELD_LEN]; /**<Event start time 0:00:00:00 (hour:minute:second:millisecond)*/ /**< CNcomment:事件开始的时间,格式为0:00:00:00(时:分:秒:百分数) */

    MT_CHAR szEnd[EVENT_FIELD_LEN]; /**<Event end time 0:00:00:00 (hour:minute:second:millisecond)*/ /**< CNcomment:事件结束的时间,格式为0:00:00:00(时:分:秒:百分数) */

    MT_CHAR szStyle[EVENT_FIELD_LEN]; /**<Subtitle style*/ /**< CNcomment:该条字幕所使用的风格 */

    MT_CHAR szName[EVENT_FIELD_LEN]; /**<Player's name*/ /**< CNcomment:player's name, 角色名 */

    MT_CHAR szMarginL[EVENT_FIELD_LEN]; /**<Spacing between the subtitle area and the left margin*/ /**< CNcomment:使用新的与左边缘的距离 */

    MT_CHAR szMarginR[EVENT_FIELD_LEN]; /**<Spacing between the subtitle area and the right margin*/ /**< CNcomment:使用新的与右边缘的距离 */

    MT_CHAR szMarginV[EVENT_FIELD_LEN]; /**<If subtitles are at the bottom of the screen, this variable indicates the spacing between the subtitle area and the screen bottom and the value is in the
unit of pixel. If subtitles are at the top of the screen, this variable indicates the spacing between the subtitle area and the screen top and the value is
in the unit of pixel. If subtitles are in the middle of the screen, this variable is ignored.*/ /**< CNcomment:使用新的垂直距离 */

    MT_CHAR szEffect[EVENT_FIELD_LEN]; /**<Transition effect. It can be empty.*/ /**< CNcomment:过渡效果.可以为空值 */

    MT_CHAR szText[DIALOG_FIELD_LEN]; /**<Dialog subtitle area*/ /**< CNcomment:对白字幕区域 */

    /**<List of multi-language subtitles*/ /**< CNcomment:支持多语言字幕的列表 */
    struct mtSO_SUBTITLE_ASS_DIALOG_S *pstNext;

} MT_SO_SUBTITLE_ASS_DIALOG_S;

/**Events field administrative information that is obtained by parsing the ASS subtitle extra data*/
/** CNcomment:ass字幕扩展数据中解析得到的Events字段管理信息 */
typedef struct mtSO_SUBTITLE_ASS_EVENTS_S
{
    mt_s32 s32EventMatchTotal; /**<Number of supported fields*/ /**< CNcomment:被支持的字段总数 */

    mt_s32 as32EventMatchIndex[EVENTMATCH_MAX]; /**<All supported fields*/ /**< CNcomment:所有能支持的所有字段 */

    MT_SO_SUBTITLE_ASS_DIALOG_S *pstDialog; /**<Obtained Dialog information*/ /**< CNcomment:得到的一行Dialog信息 */

} MT_SO_SUBTITLE_ASS_EVENTS_S;

/**Private extra information required for internal parsing*/
/** CNcomment:内部解析所要用到的私有辅助信息 */
typedef struct tagSO_SUBTITLE_ASS_EXTRADATA_S
{
    MT_CHAR *pszExtraData; /**<Subtitle extra data that is being parsing*/ /**< CNcomment:当前正在解析的字幕扩展数据 */

    mt_s32 s32ExtraDataSize; /**<Extra data size*/ /**< CNcomment:扩展数据的长度 */

} SO_SUBTITLE_ASS_EXTRADATA_S;

/**All resources for managing ASS subtitles*/
/** CNcomment:用于管理解析ASS字幕所用到的所有资源 */
typedef struct mtSO_SUBTITLE_ASS_PARSE_S
{
    MT_SO_SUBTITLE_ASS_SCRIPTINFO_S *pstScript; /**<Subtitle Script information*/ /**< CNcomment:字幕的Script Info信息 */

    MT_SO_SUBTITLE_ASS_STYLESV4_S stStyleV4; /**<Subtitle Styles information*/ /**< CNcomment:字幕的Styles信息 */

    MT_SO_SUBTITLE_ASS_EVENTS_S *pstEvents; /**<Subtitle Events information*/ /**< CNcomment:字幕的Events信息 */

    SO_SUBTITLE_ASS_EXTRADATA_S stAssExtra; /**<Subtitle extra data*/ /**< CNcomment:字幕的扩展数据信息 */

} MT_SO_SUBTITLE_ASS_PARSE_S;

/** @} */ /** <!-- ==== Structure Definition end ==== */

/******************************* API declaration *****************************/
/** \addtogroup      Suplayer */
/** @{ */ /** <!-- [Suplayer] */

/**
\brief Parses a dialog message to obtain the dialog text. The message includes the dynamic display style of dialog texts. Other attributes are stored in the
handle.
CNcomment:解析出一句Dialog信息,包括对白的动态显示样式,其它属性会被放在句柄中 CNend
\attention \n
None.
\param[in]  pstDialog Dialog message to be parsed. CNcomment:需要解析的Dialog信息 CNend
\param[out] pszOutHand The dialog texts that are obtained by parsing the dialog message is returned. CNcomment:返回字幕中解析得到的Dialog对白数据 CNend

\retval ::MT_SUCCESS    Parsing is successful. CNcomment:解析成功CNend
\retval ::MT_FAILURE    Parsing fails. CNcomment:解析失败CNend
\see \n
None.
*/
mt_s32 MT_SO_SUBTITLE_ASS_GetDialogText(MT_SO_SUBTITLE_ASS_DIALOG_S *pstDialog,
                                        MT_CHAR **pszOutHand, mt_s32 *s32OutLen);

/**
\brief Parses a dialog message. CNcomment:解析出一句Dialog信息,包括对白的动态显示样式,其它属性会被放在句柄中CNend
\attention \n
None.
\param[in]      pstSubInfo Subtitle message to be parsed. CNcomment:需要解析的字幕信息CNend
\param[in]      pstParseHand Parser handle. It must be successfully initialized. CNcomment:解析器句柄, 必须是被初始化成功的句柄CNend
\param[out]     pstEvents The dialog texts obtained by parsing the dialog message are returned. CNcomment:返回字幕中解析得到的Dialog CNend

\retval ::MT_SUCCESS    Parsing is successful. CNcomment:解析成功CNend
\retval ::MT_FAILURE    Parsing fails. CNcomment:解析失败CNend
\see \n
None.
*/
mt_s32 MT_SO_SUBTITLE_ASS_StartParse(const MT_UNF_SO_SUBTITLE_INFO_S *pstSubInfo,
                                     MT_SO_SUBTITLE_ASS_DIALOG_S **pstDialog, MT_SO_SUBTITLE_ASS_PARSE_S *pstParseHand);

/**
\brief Initializes a parser handle. The internal attributes are available only after ::MT_SO_SUBTITLE_ASS_OutputParse is successfully called.
CNcomment:初始化一个解析器句柄,内部的属性需要调用::MT_SO_SUBTITLE_ASS_OutputParse接口成功后才能使用CNend
\attention \n
None.
\param[in]      pstParseHand Parser handle to be initialized. CNcomment:需要初始化的解析器句柄CNend

\retval ::MT_SUCCESS    Initialization is successful. CNcomment:初始化成功CNend
\retval ::MT_FAILURE    Initialization fails. CNcomment:初始化失败CNend
\see \n
None.
*/
mt_s32 MT_SO_SUBTITLE_ASS_InitParseHand(MT_SO_SUBTITLE_ASS_PARSE_S **pstParseHand);

/**
\brief Releases a parser handle. A handle can be released only after it is successfully initialized.
CNcomment:释放一个解析器句柄,必须是被初始化成功后的句柄才能释放CNend
\attention \n
None.
\param[in]      pstParseHand Parser handle to be released. CNcomment:需要释放的解析器句柄CNend

\retval ::MT_SUCCESS   A handle is successfully released. CNcomment:释放成功CNend
\retval ::MT_FAILURE   A handle fails to be released. CNcomment:释放失败CNend
\see \n
None.
*/
mt_s32 MT_SO_SUBTITLE_ASS_DeinitParseHand(MT_SO_SUBTITLE_ASS_PARSE_S *pstParseHand);

/**
\brief Releases a Dialog linked list. CNcomment:释放一个Dialog链表CNend
\attention \n
None.
\param[in]      pstDialog Node of the linked list to be released. CNcomment:需要释放的链表头结点CNend

\retval ::MT_SUCCESS A linked list is successfully released.  CNcomment: 释放成功CNend
\retval ::MT_FAILURE   A linked list fails to be released. CNcomment:释放失败CNend
\see \n
None.
*/
mt_s32 MT_SO_SUBTITLE_ASS_FreeDialog(MT_SO_SUBTITLE_ASS_DIALOG_S **pstDialog);

/**
\brief Prints the extra data corresponding to the ASS subtitles. CNcomment:打印出ASS字幕的扩展数据CNend
\attention \n
None.
\param[in]      pstAssParse ASS parser handle. CNcomment:打印的ASS句柄CNend

\retval :: None
\see \n
None.
*/
MT_VOID MT_SO_SUBTITLE_ASS_PrintExtraData(MT_SO_SUBTITLE_ASS_PARSE_S *pstParseHand);

/** @} */ /** <!-- ==== API declaration end ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* _MT_SVR_ASSA_H_ */
