import * as std from "std";
import * as os from "os";

// TODO: Deno.errors.NotFound

function Event(type) {
    this.type = type;
}

globalThis.Event = Event;

function EventTarget() {
}

EventTarget.prototype.addEventListener = function (type, listener, options) {
    throw "TODO";
};

EventTarget.prototype.dispatchEvent = function (event) {
    // TODO
};

EventTarget.prototype.removeEventListener = function (type, listener) {
    throw "TODO";
};

globalThis.EventTarget = EventTarget;

const localeToDateFimeFormat = {
    ["en-US"]: (options) => {
        if (options.month === "long") {
            const monthNames = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"];
            return (date) => `${monthNames[date.getUTCMonth()]} ${date.getUTCDate()}, ${date.getUTCFullYear()}`;
        }

        throw "TODO";
    },
};

function DateTimeFormat(locale, options) {
    const factory = localeToDateFimeFormat[locale];
    if (!factory) {
        throw "TODO";
    }

    this.format = factory(options);
}

globalThis.Intl = {
    DateTimeFormat,
};

function TextDecoder() {
}

TextDecoder.prototype.decode = function (bufferOrTypedArray) {
	// If a TypedArray was supplied, use the underlying ArrayBuffer
	return __TextDecoder_decode(bufferOrTypedArray?.buffer ?? bufferOrTypedArray);
};

globalThis.TextDecoder = TextDecoder;

function TextEncoder() {
}

TextEncoder.prototype.encode = function (str) {
	return new Uint8Array(__TextEncoder_encode(str));
};

globalThis.TextEncoder = TextEncoder;

globalThis.Deno = {
    args: scriptArgs,
    exit: (status) => std.exit(status ?? 0),
    mkdir: (path) => {
        // TODO: Does not handle non-recursive
        const parts = path.split("/");
        for (let i = 0; i < parts.length; i++) {
            os.mkdir(parts.slice(0, i + 1).join("/"));
        }
        return Promise.resolve();
    },
    readDir: (path) => {
        const [names, errorCode] = os.readdir(path);
        if (errorCode) {
            return Promise.resolve(errorCode);
        }

        return {
            async *[Symbol.asyncIterator]() {
                for (const name of names) {
                    if (name !== "." && name !== "..") {
                        const entryPath = path + "/" + name;
                        const [details, errorCode] = os.stat(entryPath);
                        if (errorCode) {
                            throw new Error(`Failed to stat: ${entryPath}`);
                        }

                        const isFile = ((details.mode & os.S_IFDIR) === 0);
                        yield {
                            name,
                            isFile,
                            isDirectory: !isFile,
                        };
                    }
                }
            },
        };
    },
    readFile: (path) => {
        const file = std.open(path, "rb");
        if (!file) {
            return Promise.reject(new Error(`Failed to open file for read: ${path}`));
        }

        file.seek(0, std.SEEK_END);
        const size = file.tell();
        file.seek(0, std.SEEK_SET);

        const buffer = new ArrayBuffer(size);
        file.read(buffer, 0, size);
        file.close();
        return new Uint8Array(buffer);
    },
    readTextFile: (path) => {
        const str = std.loadFile(path);
        if (str) {
            return Promise.resolve(str);
        } else {
            return Promise.reject(new Error(`Failed to read text file: ${path}`));
        }
    },
    remove: (path) => {
        os.remove(path);
    },
    writeFile: (path, data) => {
        const file = std.open(path, "wb");
        if (!file) {
            return Promise.reject(new Error(`Failed to open file for write: ${path}`));
        }

        file.write(data.buffer, 0, data.byteLength);
        return Promise.resolve();
    },
    writeTextFile: (path, text) => {
        const file = std.open(path, "w");
        if (!file) {
            return Promise.reject(new Error(`Failed to open text file for write: ${path}`));
        }

        file.puts(text);
        const result = file.close();
        if (result !== 0) {
            return Promise.reject(new Error(`IO error writing to file: ${path}`));
        }

        return Promise.resolve();
    },
};

