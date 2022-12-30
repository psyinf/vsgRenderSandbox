#include <atomic>
#include <cstddef>
#include <vector>

template <typename T>
class SPSCQueue
{
public:
    explicit SPSCQueue(std::size_t capacity)
        : capacity_(capacity)
        , buffer_(capacity)
        , read_index_(0)
        , write_index_(0)
    {
    }

    // Move constructor
    SPSCQueue(SPSCQueue&& other)
        : capacity_(other.capacity_)
        , buffer_(std::move(other.buffer_))
        , read_index_(other.read_index_.load())
        , write_index_(other.write_index_.load())
    {
    }

    // Move assignment operator
    SPSCQueue& operator=(SPSCQueue&& other)
    {
        capacity_    = other.capacity_;
        buffer_      = std::move(other.buffer_);
        read_index_  = other.read_index_.load();
        write_index_ = other.write_index_.load();
        return *this;
    }

    bool push(T&& item)
    {
        std::size_t write      = write_index_.load(std::memory_order_relaxed);
        std::size_t next_write = (write + 1) % capacity_;
        if (next_write != read_index_.load(std::memory_order_acquire))
        {
            buffer_[write] = std::move(item);
            write_index_.store(next_write, std::memory_order_release);
            return true;
        }
        return false;
    }

    bool pop(T& item)
    {
        std::size_t read = read_index_.load(std::memory_order_relaxed);
        if (read == write_index_.load(std::memory_order_acquire))
        {
            return false;
        }
        item = std::move(buffer_[read]);
        read_index_.store((read + 1) % capacity_, std::memory_order_release);
        return true;
    }

private:
    std::size_t              capacity_;
    std::vector<T>           buffer_;
    std::atomic<std::size_t> read_index_;
    std::atomic<std::size_t> write_index_;

    // Disable copy and assignment
    SPSCQueue(const SPSCQueue&)            = delete;
    SPSCQueue& operator=(const SPSCQueue&) = delete;
};