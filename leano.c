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

void free_array_buffer_string(JSRuntime* runtime, void* opaque, void* pointer) {
	JSContext* context = (JSContext*)opaque;
	JS_FreeCString(context, (const char*)pointer);
}

static JSValue text_encoder_encode(JSContext *context, JSValueConst this_value, int argumentCount, JSValueConst* arguments) {
	const char* string;
	size_t length;
	JSValue arrayBuffer;

	if (argumentCount < 1) {
		return JS_EXCEPTION;
	}

	string = JS_ToCStringLen(context, &length, arguments[0]);
	if (!string) {
		return JS_EXCEPTION;
	}

	// Ownership of string moved to array buffer...
	arrayBuffer = JS_NewArrayBuffer(context, (uint8_t*)string, length, free_array_buffer_string, context, FALSE);
	if (!arrayBuffer) {
		JS_FreeCString(context, string);
		return JS_EXCEPTION;
	}

	return arrayBuffer;
}

static JSValue text_decoder_decode(JSContext *context, JSValueConst this_value, int argumentCount, JSValueConst* arguments) {
	uint8_t* buffer;
	size_t length;
	JSValue string;

	if (argumentCount < 1) {
		return JS_EXCEPTION;
	}

	buffer = JS_GetArrayBuffer(context, &length, arguments[0]);
	if (!buffer) {
		return JS_EXCEPTION;
	}

	string = JS_NewStringLen(context, (const char*)buffer, length);
	// TODO: Need to free?
	if (!string) {
		return JS_EXCEPTION;
	}

	return string;
}

static void add_shims(JSContext* context) {
	const char* encodeProperty = "__TextEncoder_encode";
	const char* decodeProperty = "__TextDecoder_decode";

	JSValue globalThis;
	JSValue encode;
	JSValue decode;

	globalThis = JS_GetGlobalObject(context);

	encode = JS_NewCFunction(context, text_encoder_encode, encodeProperty, 1);
	JS_SetPropertyStr(context, globalThis, encodeProperty, encode);

	decode = JS_NewCFunction(context, text_decoder_decode, decodeProperty, 1);
	JS_SetPropertyStr(context, globalThis, decodeProperty, decode);

	JS_FreeValue(context, globalThis);
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

			// Add C shims
			add_shims(context);

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

