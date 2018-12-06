#ifndef AISDI_MAPS_HASHMAP_H
#define AISDI_MAPS_HASHMAP_H

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <vector>
#include <list>
#include <algorithm>

namespace aisdi
{

template <typename KeyType, typename ValueType>
class HashMap
{
public:
  using key_type = KeyType;
  using mapped_type = ValueType;
  using value_type = std::pair<key_type, mapped_type>;
  using size_type = std::size_t;
  using reference = value_type &;
  using const_reference = const value_type &;
  using list_type = std::list<value_type>;
  using table_type = std::vector<list_type>;

  class ConstIterator;
  class Iterator;
  using iterator = Iterator;
  using const_iterator = ConstIterator;

  HashMap() : table(table_type(initialBucketsNumber)), buckets(initialBucketsNumber), size(0) {}

  HashMap(std::initializer_list<value_type> list)
  {
    size_type s = list.size();
    size = s;

    s = s < initialBucketsNumber ? initialBucketsNumber : s;
    table = table_type(s);
    buckets = s;

    for (auto &item : list)
      table[hash(item.first)].push_back(item);
  }

  bool isEmpty() const
  {
    return size == 0;
  }

  mapped_type &operator[](const key_type &key)
  {
    auto &list = table[hash(key)];
    auto it = std::find_if(list.begin(), list.end(), [&key](const value_type &other) { return other.first == key; });
    if (it == list.end())
    {
      list.emplace_back(key, ValueType{});
      if (++size >= buckets * 10 / 9)
      {
        doubleCapacity();
        return valueOf(key);
      }

      return list.back().second;
    }
    return it->second;
  }

  const mapped_type &valueOf(const key_type &key) const
  {
    const_iterator it = find(key);
    if (it == end())
      throw std::out_of_range("Key does not exists");

    return it->second;
  }

  mapped_type &valueOf(const key_type &key)
  {
    iterator it = find(key);
    if (it == end())
      throw std::out_of_range("Key does not exists");

    return it->second;
  }

  const_iterator find(const key_type &key) const
  {
    return constIteratorFind(key);
  }

  iterator find(const key_type &key)
  {
    return iterator(constIteratorFind(key));
  }

  void remove(const key_type &key)
  {
    auto &list = table[hash(key)];
    auto it = findKeyInList(key, list);

    if (it == list.end())
      throw std::out_of_range("Removing non existing key");

    list.erase(it);
    --size;
  }

  void remove(const const_iterator &it)
  {
    if (it == end())
      throw std::out_of_range("Removing end iterator");

    auto &list = table[it.bucketNumber];
    list.erase(it.bucketIterator);
    --size;
  }

  size_type getSize() const
  {
    return size;
  }

  bool operator==(const HashMap &other) const
  {
    for (auto &item : other)
    {
      auto it = find(item.first);
      if (it == end() || it->second != item.second)
        return false;
    }
    return size == other.size;
  }

  bool operator!=(const HashMap &other) const
  {
    return !(*this == other);
  }

  iterator begin()
  {
    return iterator(cbegin());
  }

  iterator end()
  {
    return iterator(cend());
  }

  const_iterator cbegin() const
  {
    if (size == 0)
      return cend();

    auto &firstBucket = table[0];
    auto it = firstBucket.begin();
    if (firstBucket.size() != 0)
      return const_iterator(table, 0, it);
    return ++const_iterator(table, 0, it);
  }

  const_iterator cend() const
  {
    return const_iterator(table, buckets - 1, table[buckets - 1].end());
  }

  const_iterator begin() const
  {
    return cbegin();
  }

  const_iterator end() const
  {
    return cend();
  }

private:

  table_type table;
  size_type buckets;
  size_type size;

  static const size_type initialBucketsNumber = 8;

  size_type hash(const key_type &key) const
  {
    return (std::hash<key_type>{}(key)) % buckets;
  }

  const_iterator constIteratorFind(const key_type &key) const
  {
    auto bucket = hash(key);
    auto &list = table[bucket];
    auto it = findKeyInList(key, list);

    if (it == list.end())
      return cend();

    return const_iterator(table, bucket, it);
  }

  static typename  list_type::const_iterator findKeyInList(const key_type &key, const list_type &list)
  {
    auto it = std::find_if(std::begin(list), std::end(list),
                        [&key](const value_type &other) { return other.first == key; }); 
    return it; 
  }

