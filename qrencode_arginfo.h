/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 4d5e13693b43090a4172c144495d58ba76ae2aba */

ZEND_BEGIN_ARG_INFO_EX(arginfo_qr_encode, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, version, IS_LONG, 1, "1")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, level, IS_LONG, 1, "QR_ECLEVEL_L")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 1, "QR_MODE_8")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, casesenstitive, IS_LONG, 1, "1")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_qr_save, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, qrencode)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, filename, IS_STRING, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, size, IS_LONG, 1, "3")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, margin, IS_LONG, 1, "4")
ZEND_END_ARG_INFO()


ZEND_FUNCTION(qr_encode);
ZEND_FUNCTION(qr_save);


static const zend_function_entry ext_functions[] = {
	ZEND_FE(qr_encode, arginfo_qr_encode)
	ZEND_FE(qr_save, arginfo_qr_save)
	ZEND_FE_END
};
