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
#include <qrencode.h>
#include <png.h>

static int le_qr;

typedef struct
{
    QRcode *c;
} php_qrcode;

static void qr_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);

zend_function_entry qrencode_functions[] = {
    PHP_FE(qr_encode, NULL)
    PHP_FE(qr_save, NULL)
    {NULL, NULL, NULL}
};

zend_module_entry qrencode_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "qrencode",
    qrencode_functions,
    PHP_MINIT(qrencode),
    PHP_MSHUTDOWN(qrencode),
    NULL,
    NULL,
    PHP_MINFO(qrencode),
#if ZEND_MODULE_API_NO >= 20010901
    "0.3",
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_QRENCODE
ZEND_GET_MODULE(qrencode)
#endif

PHP_MINIT_FUNCTION(qrencode)
{
    le_qr = zend_register_list_destructors_ex(qr_dtor, NULL, "qr", module_number);

    REGISTER_LONG_CONSTANT ("QR_MODE_NUL", QR_MODE_NUL, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("QR_MODE_NUM", QR_MODE_NUM, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("QR_MODE_AN", QR_MODE_AN, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("QR_MODE_8", QR_MODE_8, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("QR_MODE_KANJI", QR_MODE_KANJI, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("QR_ECLEVEL_L", QR_ECLEVEL_L, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("QR_ECLEVEL_M", QR_ECLEVEL_M, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("QR_ECLEVEL_Q", QR_ECLEVEL_Q, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("QR_ECLEVEL_H", QR_ECLEVEL_H, CONST_CS | CONST_PERSISTENT);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(qrencode)
{
    return SUCCESS;
}

PHP_MINFO_FUNCTION(qrencode)
{
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
PHP_FUNCTION(qr_encode)
{
    php_qrcode *qr = NULL;
    long version = 1, level = QR_ECLEVEL_L, mode = QR_MODE_8, casesensitive = 1;
    const char *text;
    int text_len;

    if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "s|llll", &text, &text_len, &version, &level, &mode, &casesensitive) == FAILURE)
        RETURN_FALSE;

    qr = (php_qrcode *) emalloc (sizeof (php_qrcode));
    if (mode == QR_MODE_8)
        qr->c = QRcode_encodeString8bit(text, version, level);
    else
        qr->c = QRcode_encodeString(text, version, level, mode, casesensitive);

    if (qr->c == NULL)
    {
        efree (qr);
        RETURN_FALSE;
    }

    ZEND_REGISTER_RESOURCE (return_value, qr, le_qr);
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
PHP_FUNCTION(qr_save)
{
    zval *link = NULL;
    long size = 3, margin = 4;
    const char *fn = NULL;
    int fn_len, argc;
    FILE *fp = NULL;
    png_structp png_ptr;
    png_infop info_ptr;
    unsigned char *row, *p, *q;
    int x, y, xx, yy, bit;
    int realwidth;
    char *path;
    int b;
    char buf[4096];

    argc = ZEND_NUM_ARGS();

    if (zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "r|sll", &link, &fn, &fn_len, &size, &margin) == FAILURE )
        RETURN_FALSE;

    if (link)
    {
        php_qrcode *qr = NULL;

        ZEND_FETCH_RESOURCE2 (qr, php_qrcode *, &link, -1, "qr handle", le_qr, NULL);

        if ((argc == 2) || (argc > 2 && fn != NULL))
        {
            fp = VCWD_FOPEN (fn, "wb");
            if (!fp)
            {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to open '%s' for writing.", fn);
                RETURN_FALSE;
            }
        }
        else
        {
            fp = php_open_temporary_file (NULL, NULL, &path TSRMLS_CC);
            if (!fp)
            {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to open temporary file for writing.");
                RETURN_FALSE;
            }
        }

        realwidth = (qr->c->width + margin * 2) * size;
        row = (unsigned char *) emalloc ((realwidth + 7) / 8);

        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if(png_ptr == NULL)
        {
            fclose(fp);
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to initialize PNG writer.");
            RETURN_FALSE;
        }

        info_ptr = png_create_info_struct(png_ptr);
        if(info_ptr == NULL)
        {
            fclose(fp);
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to initialize PNG write.");
            RETURN_FALSE;
        }


        if(setjmp(png_jmpbuf(png_ptr))) {
            png_destroy_write_struct(&png_ptr, &info_ptr);
            fclose(fp);
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to write PNG image.");
            RETURN_FALSE;
        }

        png_init_io(png_ptr, fp);

        png_set_IHDR(png_ptr, info_ptr, realwidth, realwidth, 1, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png_ptr, info_ptr);

        memset(row, 0xff, (realwidth + 7) / 8);
        for(y = 0; y < margin * size; y++)
            png_write_row(png_ptr, row);

        p = qr->c->data;
        for(y = 0; y < qr->c->width; y++)
        {
            bit = 7;
            memset(row, 0xff, (realwidth + 7) / 8);
            q = row;
            q += margin * size / 8;
            bit = 7 - (margin * size % 8);
            for(x = 0; x < qr->c->width; x++)
            {
                for(xx = 0; xx < size; xx++)
                {
                    *q ^= (*p & 1) << bit;
                    bit--;
                    if(bit < 0)
                    {
                        q++;
                        bit = 7;
                    }
                }
                p++;
            }

            for(yy=0; yy<size; yy++)
                png_write_row(png_ptr, row);
        }

        memset(row, 0xff, (realwidth + 7) / 8);
        for(y=0; y<margin * size; y++)
            png_write_row(png_ptr, row);

        png_write_end(png_ptr, info_ptr);
        png_destroy_write_struct(&png_ptr, &info_ptr);

        efree (row);

        if ((argc == 2) || (argc > 2 && fn != NULL))
        {
            fflush (fp);
            fclose (fp);
        }
        else
        {
            fseek (fp, 0, SEEK_SET);
#if APACHE && defined(CHARSET_EBCDIC)
            ap_bsetflag(php3_rqst->connection->client, B_EBCDIC2ASCII, 0);
#endif
            while ((b = fread (buf, 1, sizeof(buf), fp)) > 0)
                php_write (buf, b TSRMLS_CC);

            fclose (fp);
            VCWD_UNLINK ((const char *)path);
            efree (path);
        }

        RETURN_TRUE;
    }
    else
        RETURN_FALSE;
}
/* }}} */

static void qr_dtor (zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    php_qrcode *qr = (php_qrcode *) rsrc->ptr;

    if (qr->c)
        QRcode_free (qr->c);
    efree (qr);
}

/*
 * vim:ts=4:expandtab:sw=4
 */
