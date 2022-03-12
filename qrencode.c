/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 2007 San Tai (vanilla) Hsu                             |
  +----------------------------------------------------------------------+
  | This source file is subject to the BSD license, that is bundled with |
  | this package in the file LICENSE                                     |
  +----------------------------------------------------------------------+
  | Author:  San Tai (vanilla) Hsu (vanilla@FreeBSD.org>                 |
  +----------------------------------------------------------------------+
*/

/**
 * $file qrencode.c
 * @brief source file of php qrencode extension.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_qrencode.h"
#include "php_open_temporary_file.h"
#include "qrencode_arginfo.h"
#include <qrencode.h>
#define PNG_SKIP_SETJMP_CHECK 1
#include <png.h>

#if PHP_MAJOR_VERSION < 7
typedef zend_rsrc_list_entry zend_resource;
#define QR_ZEND_REGISTER_RESOURCE ZEND_REGISTER_RESOURCE
#define QR_ZEND_FETCH_RESOURCE ZEND_FETCH_RESOURCE
typedef int zend_size_t;
#else
#define QR_ZEND_REGISTER_RESOURCE(return_value, result, le_result)  ZVAL_RES(return_value,zend_register_resource(result, le_result))
#define QR_ZEND_FETCH_RESOURCE(rsrc, rsrc_type, passed_id, default_id, resource_type_name, resource_type) \
    if (!(rsrc = (rsrc_type)zend_fetch_resource(Z_RES_P(*passed_id), resource_type_name, resource_type))) {\
        RETURN_FALSE;\
    }
typedef size_t zend_size_t;
#endif

static int le_qr;

typedef struct {
    QRcode *c;
} php_qrcode;

static void qr_dtor(zend_resource *rsrc);

zend_module_entry qrencode_module_entry = {
#if ZEND_MODULE_API_NO >= 20050922
    STANDARD_MODULE_HEADER_EX,
    NULL,
    NULL,
#else
    STANDARD_MODULE_HEADER,
#endif
    "qrencode",
    ext_functions,
    PHP_MINIT(qrencode),
    PHP_MSHUTDOWN(qrencode),
    NULL,
    NULL,
    PHP_MINFO(qrencode),
    PHP_QRENCODE_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_QRENCODE
ZEND_GET_MODULE(qrencode)
#endif

PHP_MINIT_FUNCTION(qrencode) {
    le_qr = zend_register_list_destructors_ex(qr_dtor, NULL,
            "qr", module_number);

    REGISTER_LONG_CONSTANT("QR_MODE_NUL",
            QR_MODE_NUL, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("QR_MODE_NUM",
            QR_MODE_NUM, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("QR_MODE_AN",
            QR_MODE_AN, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("QR_MODE_8",
            QR_MODE_8, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("QR_MODE_KANJI",
            QR_MODE_KANJI, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("QR_ECLEVEL_L",
            QR_ECLEVEL_L, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("QR_ECLEVEL_M",
            QR_ECLEVEL_M, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("QR_ECLEVEL_Q",
            QR_ECLEVEL_Q, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("QR_ECLEVEL_H",
            QR_ECLEVEL_H, CONST_CS | CONST_PERSISTENT);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(qrencode) {
    return SUCCESS;
}

PHP_MINFO_FUNCTION(qrencode) {
    php_info_print_table_start();
    php_info_print_table_header(2, "qrencode support", "enabled");
    php_info_print_table_end();
}

/* {{{ resource qr_encode (string text, [int version, int level, int mode, int casesensitive]); */
/**
 * @brief main function to encode text to qr code.
 * @param text the string want to encode.
 * @param version qrcode version
 * @param level level
 * @param mode mode, could be QR_MODE_NUM, QR_MODE_AN, QR_MODE_8, QR_MODE_KANJI.
 * @param casesensitive casesentive, if you want the 8 bit mode, must set this on.
 * @return 
 */
PHP_FUNCTION(qr_encode) {
    php_qrcode *qr = NULL;
    long version = 1, level = QR_ECLEVEL_L, mode = QR_MODE_8, casesensitive = 1;
    char *text;
    zend_size_t text_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|llll",
                &text, &text_len, &version, &level, &mode,
                &casesensitive) == FAILURE)
        RETURN_FALSE;

    qr = (php_qrcode *) emalloc(sizeof(php_qrcode));
    if (mode == QR_MODE_8)
        qr->c = QRcode_encodeString8bit(text, version, level);
    else
        qr->c = QRcode_encodeString(text, version, level, mode, casesensitive);

    if (qr->c == NULL) {
        efree(qr);
        RETURN_FALSE;
    }

    QR_ZEND_REGISTER_RESOURCE(return_value, qr, le_qr);
}
/* }}} */

