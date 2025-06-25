#ifndef THREAD_H
#define THREAD_H

#ifndef THREAD_DEFAULT_STACK_SIZE
#define THREAD_DEFAULT_STACK_SIZE 2048
#endif


/**
 * Thread configuration
 */
struct ThreadCfg
{
    //!The name to be assigned to this task
    const char* name = nullptr;
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
     * Creates a new thread task with the given configuration
     * @param cfg Thread configuration
     */
    explicit Thread(const ThreadCfg& cfg = {})
    {
        // ReSharper disable once CppDFAEndlessLoop
        m_task =
            xTaskCreateStaticPinnedToCore([](void* t) { while (true) static_cast<Thread*>(t)->run(); },
                                          cfg.name, STACK, this, cfg.priority, m_taskStack, &m_taskBuf, cfg.coreId);
    }

    virtual ~Thread()
    {
        vTaskDelete(m_task);
    }

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

    // deleted copy constructor
    Thread(const Thread&) = delete;

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
 * Implementation of Thread storing a callable object to be used as the thread function
 *
 * @tparam STACK Stack size
 */
template <typename T, size_t STACK = THREAD_DEFAULT_STACK_SIZE>
struct FuncThread final : Thread<STACK>
{
    /**
     * Creates a new thread task using the given function and configuration
     * @param func Thread function
     * @param cfg Thread configuration
     */
    explicit FuncThread(T&& func, const ThreadCfg& cfg = {}) : Thread<STACK>(cfg), m_func(std::forward<T>(func)) {}

protected:
    void run() override { m_func(); }

private:
    T m_func{};
};


#endif //THREAD_H
