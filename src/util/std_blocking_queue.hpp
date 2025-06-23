#ifndef STD_BLOCKING_QUEUE_HPP
#define STD_BLOCKING_QUEUE_HPP

#include "blocking_queue.h""

#include <mutex>
#include <condition_variable>
#include <queue>


/**
 * Implementation of BlockingQueue using std::queue and std::mutex/std::condition_variable
 *
 * @tparam Capacity The amount of item to be stored in the queue
 * @tparam T The type of item to be stored in the queue
 */
template <size_t Capacity, typename T>
struct STDQueue final : BlockingQueue<Capacity, T>
{
    STDQueue() = default;
    ~STDQueue() override = default;

    /**
     * Offers a new item to the queue without waiting for space to become available in the queue;
     * if no space is available the item is skipped
     * @param src The item to offer to the queue
     * @return true if the item was successfully put on the queue
     */
    bool offer(const T& src) override
    {
        bool success = false;
        std::unique_lock lock(m_mutex);
        if (m_queue.size() < Capacity)
        {
            m_queue.emplace(src);
            success = true;
        }
        m_cvNotEmpty.notify_all();
        return success;
    }

    /**
     * Tries to take an item from the queue without waiting for the queue to have any items;
     * if no items are available on the queue, nothing will be written to the destination buffer
     * @param dest The destination to write the next queue item to if available
     * @return true if an item was taken from the queue
     */
    bool poll(T& dest) override
    {
        bool success = false;
        std::unique_lock lock(m_mutex);
        if (!m_queue.empty())
        {
            dest = m_queue.front();
            m_queue.pop();
            success = true;
        }
        m_cvNotFull.notify_all();
        return success;
    }

    /**
     * Puts a new item on the queue, waiting for space to become available on the queue
     * @param src The item to put on the queue
     */
    void put(const T& src) override
    {
        std::unique_lock lock(m_mutex);
        m_cvNotFull.wait(lock, [this] { return m_queue.size() < Capacity; });
        m_queue.emplace(src);
        m_cvNotEmpty.notify_all();
    }

    /**
     * Takes an item from the queue, waiting for an item to become available if the queue is emtpy
     * @param dest The destination to write the next queue item to
     */
    void take(T& dest) override
    {
        std::unique_lock lock(m_mutex);
        m_cvNotEmpty.wait(lock, [this] { return !m_queue.empty(); });
        dest = m_queue.front();
        m_queue.pop();
        m_cvNotFull.notify_all();
    }

private:
    std::mutex m_mutex{};
    std::queue<T> m_queue{};
    std::condition_variable m_cvNotEmpty{};
    std::condition_variable m_cvNotFull{};
};


#endif //STD_BLOCKING_QUEUE_HPP
