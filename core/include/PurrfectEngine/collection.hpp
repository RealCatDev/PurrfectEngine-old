#ifndef PURRENGINE_COLLECTION_HPP_
#define PURRENGINE_COLLECTION_HPP_

#include <iostream>
#include <map>
#include <vector>
#include <random>
#include <stdexcept>
#include <algorithm>


namespace PurrfectEngine {

  template <typename K, typename V>
  class Collection {
  private:
    std::map<K, V> data_;

  public:
    void insert(const K& key, const V& value) {
      data_[key] = value;
    }

    // Ensure that the key exists with the specified value
    void ensure(const std::string& key, const std::string& value) {
        if (!data_.count(key)) {
            data_[key] = value;
        }
    }

    // Check if all keys exist in the collection
    bool hasAll(const std::vector<std::string>& keys) const {
        for (const auto& key : keys) {
            if (!data_.count(key)) {
                return false;
            }
        }
        return true;
    }

    // Check if any key exists in the collection
    bool hasAny(const std::vector<std::string>& keys) const {
        for (const auto& key : keys) {
            if (data_.count(key)) {
                return true;
            }
        }
        return false;
    }

    // Get the first element in the collection
    std::pair<std::string, std::string> first() const {
        if (!data_.empty()) {
            return *data_.begin();
        }
        throw std::out_of_range("Collection is empty");
    }

    std::pair<std::string, std::string> first(size_t amount = 1) const {
        if (amount == 0) {
            return {};
        }

        auto it = data_.begin();
        std::advance(it, std::min(amount, data_.size()));

        return *it;
    }

    // Get the last element in the collection
    std::pair<std::string, std::string> last() const {
        if (!data_.empty()) {
            return *data_.rbegin();
        }
        throw std::out_of_range("Collection is empty");
    }

    std::pair<std::string, std::string> last(size_t amount = 1) const {
        if (amount == 0) {
            return {};
        }

        auto it = data_.rbegin();
        std::advance(it, std::min(amount, data_.size()));

        return *it;
    }

    // Get the key of the first element in the collection
    std::string firstKey() const {
        if (!data_.empty()) {
            return data_.begin()->first;
        }
        throw std::out_of_range("Collection is empty");
    }

    std::string firstKey(size_t amount = 1) const {
        if (amount == 0) {
            return "";
        }

        std::vector<std::string> keys;
        keys.reserve(amount);

        auto it = data_.begin();
        while (it != data_.end() && keys.size() < amount) {
            keys.push_back(it->first);
            ++it;
        }

        if (keys.size() == 1) {
            return keys.front();
        } else if (keys.size() > 1) {
            return keys;
        }

        return "";
    }

    // Get the key of the last element in the collection
    std::string lastKey() const {
        if (!data_.empty()) {
            return data_.rbegin()->first;
        }
        throw std::out_of_range("Collection is empty");
    }

    std::string lastKey(size_t amount = 1) const {
        if (amount == 0) {
            return "";
        }

        std::vector<std::string> keys;
        keys.reserve(amount);

        auto it = data_.rbegin();
        while (it != data_.rend() && keys.size() < amount) {
            keys.push_back(it->first);
            ++it;
        }

        if (keys.size() == 1) {
            return keys.front();
        } else if (keys.size() > 1) {
            return keys;
        }

        return "";
    }

    // Access the value at the specified key
    std::string at(const std::string& key) const {
        auto it = data_.find(key);
        if (it != data_.end()) {
            return it->second;
        }
        throw std::out_of_range("Key not found in collection");
    }

    // Access the key at the specified index
    std::string keyAt(size_t index) const {
        if (index < data_.size()) {
            auto it = data_.begin();
            std::advance(it, index);
            return it->first;
        }
        throw std::out_of_range("Index out of range");
    }

