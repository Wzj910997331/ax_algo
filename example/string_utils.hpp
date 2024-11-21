#pragma once
#include <string>
#include <vector>
#include <algorithm>

namespace string_utils
{
    /// Remove whitespace from the left side of a string
    ///
    /// @param str The string to trim
    /// @return The trimmed string
    static std::string ltrim(const std::string &str)
    {
        return str.substr(str.find_first_not_of(' '));
    }
    /// Remove whitespace from the right side of a string
    ///
    /// @param str The string to trim
    /// @return The trimmed string
    static std::string rtrim(const std::string &str)
    {
        return str.substr(0, str.find_last_not_of(' '));
    }

    /// Remove whitespace from both sides of a string
    ///
    /// @param str The string to trim
    /// @return The trimmed string
    static std::string trim(const std::string &str)
    {
        return ltrim(rtrim(str));
    }

    /// Check if a string contains a given substring
    ///
    /// @param str The string to check
    /// @param substr The substring to search for
    /// @return True if the string contains the substring, false otherwise
    static bool contains(const std::string &str, const std::string &substr)
    {
        return str.find(substr) != std::string::npos;
    }

    /// Convert a string to lowercase
    ///
    /// @param str The string to convert
    /// @return The string with all characters converted to lowercase
    static std::string tolower(const std::string &str)
    {
        std::string ret = str;
        std::transform(ret.begin(), ret.end(), ret.begin(), [](unsigned char c)
                       { return std::tolower(c); });
        return ret;
    }

    /// Convert a string to uppercase
    ///
    /// @param str The string to convert
    /// @return The string with all characters converted to uppercase
    static std::string toupper(const std::string &str)
    {
        std::string ret = str;
        std::transform(ret.begin(), ret.end(), ret.begin(), [](unsigned char c)
                       { return std::toupper(c); });
        return ret;
    }

    /// Check if a string ends with a given suffix
    ///
    /// @param str The string to check
    /// @param suffix The suffix to search for
    /// @return True if the string ends with the suffix, false otherwise
    static bool endswith(const std::string &str, const std::string &suffix)
    {
        return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    /// Check if a string starts with a given prefix
    ///
    /// @param str The string to check
    /// @param prefix The prefix to search for
    /// @return True if the string starts with the prefix, false otherwise
    static bool startswith(const std::string &str, const std::string &suffix)
    {
        return str.size() >= suffix.size() && str.compare(0, suffix.size(), suffix) == 0;
    }

    /// Split a string into a vector of strings, separated by a given delimiter
    ///
    /// @param str The string to split
    /// @param delim The delimiter to split on
    /// @return A vector of strings, each representing a token in the
    ///         original string
    static std::vector<std::string> split(const std::string &str, const std::string &delim)
    {
        std::vector<std::string> tokens;
        size_t prev = 0, pos = 0;
        do
        {
            pos = str.find(delim, prev);
            if (pos == std::string::npos)
                pos = str.length();
            std::string token = str.substr(prev, pos - prev);
            if (!token.empty())
                tokens.push_back(token);
            prev = pos + delim.length();
        } while (pos < str.length() && prev < str.length());
        return tokens;
    }

    static std::string basename(const std::string &path)
    {
        size_t pos = path.find_last_of('/');
        if (pos == std::string::npos)
            return path;
        return path.substr(pos + 1);
    }

    static std::string dirname(const std::string &path)
    {
        size_t pos = path.find_last_of('/');
        if (pos == std::string::npos)
            return "";
        return path.substr(0, pos);
    }

    static std::string extension(const std::string &path)
    {
        size_t pos = path.find_last_of('.');
        if (pos == std::string::npos)
            return "";
        return path.substr(pos + 1);
    }

    static std::string join(const std::string &part)
    {
        return part;
    }

    template <typename... Args>
    static std::string join(const std::string &part, const Args &...parts)
    {
        std::string result = part;
        std::string next_part = join(parts...); // 递归调用处理剩余的参数

        if (!next_part.empty())
        {
            if (startswith(next_part, "/") || endswith(result, "/"))
            {
                result += next_part.substr(1); // 去掉下一个部分的开头 "/"
            }
            else
            {
                result += "/" + next_part; // 加上 "/" 连接
            }
        }

        return result;
    }
}
