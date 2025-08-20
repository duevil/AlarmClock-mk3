#ifndef BOOT_PROCESS_HPP
#define BOOT_PROCESS_HPP

#include "events.hpp"
#include <queue>


//! Event posted when a boot process completes
EVENT_DEFINE(BOOT_EVENT);

/**
 * Structure defining a bootable process.
 *
 * Creating a boot process appends it to a list of process, which can be used to execute all boot processes at once.
 *
 * Running a boot process will emmit an event with the process id when the process is completed.
 * When all processes are completed, <code>EVENT_ALL_COMPLETED</code> is emitted.
 *
 * Each boot process is assigned a description string, which can be queried using the process id.
 *
 * Classes implementing a boot process must implement the <code>runBootProcess()</code> method.
 *
 * @note The internal list does not store a copy of the registered process uses
 * but instead uses a pointer to the processes, so the user must ensure that each process is still
 * in memory when calling <code>runAll()</code>
 */
struct BootProcess
{
    static constexpr int32_t EVENT_ALL_COMPLETED = INT32_MAX;

    /**
     * Create a new boot process and append it to the list of processes
     * @param description The description of this process
     */
    explicit BootProcess(const char* description) : m_description(description)
    {
        s_processes.emplace(this);
    }

    virtual ~BootProcess() = default;

    /**
     * Run all boot processes
     *
     * @note Should only be called once as a process is removed from the internal list upon completion,
     * so any later calls will only emit <code>EVENT_ALL_COMPLETED</code> again
     */
    static void runAll()
    {
        while (!s_processes.empty())
        {
            if (auto process = s_processes.front())
            {
                process->runBootProcess();
                BOOT_EVENT << process->m_id << process->m_description;
            }
            s_processes.pop();
        }
        BOOT_EVENT << EVENT_ALL_COMPLETED;
    }

    static int32_t count()
    {
        return s_counter;
    }

    // delete copy constructor and assignment operator

    BootProcess(const BootProcess&) = delete;
    BootProcess& operator=(const BootProcess&) = delete;

protected:
    virtual void runBootProcess() = 0;

private:
    int32_t m_id = s_counter++;
    const char* m_description = nullptr;

    inline static int32_t s_counter{};
    inline static std::queue<BootProcess*> s_processes{};
};


/**
 * Helper struct to define boot processes using a callable (lambda, function pointer)
 * @tparam Func Callable type
 */
template <typename Func>
struct FuncBootProcess final : BootProcess
{
    explicit FuncBootProcess(const char* description, Func func): BootProcess(description), m_func(func) {}

private:
    void runBootProcess() override { m_func(); }
    Func m_func;
};


#endif //BOOT_PROCESS_HPP
