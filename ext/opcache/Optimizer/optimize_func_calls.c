/* pass 6 
 * - optimize INIT_FCALL_BY_NAME to DO_FCALL
 */

typedef struct _optimizer_call_entry {
	zend_function *func;
	zend_op *opline;
} optimizer_call_entry;

static void optimize_func_calls(zend_op_array *op_array, zend_persistent_script *script TSRMLS_CC) {
	zend_op *opline = op_array->opcodes;
	zend_op *end = opline + op_array->last;
	int call = 0;
#if ZEND_EXTENSION_API_NO > PHP_5_4_X_API_NO
	optimizer_call_entry **call_stacks = ecalloc(op_array->nested_calls, sizeof(optimizer_call_entry *));
#else
	int stack_size = 4;
	optimizer_call_entry **call_stacks = ecalloc(stack_size, sizeof(optimizer_call_entry *));
#endif

	while (opline < end) {
		switch (opline->opcode) {
			case ZEND_INIT_FCALL_BY_NAME:
			case ZEND_INIT_NS_FCALL_BY_NAME:
				if (ZEND_OP2_TYPE(opline) == IS_CONST) {
					zend_function *func;
					zval *function_name = &op_array->literals[opline->op2.constant + 1].constant;

					if ((zend_hash_quick_find(&script->function_table, Z_STRVAL_P(function_name), Z_STRLEN_P(function_name) + 1, Z_HASH_P(function_name), (void **)&func) == SUCCESS)) {
						optimizer_call_entry *entry = emalloc(sizeof(optimizer_call_entry));
#if ZEND_EXTENSION_API_NO < PHP_5_5_X_API_NO
						if (call == stack_size) {
							stack_size += 4;
							call_stacks = erealloc(call_stacks, sizeof(optimizer_call_entry *) * stack_size);
							memset(call_stacks - 4, 0, 4 * sizeof(optimizer_call_entry *));
						}
#endif
						entry->func = func;
						entry->opline = opline;
						call_stacks[call] = entry;
					}
				}
			case ZEND_NEW:
			case ZEND_INIT_METHOD_CALL:
			case ZEND_INIT_STATIC_METHOD_CALL:
				call++;
				break;
			case ZEND_FETCH_FUNC_ARG:
			case ZEND_FETCH_OBJ_FUNC_ARG:
			case ZEND_FETCH_DIM_FUNC_ARG:
				if (call_stacks[call - 1]) {
					if (ARG_SHOULD_BE_SENT_BY_REF(call_stacks[call - 1]->func, (opline->extended_value & ZEND_FETCH_ARG_MASK))) {
						opline->opcode -= 9;
						opline++;
						opline->opcode = ZEND_SEND_REF;
						opline->extended_value = ZEND_DO_FCALL;
					} else {
						opline->opcode -= 12;
						opline++;
						opline->opcode = ZEND_SEND_VAR;
						opline->extended_value = ZEND_DO_FCALL;
					}
				}
				break;
			case ZEND_SEND_VAL:
				if (call && call_stacks[call - 1]) {
					if (ARG_MUST_BE_SENT_BY_REF(call_stacks[call - 1]->func, opline->op2.num)) {
						efree(call_stacks);
						return;
					}
					opline->extended_value = ZEND_DO_FCALL;
				}
				break;
			case ZEND_SEND_VAR:
				if (call && call_stacks[call - 1]) {
					if (ARG_SHOULD_BE_SENT_BY_REF(call_stacks[call - 1]->func, opline->op2.num)) {
						opline->opcode = ZEND_SEND_REF;
					}
					opline->extended_value = ZEND_DO_FCALL;
				}
				break;
			case ZEND_SEND_VAR_NO_REF:
				if (call && call_stacks[call - 1]) {
					if (ARG_SHOULD_BE_SENT_BY_REF(call_stacks[call - 1]->func, opline->op2.num)) {
						opline->extended_value |= ZEND_ARG_COMPILE_TIME_BOUND | ZEND_ARG_SEND_BY_REF;
					} else {
						opline->extended_value |= ZEND_ARG_COMPILE_TIME_BOUND;
					}
				}
				break;
			case ZEND_DO_FCALL_BY_NAME:
				call--;
				if (call_stacks[call]) {
					zend_op *fcall = call_stacks[call]->opline;

					opline->opcode = ZEND_DO_FCALL;

					ZEND_OP1_TYPE(opline) = IS_CONST;
					opline->op1.constant = fcall->op2.constant + 1;
					op_array->literals[fcall->op2.constant + 1].cache_slot = op_array->literals[fcall->op2.constant].cache_slot;
					literal_dtor(&ZEND_OP2_LITERAL(fcall));
					if (fcall->opcode == ZEND_INIT_NS_FCALL_BY_NAME) {
						literal_dtor(&op_array->literals[fcall->op2.constant + 2].constant);
					}
					MAKE_NOP(fcall);
					efree(call_stacks[call]);
					call_stacks[call] = NULL;
				}
				break;
			default:
				break;
		}
		opline++;
	}

	efree(call_stacks);

}
