#ifndef THREAD_HPP
#define THREAD_HPP

#ifndef THREAD_DEFAULT_STACK_SIZE
#define THREAD_DEFAULT_STACK_SIZE 4096
#endif


/**
 * Thread configuration
 */
struct ThreadCfg
{
    //!The name to be assigned to this task
    const char* name = nullptr;
    //! The size of the task's stack
    int stackSize = THREAD_DEFAULT_STACK_SIZE;
    //! The priority of the task; defaults to idle priority
    int priority = tskIDLE_PRIORITY;
    //! The core for the task to be pinned to; defaults to no affinity
    int coreId = tskNO_AFFINITY;
};


/**
 * Thread implementation as a wrapper around the FreeRTOS task API using a static task;
 * the thread runs immediately after calling the constructor and executes the run-method inside the task,
 * which must be implemented by any class implementing a thread
 *
 * @tparam STACK The size of the task stack; default: THREAD_DEFAULT_STACK_SIZE
 */
template <size_t STACK = THREAD_DEFAULT_STACK_SIZE>
struct Thread
{
    /**
     * Creates a new thread task with the given name
     * @param cfg Thread configuration
     */
    explicit Thread(const ThreadCfg &cfg = {})
    {
        // ReSharper disable once CppDFAEndlessLoop
        m_task =
            xTaskCreateStaticPinnedToCore([](void* t) { while (true) static_cast<Thread*>(t)->run(); },
                                          cfg.name, STACK, this, cfg.priority, m_taskStack, &m_taskBuf, cfg.coreId);
    }

    // deleted copy constructor
    Thread(const Thread&) = delete;

    /**
     * Suspends the thread task
     */
    void suspend() const
    {
        vTaskSuspend(m_task);
    }

    /**
     * Resumes the thread task
     */
    void resume() const
    {
        vTaskResume(m_task);
    }

    /**
     * Deletes the thread task
     */
    virtual ~Thread()
    {
        vTaskDelete(m_task);
    }

protected:
    /**
     * The main function to be run inside the thread task
     */
    virtual void run() = 0;

private:
    TaskHandle_t m_task{};
    StaticTask_t m_taskBuf{};
    StackType_t m_taskStack[STACK]{};
};


/**
 * Implementation of Thread allowing the use of a lambda or function pointer as the task function
 *
 * @tparam TFunc Function argument type
 * @tparam STACK Stack size
 */
template <typename TFunc, size_t STACK = THREAD_DEFAULT_STACK_SIZE>
struct ThreadFunc final : Thread<STACK>
{
    explicit ThreadFunc(TFunc func, const ThreadCfg &cfg = {}) : Thread<STACK>(cfg), m_func(func) {}

    ThreadFunc(const ThreadFunc&) = delete;

protected:
    void run() override { m_func(); }

private:
    TFunc m_func;
};


#endif //THREAD_HPP
