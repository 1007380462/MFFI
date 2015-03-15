#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dlfcn.h>
#include <ffi.h>

#include "php.h"
#include "zend_exceptions.h"
#include "php_mffi.h"
#include "mffi_internal.h"

zend_class_entry *mffi_ce_handle;

static zend_object_handlers mffi_handle_object_handlers;

static inline php_mffi_handle_object *php_mffi_handle_fetch_object(zend_object *obj);

/* {{{ */
PHP_METHOD(MFFI_Handle, __construct)
{
	zend_string *lib_name;
	void *handle;
	php_mffi_handle_object *object;
	zval *self;

	PHP_MFFI_ERROR_HANDLING();
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &lib_name) == FAILURE) {
		PHP_MFFI_RESTORE_ERRORS();
		return;
	}
	PHP_MFFI_RESTORE_ERRORS();

	//	object = php_mffi_handle_fetch_object(Z_OBJ_P(getThis()));
	PHP_MFFI_HANDLE_FROM_OBJECT(object, getThis());

	handle = dlopen(lib_name->val, RTLD_LAZY);
	if (!handle) {
		zend_throw_exception(mffi_ce_exception, "Could not open library", 1);
		return;
	}

	object->handle = handle;
}
/* }}} */

const zend_function_entry mffi_handle_methods[] = {
	PHP_ME(MFFI_Handle, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
		PHP_FE_END
};

static inline php_mffi_handle_object *php_mffi_handle_fetch_object(zend_object *obj) {
	return (php_mffi_handle_object *)((char*)(obj) - XtOffsetOf(php_mffi_handle_object, std));
}

static zend_object *mffi_handle_object_new(zend_class_entry *ce)
{
	php_mffi_handle_object *object = ecalloc(1, sizeof(php_mffi_handle_object) + zend_object_properties_size(ce));
	zend_object_std_init(&object->std, ce);
	object_properties_init(&object->std, ce);
	object->std.handlers = &mffi_handle_object_handlers;
	return &object->std;
}

static void mffi_handle_object_free_storage(zend_object *object)
{
	php_mffi_handle_object *intern = php_mffi_handle_fetch_object(object);

	if (!intern) {
		return;
	}

	zend_object_std_dtor(&intern->std);
}

PHP_MINIT_FUNCTION(mffi_handle)
{
	zend_class_entry handle_ce;

	memcpy(&mffi_handle_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	mffi_handle_object_handlers.offset = XtOffsetOf(php_mffi_handle_object, std);
	mffi_handle_object_handlers.free_obj = mffi_handle_object_free_storage;
	mffi_handle_object_handlers.clone_obj = NULL;

	INIT_NS_CLASS_ENTRY(handle_ce, "MFFI", "Handle", mffi_handle_methods);
	handle_ce.create_object = mffi_handle_object_new;
	mffi_ce_handle = zend_register_internal_class(&handle_ce);

	return SUCCESS;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
