
#ifndef SOASM_INSTR_SET_HPP
#define SOASM_INSTR_SET_HPP

#include <cstddef>
#include <bit>
#include <algorithm>
#include <functional>
#include <magic_enum.hpp>
#include <ranges>

namespace SOASM{
	namespace InstrArgs{
		template<bool IsBE,bool IsSigned,size_t Size>
		struct Int{
			static constexpr bool is_big_endian=IsBE;
			static constexpr bool is_signed=IsSigned;
			static constexpr size_t size=Size;

			unsigned long long val;
			Int(std::integral auto val):val(val){}
			auto get_bytes() const{
				using namespace std::views;
				std::array<uint8_t,Size> arr;
				for(auto&& [a,offset]:zip(arr,iota(0uz,Size)|transform([](size_t i){return 8*(IsBE?Size-i-1:i);}))){
					a=(val>>offset)&0xff;
				}
				return arr;
			}
		};

		using i8 =Int<false,true ,1>;
		using u8 =Int<false,false,1>;
		namespace LE{
			using i16 = Int<false,true ,2>;
			using i32 = Int<false,true ,4>;
			using i64 = Int<false,true ,8>;
			using u16 = Int<false,false,2>;
			using u32 = Int<false,false,4>;
			using u64 = Int<false,false,8>;
		} // LE
		namespace BE{
			using i16 = Int<true,true ,2>;
			using i32 = Int<true,true ,4>;
			using i64 = Int<true,true ,8>;
			using u16 = Int<true,false,2>;
			using u32 = Int<true,false,4>;
			using u64 = Int<true,false,8>;
		} // BE
	} // InstrArgs
	namespace InstrSet{
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

		template<typename instr_t,typename Unknown,typename ...T>
		struct InstrSet{
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
			static auto to_instr(instr_t instr){
				return std::bit_cast<U>(instr);
			}
			template<typename U>
			static bool is_instr(instr_t instr){
				return get_id<U>()==to_instr<U>(instr).id;
			}

			static auto gen(auto cfg){
				bool is_found=([&cfg]<typename V>(V){
					if(is_instr<V>(cfg.arg.instr)){
						cfg.gen(to_instr<V>(cfg.arg.instr));
						return true;
					}
					return false;
				}(T{})||...);
				if(!is_found){
					cfg.gen(to_instr<Unknown>(cfg.arg.instr));
				}
				return cfg;
			}
			
			template<instr_t instr>
			using get_instr=decltype(std::tuple_cat(
				std::conditional_t<is_instr<T>(instr), 
					std::tuple<T>, std::tuple<>
				>{}...));
			
			template<typename F,typename V,typename ...U>
			static auto visit_impl(F&& vis,instr_t instr){
				if(is_instr<V>(instr)){
					return vis(to_instr<V>(instr));
				}
				if constexpr(sizeof...(U)>0){
					return visit_impl<F,U...>(std::forward<F>(vis),instr);
				}else{
					return vis(to_instr<Unknown>(instr));
				}
			}
			template<typename F>
			static auto visit(F&& vis,instr_t instr){
				return visit_impl<F,T...>(std::forward<F>(vis),instr);
			}
			static auto list_instr(){
				return std::vector{
					std::tuple{T::name,get_id<T>(),T::optw}...
				};
			}
			
			struct Instrs{
				std::vector<instr_t> data;
				template<typename ...U>
				Instrs(U... instr){
					(std::ranges::move(instr(get_id<typename U::type>()), std::back_inserter(data)),...);
				}
				auto begin(){return data.begin();}
				auto end(){return data.end();}
				auto begin() const{return data.begin();}
				auto end() const{return data.end();}
			};
		};

	} // InstrSet
} // SOASM
namespace SOASM::InstrSet{




} // SOASM::InstrSet

#endif //SOASM_INSTR_SET_HPP