/* {{{ int qr_save (resource link, [string filename, int size, int margin]); */
/**
 * @brief save function for qrencode.
 * @param link qrcode resource link.
 * @param filename filename want to save, it could be empty, and will show the content directly.
 * @param size size
 * @param margin margin.
 * @return 
 */
PHP_FUNCTION(qr_save) {
    zval *link = NULL, *arg2 = NULL, *arg3 = NULL, *arg4 = NULL;
    long size = 3, margin = 4;
    int argc;
    FILE *fp = NULL;
    png_structp png_ptr;
    png_infop info_ptr;
    unsigned char *row, *p, *q;
    int x, y, xx, yy, bit;
    int realwidth, temp_file = 0;
    char buf[4096];
    int b;
#if PHP_MAJOR_VERSION < 7
    char *path;
#else
    zend_string *path;
#endif

    argc = ZEND_NUM_ARGS();

    if (zend_parse_parameters(argc TSRMLS_CC, "r|zzz",
                &link, &arg2, &arg3, &arg4) == FAILURE)
        RETURN_FALSE;

    if (link) {
        php_qrcode *qr = NULL;

        QR_ZEND_FETCH_RESOURCE(qr, php_qrcode *, &link, -1,
                "qr handle", le_qr);

        switch (argc) {
            case 4:
                if (Z_TYPE_P(arg2) == IS_STRING) {
                    // qr_save(resource $link, string $fn, int $size, int $margin);
                    fp = VCWD_FOPEN(Z_STRVAL_P(arg2), "wb");
                    if (!fp) {
                        php_error_docref(NULL TSRMLS_CC, E_WARNING,
                                "Unable to open '%s' for writing.",
                                Z_STRVAL_P(arg2));
                        RETURN_FALSE;
                    }
                    size = Z_LVAL_P(arg3);
                    margin = Z_LVAL_P(arg4);
                } else if (Z_TYPE_P(arg2) == IS_LONG) {
                    // qr_save(resource $link, int $size, int $margin, string $fn);
                    size = Z_LVAL_P(arg2);
                    margin = Z_LVAL_P(arg3);
                    
                    fp = VCWD_FOPEN(Z_STRVAL_P(arg4), "wb");
                    if (!fp) {
                        php_error_docref(NULL TSRMLS_CC, E_WARNING,
                                "Unable to open '%s' for writing.",
                                Z_STRVAL_P(arg4));
                        RETURN_FALSE;
                    }
                }
                break;

            case 3:
                // qr_save(resource $link, int $size, int $margin);
                if (Z_TYPE_P(arg2) != IS_LONG) {
                    php_error_docref(NULL TSRMLS_CC, E_WARNING,
                            "should use type long as argument 2 when pass 3 arguments");
                    RETURN_FALSE;
                }

#if PHP_MAJOR_VERSION < 7
                fp = php_open_temporary_file(NULL, NULL, &path TSRMLS_CC);
#else
                fp = php_open_temporary_file(NULL, NULL, &path);
#endif
                if (!fp) {
                    php_error_docref(NULL TSRMLS_CC, E_WARNING,
                            "Unable to open temporary file for writing.");
                    RETURN_FALSE;
                }

                size = Z_LVAL_P(arg2);
                margin = Z_LVAL_P(arg3);
                temp_file = 1;

                break;
            case 2:
                if (Z_TYPE_P(arg2) == IS_STRING) {
                    fp = VCWD_FOPEN(Z_STRVAL_P(arg2), "wb");
                    if (!fp) {
                        php_error_docref(NULL TSRMLS_CC, E_WARNING,
                                "Unable to open '%s' for writing.",
                                Z_STRVAL_P(arg2));
                        RETURN_FALSE;
                    }
                } else {
                        php_error_docref(NULL TSRMLS_CC, E_WARNING,
                                "should use type string as argument 2");
                        RETURN_FALSE;
                }
                break;

            case 1:
#if PHP_MAJOR_VERSION < 7
                fp = php_open_temporary_file(NULL, NULL, &path TSRMLS_CC);
#else
                fp = php_open_temporary_file(NULL, NULL, &path);
#endif

                if (!fp) {
                    php_error_docref(NULL TSRMLS_CC, E_WARNING,
                            "Unable to open temporary file for writing.");
                    RETURN_FALSE;
                }
                temp_file = 1;

                break;

            case 0:
            default:
                RETURN_FALSE;
        }

        realwidth = (qr->c->width + margin * 2) * size;
        row = (unsigned char *)emalloc((realwidth + 7) / 8);

        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                NULL, NULL, NULL);
        if (png_ptr == NULL) {
            fclose(fp);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Failed to initialize PNG writer.");
            RETURN_FALSE;
        }

        info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == NULL) {
            fclose(fp);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Failed to initialize PNG writer.");
            RETURN_FALSE;
        }


        if (setjmp(png_jmpbuf(png_ptr))) {
            png_destroy_write_struct(&png_ptr, &info_ptr);
            fclose(fp);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Failed to write PNG image.");
            RETURN_FALSE;
        }

        png_init_io(png_ptr, fp);

        png_set_IHDR(png_ptr, info_ptr, realwidth, realwidth, 1,
                PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png_ptr, info_ptr);

        memset(row, 0xff, (realwidth + 7) / 8);
        for (y = 0; y < margin * size; y++)
            png_write_row(png_ptr, row);

        p = qr->c->data;
        for (y = 0; y < qr->c->width; y++) {
            bit = 7;
            memset(row, 0xff, (realwidth + 7) / 8);
            q = row;
            q += margin * size / 8;
            bit = 7 - (margin * size % 8);
            for (x = 0; x < qr->c->width; x++) {
                for (xx = 0; xx < size; xx++) {
                    *q ^= (*p & 1) << bit;
                    bit--;
                    if (bit < 0) {
                        q++;
                        bit = 7;
                    }
                }
                p++;
            }

            for (yy = 0; yy < size; yy++)
                png_write_row(png_ptr, row);
        }

        memset(row, 0xff, (realwidth + 7) / 8);
        for (y = 0; y < margin * size; y++)
            png_write_row(png_ptr, row);

        png_write_end(png_ptr, info_ptr);
        png_destroy_write_struct(&png_ptr, &info_ptr);

        efree(row);

        if (temp_file) {
            fseek(fp, 0, SEEK_SET);
#if APACHE && defined(CHARSET_EBCDIC)
            ap_bsetflag(php3_rqst->connection->client, B_EBCDIC2ASCII, 0);
#endif
            while ((b = fread(buf, 1, sizeof(buf), fp)) > 0)
                php_write(buf, b TSRMLS_CC);

            fclose(fp);
            VCWD_UNLINK((const char *)path);
            efree(path);
        } else {
            fflush(fp);
            fclose(fp);
        }

        RETURN_TRUE;
    }

    RETURN_FALSE;
}
/* }}} */

static void qr_dtor(zend_resource *rsrc) {
    php_qrcode *qr = (php_qrcode *)rsrc->ptr;

    if (qr->c)
        QRcode_free(qr->c);
    efree(qr);
}

/*
 * vim:ts=4:expandtab:sw=4
 */
