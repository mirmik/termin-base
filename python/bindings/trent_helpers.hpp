// trent_helpers.hpp - trent <-> Python conversion helpers
#pragma once

#include <nanobind/nanobind.h>
#include <tcbase/trent/trent.h>

namespace nb = nanobind;

namespace termin {

inline nos::trent py_to_trent(nb::object obj) {
    if (obj.is_none()) {
        return nos::trent::nil();
    }
    if (nb::isinstance<nb::bool_>(obj)) {
        return nos::trent(nb::cast<bool>(obj));
    }
    if (nb::isinstance<nb::int_>(obj)) {
        return nos::trent(nb::cast<int64_t>(obj));
    }
    if (nb::isinstance<nb::float_>(obj)) {
        return nos::trent(nb::cast<double>(obj));
    }
    if (nb::isinstance<nb::str>(obj)) {
        return nos::trent(nb::cast<std::string>(obj));
    }
    if (nb::isinstance<nb::list>(obj)) {
        nos::trent result;
        result.init(nos::trent_type::list);
        for (auto item : obj) {
            result.as_list().push_back(py_to_trent(nb::borrow<nb::object>(item)));
        }
        return result;
    }
    if (nb::isinstance<nb::dict>(obj)) {
        nos::trent result;
        result.init(nos::trent_type::dict);
        for (auto item : nb::cast<nb::dict>(obj)) {
            std::string key = nb::cast<std::string>(item.first);
            result[key] = py_to_trent(nb::borrow<nb::object>(item.second));
        }
        return result;
    }
    return nos::trent::nil();
}

inline nb::object trent_to_py(const nos::trent& t) {
    switch (t.get_type()) {
        case nos::trent_type::nil:
            return nb::none();
        case nos::trent_type::boolean:
            return nb::bool_(t.as_bool());
        case nos::trent_type::numer: {
            double val = t.as_numer();
            if (val == static_cast<int64_t>(val)) {
                return nb::int_(static_cast<int64_t>(val));
            }
            return nb::float_(val);
        }
        case nos::trent_type::string:
            return nb::str(t.as_string().c_str());
        case nos::trent_type::list: {
            nb::list result;
            for (const auto& item : t.as_list()) {
                result.append(trent_to_py(item));
            }
            return result;
        }
        case nos::trent_type::dict: {
            nb::dict result;
            for (const auto& [key, val] : t.as_dict()) {
                result[nb::str(key.c_str())] = trent_to_py(val);
            }
            return result;
        }
    }
    return nb::none();
}

} // namespace termin
