#ifndef HTTP_COMMON_H
#define HTTP_COMMON_H

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "string_ops.h"

typedef enum http_status
{
    HTTP_RES_OK = 200,
    HTTP_RES_INTERNAL_SERVER_ERR = 500,
    HTTP_RES_BAD_REQUEST = 400,
    HTTP_RES_NOT_FOUND = 404,
} http_status;

#endif /* HTTP_COMMON_H */
