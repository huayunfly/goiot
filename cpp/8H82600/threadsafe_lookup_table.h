#ifndef THREADSAFE_LOOKUP_TABLE_H
#define THREADSAFE_LOOKUP_TABLE_H

#include <vector>
#include <list>
#include <utility>
#include <string>
#include <algorithm>
#include <shared_mutex>
#include <memory>

template<typename Key,
         typename Value,
         typename Hash=std::hash<Key> >
class ThreadSafeLookupTable
{
private:
    class BucketType
    {
    public:
        Value ValueFor(Key const& key,Value const& default_value)
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            BucketIterator const found_entry = FindEntryFor(key);
            return (found_entry == data_.end())?
                        default_value:found_entry->second;
        }

        void AddOrUpdateMapping(Key const& key,Value const& value)
        {
            std::unique_lock<std::shared_mutex> lock(mutex_);
            BucketIterator const found_entry = FindEntryFor(key);
            if(found_entry==data_.end())
            {
                data_.push_back(BucketValue(key,value));
            }
            else
            {
                found_entry->second = value;
            }
        }

        void RemoveMapping(Key const& key)
        {
            std::unique_lock<std::shared_mutex> lock(mutex_);
            BucketIterator const found_entry = FindEntryFor(key);
            if(found_entry != data_.end())
            {
                data_.erase(found_entry);
            }
        }

    private:
        typedef std::pair<Key,Value> BucketValue;
        typedef std::list<BucketValue> BucketData;
        typedef typename BucketData::iterator BucketIterator;
        BucketIterator FindEntryFor(Key const& key) // const
        {
            return std::find_if(data_.begin(), data_.end(),
                                [&](BucketValue const& item)
            {return item.first == key;});
        }

    private:
        BucketData data_;
        mutable std::shared_mutex mutex_;
    };

public:
    typedef Key KeyType;
    typedef Value MappedType;
    typedef Hash HashType;

    ThreadSafeLookupTable(
            unsigned num_buckets=19, Hash const& hasher_=Hash()):
        buckets_(num_buckets), hasher_(hasher_)
    {
        for(unsigned i = 0; i < num_buckets; ++i)
        {
            buckets_[i].reset(new BucketType);
        }
    }

    ThreadSafeLookupTable(ThreadSafeLookupTable const& other)=delete;

    ThreadSafeLookupTable& operator=(
            ThreadSafeLookupTable const& other)=delete;

    Value ValueFor(Key const& key,
                   Value const& default_value=Value()) const
    {
        return GetBucket(key).ValueFor(key,default_value);
    }

    void AddOrUpdateMapping(Key const& key, Value const& value)
    {
        GetBucket(key).AddOrUpdateMapping(key,value);
    }

    void RemoveMapping(Key const& key)
    {
        GetBucket(key).RemoveMapping(key);
    }

private:
    BucketType& GetBucket(Key const& key) const
    {
        std::size_t const bucket_index = hasher_(key)%buckets_.size();
        return *buckets_[bucket_index];
    }

private:
    std::vector<std::unique_ptr<BucketType> > buckets_;
    Hash hasher_;
};

#endif // THREADSAFE_LOOKUP_TABLE_H
