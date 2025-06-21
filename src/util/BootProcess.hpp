#ifndef BOOT_PROCESS_HPP
#define BOOT_PROCESS_HPP


//! Event posted when a boot process completes
EVENT_DEFINE(BOOT_EVENT);

/**
 * Structure defining a bootable process.
 *
 * Creating a boot process appends it to a list of process,
 * which can be used to execute all boot processes at once.
 *
 * Running a boot process will emmit an event with the process id
 * when the process is completed.
 *
 * Each boot process is assigned a description string, which can
 * be queried using the process id.
 *
 * Classes implementing a boot process must implement the <code>run()</code> method.
 *
 * @note The internal list does not store a copy of the registered process uses
 * but instead uses a pointer to the processes, so the user must ensure that each process is still
 * in memory when calling <code>runAll()</code>
 */
struct BootProcess
{
    /**
     * Create a new boot process and append it to the list of processes
     * @param description The description of this process
     */
    explicit BootProcess(const char* description)
    {
        s_processes.emplace(this);
        s_descriptions.emplace(m_id, description);
    }

    virtual ~BootProcess() = default;

    /**
     * Run all boot processes
     *
     * @note should only be called once as a process is removed from the internal list upon completion,
     * so any later calls will do nothing
     */
    static void runAll()
    {
        while (!s_processes.empty())
        {
            if (auto process = s_processes.front())
            {
                process->runProcess();
                BOOT_EVENT << static_cast<int32_t>(process->m_id);
            }
            s_processes.pop();
        }
    }

    /**
     * Get the description for a boot process
     * @param id The process id
     * @return The description of the boot process
     * or nullptr if no process is found for the given id
     */
    static const char* description(int32_t id)
    {
        if (auto it = s_descriptions.find(id); it != s_descriptions.end())
        {
            return it->second;
        }
        return nullptr;
    }

    /**
     * Get the number of boot processes
     * @return The total number of registered boot processes
     */
    static auto count()
    {
        return s_processes.size();
    }

protected:
    virtual void runProcess() = 0;

private:
    auto m_id = static_cast<int32_t>(s_processes.size());

    static std::queue<BootProcess*> s_processes;
    static std::unordered_map<int32_t, const char*> s_descriptions;
};

decltype(BootProcess::s_processes) BootProcess::s_processes{};
decltype(BootProcess::s_descriptions) BootProcess::s_descriptions{};


/**
 * Helper struct to define boot processes using a callable (lambda, function pointer)
 * @tparam Func Callable type
 */
template <typename Func>
struct FuncBootProcess final : BootProcess
{
    explicit FuncBootProcess(const char* description, Func func): BootProcess(description), m_func(func) {}

private:
    void runProcess() override { m_func(); }
    Func m_func;
};


#endif //BOOT_PROCESS_HPP
