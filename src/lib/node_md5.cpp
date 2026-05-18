#include "node_md5.h"

#include <iostream>
#include <node_api.h>

#include "md5.h"

inline napi_value create_md5_step(napi_env env, const md5_step* step) {
    napi_value result;
    if (napi_create_object(env, &result) != napi_ok) {
        return undefined(env);
    }

    napi_value blk_index, block, round, a, b, c, d, f, inner_sum, temp;

    napi_create_buffer_copy(env, step->block.size(), step->block.data(), nullptr, &block);
    napi_create_uint32(env, step->blk_index, &blk_index);
    napi_create_uint32(env, step->round, &round);
    napi_create_uint32(env, step->a, &a);
    napi_create_uint32(env, step->b, &b);
    napi_create_uint32(env, step->c, &c);
    napi_create_uint32(env, step->d, &d);
    napi_create_uint32(env, step->f, &f);
    napi_create_uint32(env, step->inner_sum, &inner_sum);
    napi_create_uint32(env, step->temp, &temp);

    napi_property_descriptor props[] = {
        { "blk_index", nullptr, nullptr, nullptr, nullptr, blk_index, napi_default, nullptr },
        { "block", nullptr, nullptr, nullptr, nullptr, block, napi_default, nullptr },
        { "round", nullptr, nullptr, nullptr, nullptr, round, napi_default, nullptr },
        { "a", nullptr, nullptr, nullptr, nullptr, a, napi_default, nullptr },
        { "b", nullptr, nullptr, nullptr, nullptr, b, napi_default, nullptr },
        { "c", nullptr, nullptr, nullptr, nullptr, c, napi_default, nullptr },
        { "d", nullptr, nullptr, nullptr, nullptr, d, napi_default, nullptr },
        { "f", nullptr, nullptr, nullptr, nullptr, f, napi_default, nullptr },
        { "inner_sum", nullptr, nullptr, nullptr, nullptr, inner_sum, napi_default, nullptr },
        { "temp", nullptr, nullptr, nullptr, nullptr, temp, napi_default, nullptr }
    };

    napi_define_properties(env, result, std::size(props), props);
    return result;
}

static void call_js(napi_env env, napi_value cb, void *, void *data) {
    auto *step = static_cast<md5_step*>(data);
    if (env != nullptr && cb != nullptr) {
        napi_value result = create_md5_step(env, step);
        napi_call_function(env, undefined(env), cb, 1, &result, nullptr);
    }
    delete step;
}

static void execute_work(napi_env env, void *data) {
    auto *work = static_cast<hash_work*>(data);
    work->output = md5_trace(work->input, [work](const md5_step& step) {
        auto *value = new md5_step(step);
        auto status = napi_call_threadsafe_function(work->ts_fn, value, napi_tsfn_blocking);

        if (status != napi_ok) {
            delete value;
        }
    });
    napi_release_threadsafe_function(work->ts_fn, napi_tsfn_release);
}

static void complete_work(napi_env, napi_status, void *) {
    // Do nothing
}

static void thread_safe_fn_cb(napi_env env, void* finalize_data, void*) {
    auto *work = static_cast<hash_work*>(finalize_data);

    napi_value result;
    if (napi_create_buffer_copy(env, work->output.size(), work->output.data(), nullptr, &result) == napi_ok) {
        napi_resolve_deferred(env, work->deferred, result);
    } else {
        napi_value undefined;
        napi_get_undefined(env, &undefined);
        napi_reject_deferred(env, work->deferred, undefined);
    }
    if (work->work != nullptr) {
        napi_delete_async_work(env, work->work);
    }

    // 3. Delete the state structure last
    delete work;
}

static napi_value compute_md5(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2];

    if (napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr) != napi_ok || argc != 2) {
        napi_throw_error(env, nullptr, "Invalid argument architecture mapping");
        return undefined(env);
    }

    napi_value data = argv[0];
    napi_value cb = argv[1];

    if (!is_function(env, cb)) {
        napi_throw_type_error(env, nullptr, "Callback must be an executable function");
        return undefined(env);
    }

    std::vector<u_int8_t> bytes = get_bytes(env, data);
    if (bytes.empty()) {
        napi_throw_error(env, nullptr, "Input payload must be a non-empty string or buffer");
        return undefined(env);
    }

    napi_value async_res_name;
    napi_create_string_utf8(env, "MD5 Compute Trace Resource", NAPI_AUTO_LENGTH, &async_res_name);

    auto *work = new hash_work();
    work->input = std::move(bytes);

    napi_value promise;
    if (napi_create_promise(env, &work->deferred, &promise) != napi_ok) {
        delete work;
        return undefined(env);
    }

    if (napi_create_threadsafe_function(env, cb, nullptr, async_res_name, 0, 1,
                                        work, thread_safe_fn_cb, nullptr, call_js, &work->ts_fn) != napi_ok) {
        napi_reject_deferred(env, work->deferred, undefined(env));
        delete work;
        return promise;
    }
    if (napi_create_async_work(env, nullptr, async_res_name, execute_work,
                               complete_work, work, &work->work) != napi_ok) {
        napi_release_threadsafe_function(work->ts_fn, napi_tsfn_release);
        napi_reject_deferred(env, work->deferred, undefined(env));
        delete work;
        return promise;
    }
    napi_queue_async_work(env, work->work);

    return promise;
}

static napi_value init(napi_env env, napi_value exports) {
    napi_status status;
    napi_property_descriptor props[] = {
        {"computeMD5", nullptr, compute_md5, nullptr, nullptr, nullptr, napi_default, nullptr },
    };
    status = napi_define_properties(env, exports, 1, props);
    if (status != napi_ok) {
        return undefined(env);
    }
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init);
