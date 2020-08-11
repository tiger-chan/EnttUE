#include "job_base.hpp"

namespace tc
{
job_base::job_base()
{
	job_.handle = this;
}

TArray<entt::id_type> job_base::dependencies() const
{
	TArray<entt::id_type> dep;

	dependencies_.GetKeys(dep);
	return std::move(dep);
}

void job_base::add_job_dependency(job_handle handle)
{
	dependencies_.Emplace(handle.id, std::move(handle));
}

} // namespace tc