
#ifndef SOASM_INSTR_SET_UTIL_HPP
#define SOASM_INSTR_SET_UTIL_HPP

#include <cstddef>
#include <bit>
#include <algorithm>
#include <functional>
#include <magic_enum.hpp>
#include <ranges>

namespace SOASM::InstrSetUtil{
	template<typename T> requires requires(T v){{std::to_string(v)};}
	std::string opt_string(T opt){
		return std::to_string(opt);
	}
	template<typename T> requires std::is_enum_v<T>
	std::string opt_string(T opt){
		return std::string(magic_enum::enum_name(opt));
	}
	template<typename T>
	static constexpr size_t opt_width(){
		if constexpr (std::is_enum_v<T>){
			constexpr size_t N = magic_enum::enum_count<T>();
			return std::bit_width(N>1?N-1:N);
		}else{
			return sizeof(T)*CHAR_BIT;
		}
	}

	template<typename T>
	static constexpr size_t instr_count(){
		return 1uz<<T::optw;
	}

	template<typename T>
	static constexpr size_t align_id(size_t id){
		constexpr size_t mask=instr_count<T>()-1;
		if(id&mask){
			return (id|mask)+1;
		}
		return id;
	}

	template <typename T>
	concept has_reserved_id = requires(T) {{T::reserve_id};};

	template<typename Unknown,typename ...T>
	struct InstrSet{
		using raw=Unknown::raw;
		using raw_t=raw::type;
		using instr_ts=std::variant<Unknown,T...>;
		static constexpr auto get_reserved_ids(){
			std::vector<std::pair<size_t,size_t>> reserved_ids;
			([&]<typename V>(V){
				if constexpr(has_reserved_id<V>){
					reserved_ids.emplace_back(V::reserve_id,V::reserve_id+instr_count<V>());
				}
			}(T{}),...);
			std::sort(reserved_ids.begin(), reserved_ids.end(), [](auto x, auto y){
				return x.first<y.first;
			});
			return reserved_ids;
		}

		template<typename U>
		static constexpr size_t skip_reserved_id(size_t id){
			auto reserved_ids=get_reserved_ids();
			auto id_max=id+instr_count<U>()-1;
			for(auto [s,e]:reserved_ids){
				if((s<=id && id<e)||(s<=id_max && id_max<e)) {
					id = align_id<U>(e);
				}
			}
			return id;
		}

		template<typename A,typename B,typename ...U>
		static constexpr size_t get_id_impl(size_t S){
			S=skip_reserved_id<B>(align_id<B>(S));
			if constexpr(std::is_same_v<A,B>){
				return S;
			}else if constexpr(has_reserved_id<B>){
				return get_id_impl<A,U...>(S);
			}else{
				return get_id_impl<A,U...>(S+instr_count<B>());
			}
		}
		template<typename U>
		static size_t get_id(){
			if constexpr(has_reserved_id<U>){
				return U::reserve_id;
			}
			return get_id_impl<U,T...>(0)>>U::optw;
		}
		template<typename U>
		static auto to_instr(raw_t instr){
			return std::bit_cast<U>(instr);
		}
		template<typename U>
		static bool is_instr(raw_t instr){
			return get_id<U>()==to_instr<U>(instr).id;
		}
		template<typename V,typename ...U>
		static instr_ts get_instr_impl(raw_t instr){
			if(is_instr<V>(instr)){
				return to_instr<V>(instr);
			}
			if constexpr(sizeof...(U)>0){
				return get_instr_impl<U...>(instr);
			}else{
				return to_instr<Unknown>(instr);
			}
		}
		static instr_ts get_instr(raw_t data){
			return get_instr_impl<T...>(data);
		}
		static instr_ts get_instr(std::span<uint8_t> data){
			return get_instr(raw::from_bytes(data));
		}
		static auto list_instr(){
			return std::vector{
				std::tuple{T::name,get_id<T>(),T::optw}...
			};
		}
	};
} // SOASM::InstrSetUtil
#endif //SOASM_INSTR_SET_UTIL_HPP
