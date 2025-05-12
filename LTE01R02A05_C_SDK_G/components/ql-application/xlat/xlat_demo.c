/*================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
=================================================================*/
/*=================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

WHEN              WHO         WHAT, WHERE, WHY
------------     -------     -------------------------------------------------------------------------------

=================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_api_datacall.h"
#include "ql_api_xlat.h"

	
#define QL_XLAT_LOG_LEVEL	                QL_LOG_LEVEL_INFO
#ifdef CONFIG_QUEC_PROJECT_FEATURE_XLAT_DEBUG
#define QL_XLAT_LOG(msg, ...)			    QL_LOG(QL_XLAT_LOG_LEVEL, "xt", msg, ##__VA_ARGS__)
#else
#define QL_XLAT_LOG(msg, ...)
#endif
#define XLAT_RESERVED_DEFAULT_ID 			1

typedef struct
{
	int  			  			client_id;
	int  			  			cfg_id;

}qxlat_ctx_t;
qxlat_ctx_t 	xlat_ctx = {0};



static int xlat_func_init(void * arg)
{
	int ret = 0;
	uint32_t enable_clat = 0;
	/*xlat default value, QL_XLAT_DEFAULT_VALUE*/
	
	/*Configuration before activation or dial-up networking*/
	ql_xlat_setopt(XLAT_RESERVED_DEFAULT_ID, QL_XLAT_OPT_CLAT_STATUS, &enable_clat, sizeof(uint32_t));/*Power-down save*/

	ql_xlat_getopt(XLAT_RESERVED_DEFAULT_ID, QL_XLAT_OPT_CLAT_STATUS, &enable_clat, sizeof(uint32_t));
	QL_XLAT_LOG("xlat init enable_clat=%d", enable_clat);

	//QL_XLAT_LOG("xlat init ret=%d", ret);
   	return ret;	
}


int ql_xlat_app_init(void)
{
	//QL_XLAT_LOG("xlat init...");
	return xlat_func_init(NULL);
}

