#include "fwd.hpp"
#include "job_processor.hpp"
#include "sync_job_processor.hpp"
#include "async_job_processor.hpp"

namespace tc
{
TUniquePtr<job_processor> make_processor(bool run_parallel)
{
	if (FPlatformProcess::SupportsMultithreading() && run_parallel) {
		return MakeUnique<async_job_processor>();
	}

	return MakeUnique<sync_job_processor>();
}
} // namespace tc
