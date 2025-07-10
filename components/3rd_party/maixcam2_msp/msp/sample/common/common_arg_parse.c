/**************************************************************************************************
 *
 * Copyright (c) 2019-2024 Axera Semiconductor Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor Co., Ltd.
 *
 **************************************************************************************************/

#include <assert.h>
#include <string.h>
#include "common_arg_parse.h"



#define SAMPLE_LOG(str, arg...)    \
    do {    \
        printf("%s: %s:%d "str"\n", "common_arg_parse.c", __func__, __LINE__, ##arg); \
    } while(0)

#define SAMPLE_ERR_LOG(str, arg...)   \
    do{  \
        printf("%s: %s:%d Error! "str"\n", "common_arg_parse.c", __func__, __LINE__, ##arg); \
    }while(0)


#define CLIP3(x, y, z)  ((z) < (x) ? (x) : ((z) > (y) ? (y) : (z)))



static AX_S32 SampleGetNext(AX_S32 argc, AX_CHAR **argv, SAMPLE_PARAMETER_T *parameter, char **p)
{
    /* End of options */
    if ((parameter->cnt >= argc) || (parameter->cnt < 0)) {
        // SAMPLE_ERR_LOG(" parameter->cnt:%d ", parameter->cnt);
        return -1;
    }
    *p = (char *)argv[parameter->cnt];
    parameter->cnt++;

    return 0;
}

static AX_S32 SampleParse(AX_S32 argc, AX_CHAR **argv, SAMPLE_OPTION_T *option,
                          SAMPLE_PARAMETER_T *parameter, char **p, AX_U32 lenght)
{
    char *arg;
    AX_U32 arg_len = 0;
    AX_S32 ret = 0;

    parameter->short_opt = option->short_opt;
    parameter->longOpt = option->long_opt;
    arg = *p + lenght;

    /* Argument and option are together */
    arg_len = strlen(arg);
    // SAMPLE_LOG("arg_len:%d lenght:%d ", arg_len, lenght);
    if (arg_len != 0) {
        /* There should be no argument */
        if (option->enable == 0) {
            SAMPLE_ERR_LOG(" ");
            return -1;
        }

        /* Remove = */
        if (strncmp("=", arg, 1) == 0) {
            arg++;
        }
        parameter->enable = 1;
        parameter->argument = (AX_CHAR *)arg;
        return 0;
    }

    /* Argument and option are separately */
    ret = SampleGetNext(argc, argv, parameter, p);
    // SAMPLE_LOG("ret:%d ", ret);
    if (ret) {
        /* There is no more parameters */
        if (option->enable == 1) {
            SAMPLE_ERR_LOG(" ");
            return -1;
        }
        return 0;
    }

    /* Parameter is missing if next start with "-" but next time this
    * option is OK so we must fix parameter->cnt */
    ret = strncmp("-", *p,  1);
    // SAMPLE_LOG("ret:%d ", ret);
    if (ret == 0) {
        parameter->cnt--;
        if (option->enable == 1) {
            SAMPLE_ERR_LOG(" ");
            return -1;
        }
        return 0;
    }

    /* There should be no argument */
    if (option->enable == 0) {
        SAMPLE_ERR_LOG(" *p:%s", *p);
        return -1;
    }

    parameter->enable = 1;
    parameter->argument = (AX_CHAR *)*p;

    return 0;
}

static AX_S32 SampleShortOption(AX_S32 argc, AX_CHAR **argv, SAMPLE_OPTION_T *option,
                SAMPLE_PARAMETER_T *parameter, char **p)
{
    AX_S32 i = 0;
    AX_S8 short_opt;

    if (strncmp("-", *p, 1) != 0) {
        return 1;
    }

    //strncpy(&short_opt, *p + 1, 1);
    short_opt = *(*p + 1);
    parameter->short_opt = short_opt;
    while (option[i].long_opt != NULL) {
        if (option[i].short_opt  == short_opt) {
            goto match;
        }
        i++;
    }
    return 1;

match:
    // SAMPLE_LOG("option[i:%d].short_opt:%c", i, option[i].short_opt);
    if (SampleParse(argc, argv, &option[i], parameter, p, 2) != 0) {
        SAMPLE_ERR_LOG(" ");
        return -2;
    }

    return 0;
}

static AX_S32 SampleLongOption(AX_S32 argc, AX_CHAR **argv, SAMPLE_OPTION_T *option,
                SAMPLE_PARAMETER_T *parameter, char **p)
{
    AX_S32 i = 0;
    AX_U32 lenght;
    // AX_U32 pLength;

    if (strncmp("--", *p, 2) != 0) {
        return 1;
    }

    // pLength = strlen(*p+2);
    while (option[i].long_opt != NULL) {
        lenght = strlen((const char *)option[i].long_opt);
        if (strncmp((const char *)option[i].long_opt, *p + 2, lenght) == 0) {
            goto match;
        }
        i++;
    }
    return 1;

match:
    lenght += 2;    /* Because option start -- */
    if (SampleParse(argc, argv, &option[i], parameter, p, lenght) != 0) {
        return -2;
    }

    return 0;
}

AX_S32 SampleGetOption(AX_S32 argc, AX_CHAR **argv, SAMPLE_OPTION_T *option, SAMPLE_PARAMETER_T *parameter)
{
    char *p = NULL;
    AX_S32 ret;

    if (argv == NULL) {
        SAMPLE_ERR_LOG("argv == NULL");
        return -2;
    }

    if (option == NULL) {
        SAMPLE_ERR_LOG("option == NULL");
        return -2;
    }

    if (parameter == NULL) {
        SAMPLE_ERR_LOG("parameter == NULL");
        return -2;
    }

    parameter->argument = (AX_CHAR *)"?";
    parameter->short_opt = '?';
    parameter->enable = 0;

    if (SampleGetNext(argc, argv, parameter, &p)) {
        // SAMPLE_ERR_LOG(" ");
        return -1;  /* End of options */
    }

    /* Long option */
    ret = SampleLongOption(argc, argv, option, parameter, &p);
    if (ret != 1) {
        // SAMPLE_ERR_LOG(" ret:%d ", ret);
        return ret;
    }

    /* Short option */
    ret = SampleShortOption(argc, argv, option, parameter, &p);
    if (ret != 1) {
        // SAMPLE_ERR_LOG(" ret:%d ", ret);
        return ret;
    }

    /* This is unknow option but option anyway so argument must return */
    parameter->argument = (AX_CHAR *)p;

    return 1;
}


AX_S32 SampleParseDelim(AX_CHAR *optArg, AX_CHAR delim)
{
    AX_S32 i;

    for (i = 0; i < (AX_S32)strlen((const char *)optArg); i++) {
        if (optArg[i] == delim) {
            optArg[i] = 0;
            return i;
        }
    }

    return -1;
}


AX_S32 SampleOptionsFill(SAMPLE_OPTION_T *p, int offset, AX_CHAR *long_opt, AX_S8 short_opt, AX_BOOL enable)
{
    if (p == NULL || long_opt == NULL) {
        SAMPLE_ERR_LOG("p == NULL || long_opt == NULL");
        return -1;
    }
    if (p + offset == NULL){
        SAMPLE_ERR_LOG("p + offset:%d == NULL", offset);
        return -1;
    }

    (p + offset)->long_opt = long_opt;
    (p + offset)->short_opt = short_opt;
    (p + offset)->enable = enable;
    return 0;
}


enum ParseMode {
    CFG_MODE_IDLE = 100,
    CFG_MODE_READ_NAME = 101,
    CFG_MODE_READ_VALUE = 102,
    CFG_MODE_NAME_READY = 103,
    CFG_MODE_EAT_COMMENT = 104,
};


enum ParseResult {
    CFG_FMT_OK = 200,
    CFG_FMT_ERR_EXPECTED_ALPHA = 201,
    CFG_FMT_ERR_EXPECTED_BLK_START = 202,
    CFG_FMT_ERR_UNEXPECTED_BLK_START = 203,
    CFG_FMT_ERR_UNEXPECTED_BLK_END = 204,
    CFG_FMT_ERR_UNEXPECTED_VALUE_END = 205,
    CFG_FMT_ERR_UNEXPECTED_LINE_BREAK = 206,
    CFG_FMT_ERR_COMMENT = 207,
};

/* Parse a block from config file. */
static enum ParseResult __ParseBlock(FILE *fid, char *block, AX_U32 *line,
                                   TBCfgCallback callback, void *cb_param) {
    enum ParseMode mode = CFG_MODE_IDLE;
    enum ParseMode prev_mode = CFG_MODE_IDLE;
    enum ParseMode ax_prev_mode = 0;

    enum ParseResult sub_block_res;
    enum TBCfgCallbackResult cb_result;
    char blk[1024];
    char val[1024];
    char escape;
    AX_S32 blk_len = 0;
    AX_S32 val_len = 0;

    if (fid == NULL) {
        SAMPLE_ERR_LOG("fid == NULL");
        return CFG_FMT_ERR_EXPECTED_ALPHA;
    }

    if (callback == NULL) {
        SAMPLE_ERR_LOG("callback == NULL");
        return CFG_FMT_ERR_EXPECTED_ALPHA;
    }

    if (block == NULL) {
        SAMPLE_ERR_LOG("block == NULL");
        return CFG_FMT_ERR_EXPECTED_ALPHA;
    }

    if (line == NULL) {
        SAMPLE_ERR_LOG("line == NULL");
        return CFG_FMT_ERR_EXPECTED_ALPHA;
    }

    if (cb_param == NULL) {
        SAMPLE_ERR_LOG("cb_param == NULL");
        return CFG_FMT_ERR_EXPECTED_ALPHA;
    }

    while (!feof(fid)) {
        char ch = fgetc(fid); /* read char from file */
        if (ch == '#') {
            if (fgetc(fid) == '#') {
                SAMPLE_LOG("Ignore line, because '##', line %d mode:%d.", *line, mode);
                ax_prev_mode = mode;
                mode = CFG_MODE_EAT_COMMENT;
                continue;
            }
        }

        /* Check for comments, if we are not parsing for a key value.
        If 1st comment start mark encountered, check that next
        mark is OK also. Otherwise roll back. */
        if (mode != CFG_MODE_READ_VALUE && ch == '/') {
            if (fgetc(fid) == '*') {
                prev_mode = mode;            /* store previous mode */
                mode = CFG_MODE_EAT_COMMENT; /* comment eating mode */
                continue;
            } else
                fseeko(fid, -1, SEEK_CUR); /* rollback */
        }

        switch (mode) {
        /* Wait for block or value */
        case CFG_MODE_IDLE:
            /* Extra { */
            if (ch == '{') {
                SAMPLE_ERR_LOG("Unexpected {, line %d.\n", *line);
                return CFG_FMT_ERR_UNEXPECTED_BLK_START;
            }
            /* Extra ; */
            else if (ch == ';') {
                SAMPLE_ERR_LOG("Unexpected ;, line %d.\n", *line);
                return CFG_FMT_ERR_UNEXPECTED_VALUE_END;
            }
            /* Block end */
            else if (ch == '}') {
                if (block)
                    return CFG_FMT_OK;
                else {
                    SAMPLE_ERR_LOG("Unexpected }, line %d.\n", *line);
                    return CFG_FMT_ERR_UNEXPECTED_BLK_END;
                }
            }
            /* Whitespace and linefeeds are ignored. */
            else if (isspace(ch) || ch == '\n') {
                if (ch == '\n') (*line)++;
                continue;
            }
            /* Literal name started */
            else {
                blk[blk_len++] = ch;
                mode = CFG_MODE_READ_NAME;
            }
            break;
        /* Read string (block/key name) */
        case CFG_MODE_READ_NAME:
            if (isspace(ch) || ch == '\n') {
                if (ch == '\n') (*line)++;
                blk[blk_len++] = 0;
                mode = CFG_MODE_NAME_READY;
            } else if (!isalnum(ch) && ch != '_') {
                SAMPLE_ERR_LOG("Expected { or = near '%.*s', line %d.\n",
                               blk_len, blk, *line);
                return CFG_FMT_ERR_EXPECTED_ALPHA;
            } else {
                blk[blk_len++] = ch;
            }
            break;
        /* Literal name read, only thing allowed past this is
        whitespace, linefeed, block start or value start */
        case CFG_MODE_NAME_READY:
            /* Ignore white space */
            if (isspace(ch) || ch == '\n') {
                if (ch == '\n') (*line)++;
                continue;
            } else if (ch == '{') {
                /* Notify caller on new block */
                callback(block, blk, NULL, TB_CFG_CALLBACK_BLK_START, cb_param);
                /* Parse sub-block */
                if (CFG_FMT_OK != (sub_block_res = __ParseBlock(fid, blk, line,
                                                                callback, cb_param))) {
                    return sub_block_res; /* Return error */
                }
                blk_len = 0; /* Clear block name */
                mode = CFG_MODE_IDLE;
            } else if (ch == '=') {
                if (!block) { /* no global values */
                    SAMPLE_ERR_LOG("Expected { near '%.*s', line %d.\n",
                                    blk_len, blk, *line);
                    return CFG_FMT_ERR_EXPECTED_BLK_START;
                }
                /* Parse value */
                mode = CFG_MODE_READ_VALUE;
            } else {
                SAMPLE_ERR_LOG("Expected { or = near '%.*s', line %d.\n",
                               blk_len, blk, *line);
                return CFG_FMT_ERR_EXPECTED_BLK_START;
            }
            break;
        /* Reading value. Whitespace ignored if before value. */
        case CFG_MODE_READ_VALUE:
            if (val_len == 0 && isspace(ch)) {
                continue;
            } else if (ch == '\\') {/* escape char */
                escape = fgetc(fid);
                switch (escape) {
                case '\\': /* back-slash */
                    val[val_len++] = '\\';
                    break;
                case '\n': /* line continuation */
                    (*line)++;
                    break;
                default:
                    fseek(fid, -1, SEEK_CUR); /* rollback */
                    break;
                }
            } else if (ch == '\n') {
                SAMPLE_ERR_LOG("Unexpected line break, line %d\n", *line);
                return CFG_FMT_ERR_UNEXPECTED_LINE_BREAK;
            } else if (ch == ';') {
                /* Value read. Invoke callback. */
                val[val_len++] = 0;
                if (TB_CFG_OK !=
                    (cb_result = callback(block, blk, val, TB_CFG_CALLBACK_VALUE,
                                        cb_param))) {
                    SAMPLE_ERR_LOG("Error reading value '%s.%s', line %d\n",
                                   block, blk, *line);
                    return (enum ParseResult)cb_result;
                }
                /* Clear param and value name */
                blk_len = 0;
                val_len = 0;
                mode = CFG_MODE_IDLE;
            } else {
                val[val_len++] = ch;
            }

            break;
        /* Eat comment text */
        case CFG_MODE_EAT_COMMENT:
            /* Check if comment should be terminated */
            switch (ch) {
            case '\n':
                if (ax_prev_mode)
                    mode = ax_prev_mode;
                (*line)++;
                break;
            case '*':
                if (fgetc(fid) == '/')
                    mode = prev_mode;
                else /* Comment doesn't end, rollback */
                    fseek(fid, -1, SEEK_CUR);
            }
            break;
        }
    }
    return CFG_FMT_OK;
}



AX_BOOL TBParseConfig(char *filename, TBCfgCallback callback, void *cb_param)
{
    AX_BOOL result = AX_FALSE;
    AX_U32 lines = 1;
    FILE *fid;

    if (filename == NULL || callback == NULL || cb_param == NULL) {
        SAMPLE_ERR_LOG("filename == NULL || callback == NULL || cb_param == NULL");
        return AX_FALSE;
    }

    fid = fopen(filename, "r");
    if (!fid) {
        SAMPLE_ERR_LOG("Error opening file '%s'.\n", filename);
    } else {
        if (CFG_FMT_OK == __ParseBlock(fid, NULL, &lines, callback, cb_param))
            result = AX_TRUE;
        fclose(fid);
    }

    return result;
}