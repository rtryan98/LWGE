#pragma once

#include "LWGE/Common/Function.hpp"

#include <atomic>
#include <concurrentqueue.h>
#include <thread>
#include <vector>

namespace lwge::thread
{
	using Func = Function<80>;

#pragma warning(push)
#pragma warning(disable : 4324) // Structure was padded due to alignment specifier
	struct alignas(std::hardware_destructive_interference_size) Job
	{
		Job(Func&& fn, std::atomic<uint64_t>* cnt, Job* prnt)
			: children(1), parent(prnt), counter(cnt), func(fn)
		{
			if (parent)
			{
				parent->children.fetch_add(1);
			}
		}

		std::atomic<uint64_t> children;
		Job* parent;
		std::atomic<uint64_t>* counter;
		Func func;
	};
#pragma warning(pop)

	class JobSystem
	{
	public:
		JobSystem(uint32_t min_thread_count);

		void stop();

		static Job* get_current_job() { return tl_current_job; }
		static uint32_t get_worker_thread_cnt() { return s_thread_idx_counter; }
		static uint32_t get_thread_idx() { return tl_thread_idx; }

		/// @brief Schedules a function.
		/// @details Pushes a job with the given function into this
		/// instance's work queue.
		void schedule(Func&& func);

		/// @brief Schedules a function.
		/// @details If called from within another function inside the
		/// job system, this will add the newly scheduled function as
		/// a child of the currently running job, otherwise works
		/// equally like `schedule`.
		void schedule_child(Func&& func);

		/// @brief Schedules a function and awaits *only* this function.
		/// @details If it's desired to await spawned children,
		/// `await_children()` should be called at the end of `func`.
		void schedule_and_await(Func&& func);

		/// @brief Await the spawned children when inside a job.
		/// @details This function only works when already inside a job.
		/// Otherwise it will be a no-op. This function will poll for
		/// other jobs and run those until all children have been executed.
		void await_children();

		/// @brief Await until a provided counter hits the given value.
		/// @details Similarly to `await_children`, this function will keep
		/// polling for new jobs, although the main difference being that
		/// the polling will stop as soon as `counter->load()` hits `val`.
		void await_counter(std::atomic<uint64_t>* counter, uint64_t val);

	private:
		void schedule(Job* job);

		Job* allocate_job(Func&& fn, std::atomic<uint64_t>* counter, Job* parent);
		void free_job(Job* job);
		Job* try_fetch_next_job();

		void finish_job(Job* job);
		void finalize_job(Job* job);

	private:
		constexpr static int64_t YIELD_TIMING = 100; // 0.01ms
		constexpr static uint64_t SPINS_UNTIL_YIELD = 0x100;

		std::atomic<uint64_t> m_running = 1;
		std::vector<std::jthread> m_workers;
		moodycamel::ConcurrentQueue<Job*> m_global_job_queue;

		static std::atomic<uint32_t> s_thread_idx_counter;
		thread_local static uint32_t tl_thread_idx;
		thread_local static Job* tl_current_job;
	};
}
