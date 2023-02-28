#include <string.h>
#include "quickjs/quickjs-libc.h"
#include "quickjs/cutils.h"
#include "lib.h"

static int evaluate_buffer(JSContext* context, const void* buffer, int bufferLength, const char* filePath) {
	int status = 0;
	JSValue value = JS_Eval(context, buffer, bufferLength, filePath, JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);

	if (!JS_IsException(value)) {
		js_module_set_import_meta(context, value, TRUE, TRUE);
		value = JS_EvalFunction(context, value);
	}

	if (JS_IsException(value)) {
		js_std_dump_error(context);
		status = -1;
	}

	JS_FreeValue(context, value);
	return status;
}

static int evaluate_file(JSContext* context, const char* filePath) {
	int status = 0;
	uint8_t *buffer;
	size_t bufferLength;

	buffer = js_load_file(context, &bufferLength, filePath);
	if (!buffer) {
		puts("Failed to read file!\n");
		return -1;
	}

	status = evaluate_buffer(context, buffer, bufferLength, filePath);

	js_free(context, buffer);
	return status;
}

int main(int argumentCount, char** arguments) {
	const int internalArgumentCount = 2;

	int status = 0;
	JSRuntime* runtime;
	JSContext* context;

	if (argumentCount < internalArgumentCount) {
		puts("USAGE: leano <script>\n");
		exit(-1);
	}

	runtime = JS_NewRuntime();
	if (runtime) {
		js_std_init_handlers(runtime);
    	context = JS_NewContext(runtime);
		if (context) {
			js_init_module_std(context, "std");
			js_init_module_os(context, "os");
        	js_std_add_helpers(context, argumentCount - internalArgumentCount, arguments + internalArgumentCount);

			// Add internal library
			const char* shim = STRINGIFIED_SCRIPT;
			status = evaluate_buffer(context, shim, strlen(shim), "<input>");
			if (status) {
				puts("Failed to evaluate shim!\n");
			}

			// Run the script!
			status = evaluate_file(context, arguments[1]);
			js_std_loop(context);

			JS_FreeContext(context);
		}
		else {
			puts("Failed to create context!\n");
			status = -1;
		}

		js_std_free_handlers(runtime);
		JS_FreeRuntime(runtime);
	}
	else {
		puts("Failed to create runtime!\n");
		status = -1;
	}


	return status;
}

