#pragma once

namespace tc
{
template <typename Requirements, typename System, typename Type, typename... Args>
struct job_requirements_builder {
	job_requirements_builder(System *system, TSharedPtr<Type> &&task)
		: system_{ system }, task_{ task }
	{
	}

	template <typename... Reads> Requirements &add_read() noexcept
	{
		assert(task_.IsValid());
		task_data_access &data = task_->data;
		(data.add<Reads>(access_t::read), ...);
		return *static_cast<Requirements *>(this);
	}

	template <typename... Writes> Requirements &add_write() noexcept
	{
		assert(task_.IsValid());
		task_data_access &data = task_->data;
		(data.add<Writes>(access_t::write), ...);
		return *static_cast<Requirements *>(this);
	}

	template <typename Func>
	void schedule(Func func,
		      EAsyncExecution execution_method = EAsyncExecution::TaskGraph) noexcept
	{
		assert(task.IsValid());
		static_cast<Requirements *>(this)->set_work(*task_, std::forward<Func>(func));

		task_->execution_method = execution_method;
		system_->add_task(task_);
	}

	template <typename Func>
	void
	schedule_parallel(Func func,
			  EAsyncExecution execution_method = EAsyncExecution::TaskGraph) noexcept
	{
		assert(task.IsValid());
		static_cast<Requirements *>(this)->set_work(*task_, std::forward<Func>(func));

		task_->execution_method = execution_method;
		task_->can_parallelize = true;
		system_->add_task(task_);
	}

    private:
	System *system_;
	TSharedPtr<Type> task_;
};

} // namespace tc
