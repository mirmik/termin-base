#ifndef TCBASE_SETTINGS_H
#define TCBASE_SETTINGS_H

#include <string>
#include <vector>
#include <tcbase/trent/trent.h>
#include <tcbase/trent/json.h>

namespace tc
{
    class Settings
    {
    public:
        /// Construct with app name -> ~/.config/{app_name}/settings.json
        explicit Settings(const std::string &app_name);

        /// Construct with explicit file path
        Settings(const std::string &path, bool explicit_path);

        /// Get value by hierarchical key ("a/b/c").
        /// Returns static nil if key not found.
        const nos::trent &get(const std::string &key) const;

        /// Get value with default fallback.
        nos::trent get(const std::string &key,
                       const nos::trent &default_value) const;

        /// Set value by hierarchical key. Creates intermediate dicts.
        void set(const std::string &key, const nos::trent &value);

        /// Remove a key.
        void remove(const std::string &key);

        /// Check if key exists and is not nil.
        bool contains(const std::string &key) const;

        /// Push a group prefix onto the stack.
        void begin_group(const std::string &name);

        /// Pop the last group prefix.
        void end_group();

        /// Load settings from disk. Called automatically on construction.
        void load();

        /// Save settings to disk. Called automatically on set/remove.
        void save();

        /// File path.
        const std::string &path() const { return _path; }

    private:
        std::string _path;
        nos::trent _data;
        std::vector<std::string> _group_stack;

        std::string _resolve_key(const std::string &key) const;

        // Navigate to parent dict + leaf key.
        // Returns (parent_ptr, leaf_key). parent_ptr is null if not found.
        std::pair<nos::trent *, std::string>
        _navigate_mut(const std::string &full_key, bool create);

        std::pair<const nos::trent *, std::string>
        _navigate(const std::string &full_key) const;
    };
}

#endif
