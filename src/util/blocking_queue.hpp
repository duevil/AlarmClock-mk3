#ifndef BLOCKING_QUEUE_HPP
#define BLOCKING_QUEUE_HPP


/**
 * Interface for a blocking queue for transfer between threads
 *
 * @tparam T The type of item to be stored in the queue
 */
template <size_t, typename T>
struct BlockingQueue
{
    virtual ~BlockingQueue() = default;
    //! Offer a new item to the queue, skipping if the queue is full
    virtual bool offer(T const&) = 0;
    //! Try to take an item from the queue, skipping if the queue is empty
    virtual bool poll(T&) = 0;
    //! Put an item on the queue, waiting until the queue has free space
    virtual void put(T const&) = 0;
    //! Take an item from the queue, waiting until an item is available
    virtual void take(T&) = 0;
};


/**
 * Implementation of BlockingQueue using a static FreeRTOS queue
 *
 * @tparam Capacity The amount of item to be stored in the queue
 * @tparam T The type of item to be stored in the queue
 */
template <size_t Capacity, typename T>
struct ESPQueue final : BlockingQueue<Capacity, T>
{
    /**
     * Creates a new static FreeRTOS queue
     */
    ESPQueue()
    {
        m_queue = xQueueCreateStatic(Capacity, sizeof(T), m_queueStack, &m_queueBuf);
    }

    /**
     * Deletes the FreeRTOS queue
     */
    ~ESPQueue() override
    {
        vQueueDelete(m_queue);
    }

    /**
     * Offers a new item to the queue without waiting for space to become available in the queue;
     * if no space is available the item is skipped
     * @param src The item to offer to the queue
     * @return true if the item was successfully put on the queue
     */
    bool offer(T const& src) override
    {
        return m_queue && xQueueSend(m_queue, &src, 0) == pdTRUE;
    }

    /**
     * Tries to take an item from the queue without waiting for the queue to have any items;
     * if no items are available on the queue, nothing will be written to the destination buffer
     * @param dest The destination to write the next queue item to if available
     * @return true if an item was taken from the queue
     */
    bool poll(T& dest) override
    {
        return m_queue && xQueueReceive(m_queue, &dest, 0) == pdTRUE;
    }

    /**
     * Puts a new item on the queue, waiting for space to become available on the queue
     * @param src The item to put on the queue
     */
    void put(T const& src) override
    {
        if (m_queue)
        {
            xQueueSend(m_queue, &src, portMAX_DELAY);
        }
    }

    /**
     * Takes an item from the queue, waiting for an item to become available if the queue is emtpy
     * @param dest The destination to write the next queue item to
     */
    void take(T& dest) override
    {
        if (m_queue)
        {
            xQueueReceive(m_queue, &dest, portMAX_DELAY);
        }
    }

private:
    QueueHandle_t m_queue = nullptr;
    StaticQueue_t m_queueBuf{};
    StackType_t m_queueStack[Capacity * sizeof(T)]{};
};


#endif //BLOCKING_QUEUE_HPP
