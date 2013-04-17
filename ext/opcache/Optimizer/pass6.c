if (ZEND_OPTIMIZER_PASS_6 & OPTIMIZATION_LEVEL) {
	optimize_func_calls(op_array, script TSRMLS_CC);
}
