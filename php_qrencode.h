/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 2007 Vanilla (San Tai) Hsu                             |
  +----------------------------------------------------------------------+
  | This source file is subject to the BSD license, that is bundled with |
  | this package in the file LICENSE                                     |
  +----------------------------------------------------------------------+
  | Author:  Vanilla (San Tai) Hsu (vanilla@FreeBSD.org>                 |
  +----------------------------------------------------------------------+
*/

/*
 * $Header: /home/ncvs/php_extension/qrencode/php_qrencode.h,v 1.1 2007/03/29 08:58:11 vanilla Exp $
 */

#ifndef PHP_QRENCODE_H
#define PHP_QRENCODE_H

extern zend_module_entry qrencode_module_entry;
#define phpext_qrencode_ptr &qrencode_module_entry

#ifdef PHP_WIN32
#define PHP_QRENCODE_API __declspec(dllexport)
#else
#define PHP_QRENCODE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(qrencode);
PHP_MSHUTDOWN_FUNCTION(qrencode);
PHP_MINFO_FUNCTION(qrencode);
PHP_FUNCTION(qr_encode);
PHP_FUNCTION(qr_save);

#ifdef ZTS
#define QRENCODE_G(v) TSRMG(qrencode_globals_id, zend_qrencode_globals *, v)
#else
#define QRENCODE_G(v) (qrencode_globals.v)
#endif

#endif	/* PHP_QRENCODE_H */

