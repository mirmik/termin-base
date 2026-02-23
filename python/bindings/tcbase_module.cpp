#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <tcbase/input_enums.hpp>
#include <tcbase/tc_log.hpp>

namespace nb = nanobind;

static nb::callable g_py_callback;

static void py_log_callback_wrapper(tc_log_level level, const char* message) {
    if (g_py_callback.is_valid()) {
        nb::gil_scoped_acquire acquire;
        try {
            g_py_callback(static_cast<int>(level), std::string(message));
        } catch (const std::exception&) {
            // Don't recurse if callback fails
        }
    }
}

// Helper to format Python exception
static std::string format_py_exception(nb::object exc, const std::string& context) {
    std::string exc_str;
    try {
        exc_str = nb::cast<std::string>(nb::str(exc));
    } catch (...) {
        exc_str = "<unknown exception>";
    }
    if (context.empty()) {
        return exc_str;
    }
    return context + ": " + exc_str;
}

static void bind_log(nb::module_& m) {
    nb::enum_<tc_log_level>(m, "Level")
        .value("DEBUG", TC_LOG_DEBUG)
        .value("INFO", TC_LOG_INFO)
        .value("WARN", TC_LOG_WARN)
        .value("ERROR", TC_LOG_ERROR)
        .export_values();

    m.def("set_level", &tc_log_set_level, nb::arg("level"),
        "Set minimum log level");

    m.def("set_callback", [](nb::object callback) {
        if (callback.is_none()) {
            g_py_callback = nb::callable();
            tc_log_set_callback(nullptr);
        } else {
            g_py_callback = nb::cast<nb::callable>(callback);
            tc_log_set_callback(py_log_callback_wrapper);
        }
    }, nb::arg("callback"),
        "Set callback for log interception. Callback signature: (level: int, message: str)");

    m.def("debug", [](const std::string& msg) {
        tc_log_debug("%s", msg.c_str());
    }, nb::arg("message"), "Log debug message");

    m.def("debug", [](nb::object exc, const std::string& context) {
        std::string msg = format_py_exception(exc, context);
        tc_log_debug("%s", msg.c_str());
    }, nb::arg("exception"), nb::arg("context") = "", "Log debug with exception");

    m.def("info", [](const std::string& msg) {
        tc_log_info("%s", msg.c_str());
    }, nb::arg("message"), "Log info message");

    m.def("info", [](nb::object exc, const std::string& context) {
        std::string msg = format_py_exception(exc, context);
        tc_log_info("%s", msg.c_str());
    }, nb::arg("exception"), nb::arg("context") = "", "Log info with exception");

    m.def("warn", [](const std::string& msg) {
        tc_log_warn("%s", msg.c_str());
    }, nb::arg("message"), "Log warning message");

    m.def("warn", [](nb::object exc, const std::string& context) {
        std::string msg = format_py_exception(exc, context);
        tc_log_warn("%s", msg.c_str());
    }, nb::arg("exception"), nb::arg("context") = "", "Log warning with exception");

    m.def("error", [](const std::string& msg) {
        tc_log_error("%s", msg.c_str());
    }, nb::arg("message"), "Log error message");

    m.def("error", [](nb::object exc, const std::string& context) {
        std::string msg = format_py_exception(exc, context);
        tc_log_error("%s", msg.c_str());
    }, nb::arg("exception"), nb::arg("context") = "", "Log error with exception");

    // Alias for consistency with Python's logging module
    m.def("warning", [](const std::string& msg) {
        tc_log_warn("%s", msg.c_str());
    }, nb::arg("message"), "Log warning message (alias for warn)");

    // Log error with exception traceback
    m.def("exception", [](const std::string& msg) {
        nb::module_ traceback = nb::module_::import_("traceback");
        nb::object format_exc = traceback.attr("format_exc");
        std::string tb = nb::cast<std::string>(format_exc());
        tc_log_error("%s\n%s", msg.c_str(), tb.c_str());
    }, nb::arg("message"), "Log error message with current exception traceback");

    // Log with optional exc_info
    m.def("error", [](const std::string& msg, bool exc_info) {
        if (exc_info) {
            nb::module_ traceback = nb::module_::import_("traceback");
            nb::object format_exc = traceback.attr("format_exc");
            std::string tb = nb::cast<std::string>(format_exc());
            tc_log_error("%s\n%s", msg.c_str(), tb.c_str());
        } else {
            tc_log_error("%s", msg.c_str());
        }
    }, nb::arg("message"), nb::arg("exc_info"), "Log error message with optional traceback");

    m.def("warning", [](const std::string& msg, bool exc_info) {
        if (exc_info) {
            nb::module_ traceback = nb::module_::import_("traceback");
            nb::object format_exc = traceback.attr("format_exc");
            std::string tb = nb::cast<std::string>(format_exc());
            tc_log_warn("%s\n%s", msg.c_str(), tb.c_str());
        } else {
            tc_log_warn("%s", msg.c_str());
        }
    }, nb::arg("message"), nb::arg("exc_info"), "Log warning message with optional traceback");
}

NB_MODULE(_tcbase_native, m) {
    m.doc() = "Base types shared between termin libraries";

    nb::enum_<tcbase::MouseButton>(m, "MouseButton", "Mouse button constants")
        .value("LEFT", tcbase::MouseButton::LEFT)
        .value("RIGHT", tcbase::MouseButton::RIGHT)
        .value("MIDDLE", tcbase::MouseButton::MIDDLE)
        .export_values();

    nb::enum_<tcbase::Action>(m, "Action", "Action constants")
        .value("RELEASE", tcbase::Action::RELEASE)
        .value("PRESS", tcbase::Action::PRESS)
        .value("REPEAT", tcbase::Action::REPEAT)
        .export_values();

    nb::enum_<tcbase::Mods>(m, "Mods", "Modifier key flags")
        .value("SHIFT", tcbase::Mods::SHIFT)
        .value("CTRL", tcbase::Mods::CTRL)
        .value("ALT", tcbase::Mods::ALT)
        .value("SUPER", tcbase::Mods::SUPER)
        .export_values();

    // Logging submodule
    auto log_module = m.def_submodule("log", "Logging module");
    bind_log(log_module);
}
