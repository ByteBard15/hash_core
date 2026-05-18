#ifndef NODE_MD5_H
#define NODE_MD5_H

#include <node_api.h>
#include <string>
#include <vector>
#include <sys/types.h>

struct hash_work {
    napi_async_work work;
    napi_threadsafe_function ts_fn;
    napi_deferred deferred;
    std::vector<u_int8_t> input;
    std::vector<u_int8_t> output;
};

inline napi_value undefined(napi_env env) {
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

inline napi_value null(napi_env env) {
    napi_value result;
    napi_get_null(env, &result);
    return result;
}

inline std::string get_string(napi_env env, napi_value value) {
    napi_valuetype type;
    napi_status status = napi_typeof(env, value, &type);
    if (status != napi_ok || type != napi_string) {
        napi_throw_type_error(env, nullptr, "Expected a string argument");
        return {};
    }

    size_t str_size;
    status = napi_get_value_string_utf8(env, value, nullptr, 0, &str_size);

    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get string length");
        return {};
    }

    std::string str(str_size, '\0');
    status = napi_get_value_string_utf8(env, value, str.data(), str_size + 1, nullptr);

    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to read string argument");
        return {};
    }
    return str;
}

inline size_t get_int32(napi_env env, napi_value value) {
    napi_valuetype type;
    if (napi_typeof(env, value, &type) != napi_ok || type != napi_number) {
        napi_throw_type_error(env, nullptr, "Expected a number");
        return -1;
    }

    int32_t out;
    napi_status status = napi_get_value_int32(env, value, &out);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to read integer");
        return -1;
    }

    return out;
}

inline bool is_buffer(napi_env env, napi_value value) {
    bool is_buffer;
    if (napi_is_buffer(env, value, &is_buffer) != napi_ok || !is_buffer) {
        return false;
    }
    return true;
}

inline bool is_string(napi_env env, napi_value value) {
    napi_valuetype type;
    if (napi_typeof(env, value, &type) != napi_ok || type != napi_string) {
        return false;
    }
    return true;
}

inline bool get_buffer(napi_env env, napi_value value, void*& data, size_t& length) {
    bool is_buffer;
    if (napi_is_buffer(env, value, &is_buffer) != napi_ok || !is_buffer) {
        napi_throw_type_error(env, nullptr, "Expected a buffer");
        return false;
    }

    napi_status status = napi_get_buffer_info(env, value, &data, &length);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get buffer info");
        return false;
    }
    return true;
}

static std::vector<u_int8_t> get_bytes(napi_env env, napi_value value) {
    napi_status status;
    if (is_string(env, value)) {
        size_t size_len;
        status = napi_get_value_string_utf8(env, value, nullptr, 0, &size_len);
        if (status != napi_ok) {
            return {};
        }
        std::vector<u_int8_t> bytes(size_len + 1);

        napi_get_value_string_utf8(env, value, reinterpret_cast<char *>(bytes.data()), bytes.size(), nullptr);
        bytes.pop_back();

        return bytes;
    }
    if (is_buffer(env, value)) {
        void *buffer;
        size_t length;

        napi_get_buffer_info(env, value, &buffer, &length);
        std::vector<u_int8_t> bytes(static_cast<u_int8_t*>(buffer), static_cast<u_int8_t *>(buffer) + length);
        return bytes;
    }
    return {};
}

inline bool is_function(napi_env env, napi_value value) {
    napi_status status;
    napi_valuetype type;
    status = napi_typeof(env, value, &type);

    if (status != napi_ok) {
        return false;
    }
    return type == napi_function;
}

#endif