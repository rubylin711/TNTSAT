
/** @defgroup USER_MODE In user mode
    @brief The following interfaces should be only called in user mode. */

/** @{ */

/** @defgroup iMODULE_MGR Module pool
    @brief Module manager pool interfaces just only for innter */
/** @defgroup oMODULE_MGR Module manager
    @brief Module manager interfaces for other modules */

/** @defgroup iMEM Memory pool
    @brief Memory pool manager initialize interfaces */
/** @defgroup oMEM Memory
    @brief System heap memory allocate interfaces for other modules */

/** @defgroup iMMZ MMZ pool
    @brief MMZ pool interfaces just only for inner modules */
/** @defgroup oMMZ MMZ
    @brief MMZ memory allocate interfaces for other modules. */

/** @} */

/** @addtogroup oMODULE_MGR */
/** @{ */
#ifndef __MT_MODULE_MGR_H__
#define __MT_MODULE_MGR_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "mt_type.h"

/**
@brief Initialize this manager moudle .
@attention Before calling other interfaces of this module, calling this interface.
@param[in] None
@param[out] None
@retval ::MT_SUCCESS Success
@retval ::MT_FAILURE Failure
@see \n
N/A
*/
mt_s32 mt_module_init(mt_void);

/**
@brief Terminate this manager moudle
@attention None
@param[in] None
@param[out] None
@retval ::MT_SUCCESS Success
@retval ::MT_FAILURE Failure
@see \n
N/A
*/
mt_s32 mt_module_deinit(mt_void);

/**
@brief Get the module ID that has been registered. 
@attention None
@param[in] pu8ModuleName The module name
@param[out] None
@retval ::The valid module ID, which has been registered
@retval ::MT_FAILURE Failure
@see \n
N/A
*/
mt_s32 mt_module_get_module_id(const mt_u8* pu8ModuleName);

#ifdef __cplusplus
}
#endif

#endif

