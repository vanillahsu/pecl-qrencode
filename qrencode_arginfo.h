/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 9c06dd059f5d508465f74f01e9d06ceca9390b35 */

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_qr_encode, 0, 5, resource, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, version, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, leve, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, casesenstitive, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_qr_save, 0, 4, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, qrencode, resource, 0)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, margin, IS_LONG, 1)
ZEND_END_ARG_INFO()


ZEND_FUNCTION(qr_encode);
ZEND_FUNCTION(qr_save);


static const zend_function_entry ext_functions[] = {
	ZEND_FE(qr_encode, arginfo_qr_encode)
	ZEND_FE(qr_save, arginfo_qr_save)
	ZEND_FE_END
};
