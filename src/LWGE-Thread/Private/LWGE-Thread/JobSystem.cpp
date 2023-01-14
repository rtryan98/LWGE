#include "LWGE-Thread/JobSystem.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

namespace lwge::thread
{
    void nano100_sleep(int64_t nano100s)
    {
        HANDLE waitable_timer = CreateWaitableTimerEx(NULL, NULL,
            CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
        if (!waitable_timer)
        {
            YieldProcessor();
            return;
        }
        LARGE_INTEGER time = {
            .QuadPart = -nano100s
        };
        if (!SetWaitableTimerEx(waitable_timer, &time, 0, NULL, NULL, NULL, 0))
        {
            CloseHandle(waitable_timer);
            YieldProcessor();
            return;
        }
        if (WaitForSingleObject(waitable_timer, 1) != WAIT_OBJECT_0)
        {
            CloseHandle(waitable_timer);
            YieldProcessor();
            return;
        }
        CloseHandle(waitable_timer);
    }

    std::atomic<uint32_t> JobSystem::s_thread_idx_counter = 0;
    thread_local uint32_t JobSystem::tl_thread_idx = 0;
    thread_local Job* JobSystem::tl_current_job = nullptr;

    JobSystem::JobSystem(uint32_t min_thread_count)
    {
         uint32_t thread_count =
             std::max(min_thread_count, std::thread::hardware_concurrency() - 1);
         m_workers.reserve(thread_count);
         for (uint32_t i = 0; i < thread_count; i++)
         {
             m_workers.push_back(std::jthread([this]() {
                 tl_current_job = nullptr;
                 tl_thread_idx = s_thread_idx_counter.fetch_add(1) + 1;
                 await_counter(&m_running, 0);
                 }));
         }
    }

    void JobSystem::stop()
    {
        m_running.store(0);
    }

    void JobSystem::schedule(Func&& func)
    {
        Job* job = allocate_job(std::forward<Func>(func), nullptr, nullptr);
        schedule(job);
    }

    void JobSystem::schedule_child(Func&& func)
    {
        Job* job = allocate_job(std::forward<Func>(func), nullptr, get_current_job());
        schedule(job);
    }

    void JobSystem::schedule_and_await(Func&& func)
    {
        std::atomic<uint64_t> counter = 1;
        Job* job = allocate_job(std::forward<Func>(func), &counter, nullptr);
        schedule(job);
        await_counter(&counter, 0);
    }

    void JobSystem::await_children()
    {
        Job* job = get_current_job();
        if (job)
        {
            await_counter(&job->children, 1);
        }
    }

    void JobSystem::schedule(Job* job)
    {
        m_global_job_queue.enqueue(job);
    }

    void JobSystem::await_counter(std::atomic<uint64_t>* counter, uint64_t val)
    {
        Job* last_job = get_current_job();

        uint32_t spin_count = 0;
        while (counter->load() != val)
        {
            if (spin_count >= JobSystem::SPINS_UNTIL_YIELD)
            {
                spin_count = 0;
                nano100_sleep(JobSystem::YIELD_TIMING);
            }

            Job* current_job = try_fetch_next_job();
            if (current_job != nullptr)
            {
                tl_current_job = current_job;
                current_job->func();
                finish_job(current_job);
                spin_count = 0;
            }
            else
            {
                ++spin_count;
            }
        }

        tl_current_job = last_job;
    }

    // TODO: arena allocator for jobs.
    Job* JobSystem::allocate_job(Func&& fn, std::atomic<uint64_t>* counter, Job* parent)
    {
        return new Job(std::forward<Func>(fn), counter, parent);
    }

    // TODO: arena allocator for jobs.
    void JobSystem::free_job(Job* job)
    {
        delete job;
    }

    Job* JobSystem::try_fetch_next_job()
    {
        Job* job = nullptr;
        m_global_job_queue.try_dequeue(job);
        return job;
    }

    void JobSystem::finish_job(Job* job)
    {
        if (job->counter)
        {
            job->counter->fetch_sub(1);
        }
        finalize_job(job);
    }

    void JobSystem::finalize_job(Job* job)
    {
        auto child_count = job->children.fetch_sub(1);
        if (child_count == 1)
        {
            if (job->parent != nullptr)
            {
                finalize_job(job->parent);
            }
            free_job(job);
        }
    }
}