  // static typename  list_type::iterator findKeyInList(const key_type &key, list_type &list)
  // {
  //   auto it = findKeyInList(key, list);
  //   return list.erase(it, it); 
  // }

  void doubleCapacity()
  {
    buckets *= 2;
    auto newTable = table_type(buckets);
    for (auto &&bucket : table)
    {
      auto it = bucket.begin();
      while (it != bucket.end())
      {
        auto &newBucket = newTable[hash(it->first)];
        newBucket.splice(newBucket.begin(), bucket, it);
        it = bucket.begin();
      }
    }
    table = std::move(newTable);
  }
};

template <typename KeyType, typename ValueType>
class HashMap<KeyType, ValueType>::ConstIterator
{
public:
  using reference = typename HashMap::const_reference;
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = typename HashMap::value_type;
  using pointer = const typename HashMap::value_type *;
  using size_type = HashMap::size_type;
  using list_type = typename HashMap::list_type;
  using list_iterator = typename list_type::const_iterator;
  using table_type = const HashMap::table_type;

  explicit ConstIterator(table_type &buckets, size_type bucketNumber,
                         list_iterator it) : buckets(buckets), bucketNumber(bucketNumber), bucketIterator(it)
  {
  }

  ConstIterator(const ConstIterator &other) : buckets(other.buckets), bucketNumber(other.bucketNumber), bucketIterator(other.bucketIterator)
  {
  }

  ConstIterator &operator++()
  {
    operator++(0);
    return *this;
  }

  ConstIterator operator++(int)
  {
    auto &bucket = buckets[bucketNumber];

    if (bucketNumber + 1 >= buckets.size() && bucketIterator == bucket.end())
      throw std::out_of_range("Incrementing end iterator");

    if (++bucketIterator == bucket.end())
    {
      while (true)
      {
        auto &bucket = buckets[++bucketNumber];
        if (bucket.size() != 0)
        {
          bucketIterator = bucket.begin();
          break;
        }
        if (bucketNumber >= buckets.size() - 1)
        {
          bucketIterator = bucket.end();
          break;
        }
      }
    }
    return *this;
  }

  ConstIterator &operator--()
  {
    operator--(0);
    return *this;
  }

  ConstIterator operator--(int)
  {
    auto &bucket = buckets[bucketNumber];
    if (bucketIterator == bucket.begin())
    {
      while (true)
      {
        if (bucketNumber == 0)
          throw std::out_of_range("Decrementing begin iterator");

        auto &bucket = buckets[--bucketNumber];
        if (bucket.size() != 0)
        {
          bucketIterator = --(bucket.end());
          break;
        }
      }
    }
    return *this;
  }

  reference operator*() const
  {
    if (bucketIterator == buckets[buckets.size() - 1].end())
      throw std::out_of_range("Dereferencing end iterator");
    return *bucketIterator;
  }

  pointer operator->() const
  {
    return &this->operator*();
  }

  bool operator==(const ConstIterator &other) const
  {
    return bucketNumber == other.bucketNumber && bucketIterator == other.bucketIterator;
  }

  bool operator!=(const ConstIterator &other) const
  {
    return !(*this == other);
  }

  table_type &buckets;
  size_type bucketNumber;
  list_iterator bucketIterator;
};

template <typename KeyType, typename ValueType>
class HashMap<KeyType, ValueType>::Iterator : public HashMap<KeyType, ValueType>::ConstIterator
{
public:
  using reference = typename HashMap::reference;
  using pointer = typename HashMap::value_type *;

  explicit Iterator()
  {
  }

  Iterator(const ConstIterator &other)
      : ConstIterator(other)
  {
  }

  Iterator &operator++()
  {
    ConstIterator::operator++();
    return *this;
  }

  Iterator operator++(int)
  {
    auto result = *this;
    ConstIterator::operator++();
    return result;
  }

  Iterator &operator--()
  {
    ConstIterator::operator--();
    return *this;
  }

  Iterator operator--(int)
  {
    auto result = *this;
    ConstIterator::operator--();
    return result;
  }

  pointer operator->() const
  {
    return &this->operator*();
  }

  reference operator*() const
  {
    // ugly cast, yet reduces code duplication.
    return const_cast<reference>(ConstIterator::operator*());
  }
};

} // namespace aisdi

#endif /* AISDI_MAPS_HASHMAP_H */
