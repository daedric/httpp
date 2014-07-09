/*
 * Part of HTTPP.
 *
 * Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
 *
 */

#ifndef _HTTPP_UTILS_SORTED_VECTOR_KEY_PAIR_HPP_
# define _HTTPP_UTILS_SORTED_VECTOR_KEY_PAIR_HPP_

# include <algorithm>
# include <vector>

namespace HTTPP
{
namespace UTILS
{

template <typename Key, typename Value>
class SortedVectorKP : private std::vector<std::pair<Key, Value>>
{
    using Base = std::vector<std::pair<Key, Value>>;
    using value_type = typename Base::value_type;

    static bool comparator(const value_type& lhs, const value_type& rhs)
    {
        return lhs.first < rhs.first;
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
        auto it = std::lower_bound(
            begin(), end(), value_type{ key, not_found_ }, &comparator);

        if (it != end() && it->first == key)
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

template <typename K, typename V>
SortedVectorKP<K, V> create_sorted_vector(const std::vector<std::pair<K, V>>& vector)
{
    return SortedVectorKP<K, V>(vector);
}

template <typename K, typename V>
const V SortedVectorKP<K, V>::not_found_;

} // namespace UTILS
} // namespace HTTPP


#endif // !_HTTPP_UTILS_SORTED_VECTOR_KEY_PAIR_HPP_
