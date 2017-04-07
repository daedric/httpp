/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP_UTILS_SORTED_VECTOR_KEY_PAIR_HPP_
# define _HTTPP_UTILS_SORTED_VECTOR_KEY_PAIR_HPP_

# include <algorithm>
# include <vector>
# include <strings.h>

namespace HTTPP
{
namespace UTILS
{

struct case_insensitive
{
    template <typename StringLike>
    bool operator()(const StringLike & left, const StringLike & right)
    {
        if (left.size() < right.size())
        {
            return true;
        }
        else if (left.size() == right.size())
        {
            return ::strncasecmp(left.data(), right.data(), left.size()) < 0;
        }
        else
        {
            return false;
        }
    }
};

template <typename Key, typename Value, typename Comparator = std::less<Key>>
class SortedVectorKP : private std::vector<std::pair<Key, Value>>
{
    using Base = std::vector<std::pair<Key, Value>>;
    using value_type = typename Base::value_type;

    static bool comparator(const value_type& lhs, const value_type& rhs)
    {
        return Comparator()(lhs.first, rhs.first);
    }

public:
    using Base::begin;
    using Base::end;
    using Base::operator[];

    SortedVectorKP(const std::vector<std::pair<Key, Value>>& vector)
    : Base(std::begin(vector), std::end(vector))
    {
        std::sort(begin(), end(), &comparator);
    }

    Value const& find(const Key& key) const
    {
        const value_type value{key, not_found_};
        auto it = std::lower_bound(begin(), end(), value, &comparator);

        if (it != end() && !comparator(value, *it))
        {
            return it->second;
        }

        return not_found_;
    }

    Value const& operator[](const Key& key) const
    {
        return find(key);
    }

private:
    static const Value not_found_;

};

template <typename K, typename V, typename C=std::less<K>>
SortedVectorKP<K, V, C> create_sorted_vector(const std::vector<std::pair<K, V>>& vector)
{
    return SortedVectorKP<K, V, C>(vector);
}

template <typename K, typename V, typename C>
const V SortedVectorKP<K, V, C>::not_found_{};

} // namespace UTILS
} // namespace HTTPP


#endif // !_HTTPP_UTILS_SORTED_VECTOR_KEY_PAIR_HPP_
