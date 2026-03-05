// tc_value_trent.cpp - Conversion between tc_value and nos::trent
#include <tcbase/tc_value_trent.hpp>

namespace tc {

tc_value trent_to_tc_value(const nos::trent& t) {
    if (t.is_nil()) {
        return tc_value_nil();
    }

    if (t.is_bool()) {
        return tc_value_bool(t.as_bool());
    }

    if (t.is_numer()) {
        double val = static_cast<double>(t.as_numer());
        // Check if it's an integer value
        if (val == static_cast<double>(static_cast<int64_t>(val))) {
            return tc_value_int(static_cast<int64_t>(val));
        }
        return tc_value_double(val);
    }

    if (t.is_string()) {
        return tc_value_string(t.as_string().c_str());
    }

    if (t.is_list()) {
        tc_value list = tc_value_list_new();
        for (const auto& item : t.as_list()) {
            tc_value v = trent_to_tc_value(item);
            tc_value_list_push(&list, v);
        }
        return list;
    }

    if (t.is_dict()) {
        tc_value dict = tc_value_dict_new();
        for (const auto& [key, val] : t.as_dict()) {
            tc_value v = trent_to_tc_value(val);
            tc_value_dict_set(&dict, key.c_str(), v);
        }
        return dict;
    }

    return tc_value_nil();
}

nos::trent tc_value_to_trent(const tc_value& v) {
    switch (v.type) {
        case TC_VALUE_NIL:
            return nos::trent();

        case TC_VALUE_BOOL:
            return nos::trent(v.data.b);

        case TC_VALUE_INT:
            return nos::trent(v.data.i);

        case TC_VALUE_FLOAT:
            return nos::trent(static_cast<double>(v.data.f));

        case TC_VALUE_DOUBLE:
            return nos::trent(v.data.d);

        case TC_VALUE_STRING:
            return nos::trent(v.data.s ? v.data.s : "");

        case TC_VALUE_VEC3: {
            nos::trent result;
            result.init(nos::trent_type::list);
            result.push_back(nos::trent(static_cast<double>(v.data.v3.x)));
            result.push_back(nos::trent(static_cast<double>(v.data.v3.y)));
            result.push_back(nos::trent(static_cast<double>(v.data.v3.z)));
            return result;
        }

        case TC_VALUE_QUAT: {
            nos::trent result;
            result.init(nos::trent_type::list);
            result.push_back(nos::trent(static_cast<double>(v.data.q.w)));
            result.push_back(nos::trent(static_cast<double>(v.data.q.x)));
            result.push_back(nos::trent(static_cast<double>(v.data.q.y)));
            result.push_back(nos::trent(static_cast<double>(v.data.q.z)));
            return result;
        }

        case TC_VALUE_LIST: {
            nos::trent result;
            result.init(nos::trent_type::list);
            for (size_t i = 0; i < v.data.list.count; i++) {
                result.push_back(tc_value_to_trent(v.data.list.items[i]));
            }
            return result;
        }

        case TC_VALUE_DICT: {
            nos::trent result;
            result.init(nos::trent_type::dict);
            for (size_t i = 0; i < v.data.dict.count; i++) {
                const char* key = v.data.dict.entries[i].key;
                if (key && v.data.dict.entries[i].value) {
                    result[key] = tc_value_to_trent(*v.data.dict.entries[i].value);
                }
            }
            return result;
        }

        default:
            return nos::trent();
    }
}

} // namespace tc
