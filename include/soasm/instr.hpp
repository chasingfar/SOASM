
#ifndef SOASM_INSTR_HPP
#define SOASM_INSTR_HPP

#include "types.hpp"

namespace SOASM{

	template<typename ...Args>
	struct InstrArgs{
		static constexpr size_t num = sizeof...(Args);
		static constexpr size_t size = (0 + ... + Args::size);
		template<size_t I>
		using raw=std::tuple_element_t<I,std::tuple<Args...>>;
		template<typename T,size_t I>
		using raw_t=raw<I>::type;

		using raws_t=std::tuple<typename Args::type...>;

		static raws_t from_bytes(std::span<uint8_t> data){
			if constexpr(num==0){
				return {};
			}else{
				return [&]<size_t ...I>(std::index_sequence<I...>)->raws_t{
					std::array<size_t,num+1> offsets{0};
					((offsets[I+1]=offsets[I]+raw<I>::size),...);
					return {Args::from_bytes(data.subspan(offsets[I]))...};
				}(std::make_index_sequence<num>{});
			}
		}
	};

	static inline Code instr_to_code(auto instr,const data_t& arg);

	template<typename Raw,typename Instr,typename ...Args>
	struct InstrBase{
		using raw = Raw;
		using raw_t = Raw::type;
		using args_t = InstrArgs<Args...>;
		static constexpr size_t size = Raw::size + args_t::size;
		struct Record{
			Instr instr;
			args_t::raws_t args;
		};

		template<typename InstrSet>
		Instr set_id(){
			Instr instr=*static_cast<Instr*>(this);
			instr.id=InstrSet::template get_id<Instr>();
			return instr;
		}
		raw_t to_raw() const{
			return std::bit_cast<raw_t>(*this);
		}
		static Instr from_bytes(std::span<uint8_t,raw::size> data){
			return std::bit_cast<Instr>(static_cast<raw_t>(Raw::from_bytes(data)));
		}
		Code operator()(Args... args){
			Instr instr=*static_cast<Instr*>(this);
			data_t data{};
			(std::ranges::move(args.may_lazys(), std::back_inserter(data)),...);
			return instr_to_code(instr,data);
		}
	};

};
#endif //SOASM_INSTR_HPP
