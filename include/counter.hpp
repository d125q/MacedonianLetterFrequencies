#ifndef COUNTER_H
#define COUNTER_H

#include <map>
#include <mutex>
#include <functional>

template <typename K>
class counter final {
public:
    unsigned long long size_;    
    std::map<K, unsigned long long> *tally_;
    std::mutex mutex_;

public:
    counter();
    counter(const counter<K> &other);
    counter(counter<K> &&other) noexcept;
    counter& operator=(const counter<K> &other);
    counter& operator=(counter<K> &&other) noexcept;
    ~counter() noexcept;
    void increment(const K& key) noexcept;
    unsigned long long count_of(const K& key) noexcept;
    inline unsigned long long size();
    inline void for_each(const std::function<void(K, unsigned long long)>& fn);
};

template <typename K>
counter<K>::counter()
    : size_{0ull},
      tally_{new std::map<K, unsigned long long>()} {}

template <typename K>
counter<K>::counter(const counter<K> &other)
    : size_{other.size_},
      tally_{new std::map<K, unsigned long long>(*other.tally_)} {}

template <typename K>
counter<K>::counter(counter &&other) noexcept
    : size_{std::move(other.size_)},
      tally_{other.tally_}
{
    other.tally_ = nullptr;
}

template <typename K>
counter<K>& counter<K>::operator=(const counter<K> &other)
{
    counter tmp(other);
    *this = std::move(tmp);
    return *this;
}

template <typename K>
counter<K>& counter<K>::operator=(counter<K> &&other) noexcept
{
    delete tally_;
    tally_ = other.tally_;
    other.tally_ = nullptr;
    return *this;
}

template <typename K>
counter<K>::~counter() noexcept
{
    delete tally_;
}

template <typename K>
void counter<K>::increment(const K& key) noexcept
{
    // Ensure thread-safety with a RAII mechanism.
    std::lock_guard<std::mutex> guard(mutex_);

    if (tally_->find(key) == tally_->end())
	tally_->insert(std::make_pair(key, 1ull));
    else
	++tally_->at(key);

    ++size_;
}

template <typename K>
unsigned long long counter<K>::count_of(const K& key) noexcept
{
    // Prevent a caller from messing with the counter
    // by returning copies rather than references to inside
    // elements.  Ensure thread-safety with a RAII mechanism.
    std::lock_guard<std::mutex> guard(mutex_);

    if (tally_->find(key) == tally_->end())
	return 0ull;

    return tally_->find(key)->second;
}

template <typename K>
inline unsigned long long counter<K>::size() { return size_; }

template <typename K>
inline void counter<K>::for_each(const std::function<void(K, unsigned long long)>& fn)
{
    for (auto it = tally_->begin(); it != tally_->end(); ++it) {
	fn(it->first, it->second);
    }
}

#endif /* COUNTER_H */
