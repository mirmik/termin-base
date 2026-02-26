#include <tcbase/settings.h>
#include <tcbase/tc_log.hpp>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#endif

namespace
{
    bool is_sep(char c)
    {
        return c == '/'
#ifdef _WIN32
            || c == '\\'
#endif
            ;
    }

    std::string expand_home(const std::string &path)
    {
        if (!path.empty() && path[0] == '~')
        {
            const char *home = std::getenv("HOME");
#ifdef _WIN32
            if (!home)
                home = std::getenv("USERPROFILE");
#endif
            if (home)
                return std::string(home) + path.substr(1);
        }
        return path;
    }

    void mkdir_p(const std::string &dir)
    {
        std::string accum;
        for (size_t i = 0; i < dir.size(); ++i)
        {
            accum += dir[i];
            if (is_sep(dir[i]) || i == dir.size() - 1)
            {
#ifdef _WIN32
                _mkdir(accum.c_str());
#else
                mkdir(accum.c_str(), 0755);
#endif
            }
        }
    }

    std::string dir_of(const std::string &path)
    {
        auto pos = path.rfind('/');
#ifdef _WIN32
        auto pos2 = path.rfind('\\');
        if (pos == std::string::npos || (pos2 != std::string::npos && pos2 > pos))
            pos = pos2;
#endif
        if (pos == std::string::npos)
            return ".";
        return path.substr(0, pos);
    }

    // Split "a/b/c" into ["a", "b", "c"]
    std::vector<std::string> split_key(const std::string &key)
    {
        std::vector<std::string> parts;
        std::string part;
        for (char c : key)
        {
            if (c == '/')
            {
                if (!part.empty())
                {
                    parts.push_back(std::move(part));
                    part.clear();
                }
            }
            else
            {
                part += c;
            }
        }
        if (!part.empty())
            parts.push_back(std::move(part));
        return parts;
    }
}

namespace tc
{
    Settings::Settings(const std::string &app_name)
        : _path(expand_home("~/.config/" + app_name + "/settings.json"))
    {
        _data.init(nos::trent_type::dict);
        load();
    }

    Settings::Settings(const std::string &path, bool /*explicit_path*/)
        : _path(expand_home(path))
    {
        _data.init(nos::trent_type::dict);
        load();
    }

    void Settings::load()
    {
        std::ifstream f(_path);
        if (!f.is_open())
            return;

        std::stringstream ss;
        ss << f.rdbuf();
        std::string content = ss.str();
        if (content.empty())
            return;

        try
        {
            _data = nos::json::parse(content);
            if (!_data.is_dict())
            {
                tc::Log::error("[Settings] Root is not a dict, resetting");
                _data.init(nos::trent_type::dict);
            }
        }
        catch (const std::exception &e)
        {
            tc::Log::error("[Settings] Failed to parse %s: %s", _path.c_str(), e.what());
            _data.init(nos::trent_type::dict);
        }
    }

    void Settings::save()
    {
        mkdir_p(dir_of(_path));
        std::ofstream f(_path);
        if (!f.is_open())
        {
            tc::Log::error("[Settings] Cannot write to %s", _path.c_str());
            return;
        }
        f << nos::json::dump(_data, 2);
    }

    std::string Settings::_resolve_key(const std::string &key) const
    {
        if (_group_stack.empty())
            return key;
        std::string prefix;
        for (const auto &g : _group_stack)
        {
            prefix += g + "/";
        }
        return prefix + key;
    }

    std::pair<nos::trent *, std::string>
    Settings::_navigate_mut(const std::string &full_key, bool create)
    {
        auto parts = split_key(full_key);
        if (parts.empty())
            return {nullptr, ""};

        std::string leaf = parts.back();
        parts.pop_back();

        nos::trent *node = &_data;
        for (const auto &p : parts)
        {
            if (!node->is_dict())
            {
                if (create)
                    node->init(nos::trent_type::dict);
                else
                    return {nullptr, leaf};
            }
            if (create)
            {
                // operator[] on dict creates entry if missing
                node = &(*node)[p];
                if (node->is_nil())
                    node->init(nos::trent_type::dict);
            }
            else
            {
                if (!node->contains(p.c_str()))
                    return {nullptr, leaf};
                node = &(*node)[p];
            }
        }

        if (!node->is_dict())
        {
            if (create)
                node->init(nos::trent_type::dict);
            else
                return {nullptr, leaf};
        }
        return {node, leaf};
    }

    std::pair<const nos::trent *, std::string>
    Settings::_navigate(const std::string &full_key) const
    {
        auto parts = split_key(full_key);
        if (parts.empty())
            return {nullptr, ""};

        std::string leaf = parts.back();
        parts.pop_back();

        const nos::trent *node = &_data;
        for (const auto &p : parts)
        {
            if (!node->is_dict())
                return {nullptr, leaf};
            if (!node->contains(p.c_str()))
                return {nullptr, leaf};
            node = &(*node)[p];
        }
        if (!node->is_dict())
            return {nullptr, leaf};
        return {node, leaf};
    }

    const nos::trent &Settings::get(const std::string &key) const
    {
        auto full = _resolve_key(key);
        auto [parent, leaf] = _navigate(full);
        if (!parent || !parent->contains(leaf.c_str()))
            return nos::trent::static_nil();
        return (*parent)[leaf];
    }

    nos::trent Settings::get(const std::string &key,
                             const nos::trent &default_value) const
    {
        auto full = _resolve_key(key);
        auto [parent, leaf] = _navigate(full);
        if (!parent || !parent->contains(leaf.c_str()))
            return default_value;
        const auto &val = (*parent)[leaf];
        if (val.is_nil())
            return default_value;
        return val;
    }

    void Settings::set(const std::string &key, const nos::trent &value)
    {
        auto full = _resolve_key(key);
        auto [parent, leaf] = _navigate_mut(full, true);
        if (parent)
        {
            (*parent)[leaf] = value;
            save();
        }
    }

    void Settings::remove(const std::string &key)
    {
        auto full = _resolve_key(key);
        auto [parent, leaf] = _navigate_mut(full, false);
        if (parent && parent->is_dict() && parent->contains(leaf.c_str()))
        {
            (*parent)[leaf] = nos::trent::nil();
            save();
        }
    }

    bool Settings::contains(const std::string &key) const
    {
        auto full = _resolve_key(key);
        auto [parent, leaf] = _navigate(full);
        if (!parent || !parent->contains(leaf.c_str()))
            return false;
        return !(*parent)[leaf].is_nil();
    }

    void Settings::begin_group(const std::string &name)
    {
        _group_stack.push_back(name);
    }

    void Settings::end_group()
    {
        if (!_group_stack.empty())
            _group_stack.pop_back();
    }
}