    // Get a random element from the collection
    std::pair<std::string, std::string> random() const {
        if (!data_.empty()) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, data_.size() - 1);
            auto it = data_.begin();
            std::advance(it, dis(gen));
            return *it;
        }
        throw std::out_of_range("Collection is empty");
    }

    std::vector<std::string> random(size_t amount) const {
        if (amount >= data_.size()) {
            return {};
        }

        std::vector<std::string> keys;
        keys.reserve(amount);

        std::random_device rd;
        std::mt19937 g(rd());

        std::vector<size_t> indices(data_.size());
        std::iota(indices.begin(), indices.end(), 0);

        std::shuffle(indices.begin(), indices.end(), g);

        for (size_t i = 0; i < amount; ++i) {
            keys.push_back(data_[indices[i]].first);
        }

        return keys;
    }

    std::string randomKey(size_t amount = 1) const {
        if (amount == 0) {
            return "";
        }

        std::vector<std::string> keys = random(amount);

        if (keys.size() == 1) {
            return keys.front();
        } else if (keys.size() > 1) {
            return keys;
        }

        return "";
    }

    Collection reverse() const {
        Collection reversed;
        for (auto it = data_.rbegin(); it != data_.rend(); ++it) {
            reversed.insert(it->first, it->second);
        }
        return reversed;
    }

    template <typename Func>
    V find(Func&& fn) const {
        for (const auto& [key, value] : data_) {
            if (fn(value, key, *this)) {
                return value;
            }
        }
        return V{};
    }

    template <typename Func>
    std::vector<V> findAll(Func&& fn) const {
        std::vector<V> result;
        for (const auto& [key, value] : data_) {
            if (fn(value, key, *this)) {
                result.push_back(value);
            }
        }
        return result;
    }

    template <typename Func>
    K findKey(Func&& fn) const {
        for (const auto& [key, value] : data_) {
            if (fn(value, key, *this)) {
                return key;
            }
        }
        return K{};
    }

    template <typename Func>
    std::vector<K> findAllKeys(Func&& fn) const {
        std::vector<K> result;
        for (const auto& [key, value] : data_) {
            if (fn(value, key, *this)) {
                result.push_back(key);
            }
        }
        return result;
    }

    template <typename Func>
    size_t sweep(Func&& fn) {
        size_t removedCount = 0;
        auto it = data_.begin();
        while (it != data_.end()) {
            if (fn(it->second, it->first, *this)) {
                it = data_.erase(it);
                ++removedCount;
            } else {
                ++it;
            }
        }
        return removedCount;
    }

    template <typename T, typename Func>
    size_t sweep(Func&& fn, T&& thisArg) {
        size_t removedCount = 0;
        auto it = data_.begin();
        while (it != data_.end()) {
            if (fn(thisArg, it->second, it->first, *this)) {
                it = data_.erase(it);
                ++removedCount;
            } else {
                ++it;
            }
        }
        return removedCount;
    }

    template <typename Func>
    Collection<K, V> filter(Func&& fn) const {
        Collection<K, V> result;
        for (const auto& [key, value] : data_) {
            if (fn(value, key, *this)) {
                result.insert(key, value);
            }
        }
        return result;
    }

    template <typename This, typename Func>
    Collection<K, V> filter(Func&& fn, This&& thisArg) const {
        Collection<K, V> result;
        for (const auto& [key, value] : data_) {
            if (fn(thisArg, value, key, *this)) {
                result.insert(key, value);
            }
        }
        return result;
    }

    template <typename Func>
    std::pair<
        Collection<K, V>,
        Collection<K, V>
    > partition(Func&& predicate) const {
        Collection<K, V> passing;
        Collection<K, V> failing;

        for (const auto& [key, value] : data_) {
            if (predicate(value, key, *this)) {
                passing.insert(key, value);
            } else {
                failing.insert(key, value);
            }
        }

        return std::make_pair(passing, failing);
    }
  };
}


#endif // PURRENGINE_COLLECTION_HPP_