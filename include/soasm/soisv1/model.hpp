
#ifndef SOASM_SOISV1_MODEL_HPP
#define SOASM_SOISV1_MODEL_HPP

#include <cstddef>
#include <memory>
#include <map>
#include <ranges>
#include "soasm/models/memory.hpp"
#include "instr_set.hpp"

namespace SOASM::SOISv1{
	struct Context{
		static constexpr size_t mem_size=1uz<<16;
		Models::Memory<mem_size> mem;

		uint16_t sp=0;
		uint16_t pc=0;
		bool CF=true;
		Regs::RegFile reg;

		template<typename Instr,typename ...Args>
		void run_instr(Instr,Args...);
		bool run();

		template<typename T>
		T::type imm(){
			std::array<uint8_t,T::size> data;
			for(auto&& d:data){
				d=mem.get(++pc);
			}
			return T::from_bytes(data);
		}
		template<typename T>
		T::type pop(){
			std::array<uint8_t,T::size> data;
			for(auto&& d:data){
				d=mem.get(sp++);
			}
			return T::from_bytes(data);
		}
		template<typename T>
		void push(T::type v){
			for(auto d:T::to_bytes(v)|std::views::reverse){
				mem.set(--sp,d);
			}
		}

	};
} // SOASM

#endif //SOASM_SOISV1_MODEL_HPP
