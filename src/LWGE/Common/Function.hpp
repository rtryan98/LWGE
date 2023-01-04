#pragma once

#include <cstdlib>
#include <concepts>

namespace lwge
{
	template<class Functor>
	concept LambdaConcept = requires(Functor&& f)
	{
		&Functor::operator();
		{
			Functor{ std::move(f) }
		} -> std::same_as<Functor>;
	};

	template<std::size_t StackSize>
	class Function
	{
		static_assert(StackSize >= 8);
	private:
		struct Impl {};
		using PfnMember = void(Impl::*)() const;

	public:
		template<LambdaConcept L>
		Function(L&& lambda)
		{
			static_assert(sizeof(L) <= StackSize,
				"Overflow behaviour not yet implemented.");
			new (m_buf) L{ std::move(lambda) };
			m_func = reinterpret_cast<PfnMember>(&L::operator());
		}

		void operator()()
		{
			(reinterpret_cast<Impl*>(m_buf)->*m_func)();
		}

	private:
		PfnMember m_func;
		std::byte m_buf[StackSize];
	};
}
