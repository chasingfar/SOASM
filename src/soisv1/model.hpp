
#ifndef SOASM_SOISV1_MODEL_HPP
#define SOASM_SOISV1_MODEL_HPP

#include <cstddef>
#include <memory>
#include <map>
#include <ranges>
#include "../memory.hpp"
#include "regs.hpp"

namespace SOASM::SOISv1{
	struct Context{
		static constexpr size_t mem_size=1uz<<16;
		Models::Memory<mem_size> mem;

		uint16_t sp=0xFFFF;
		uint16_t pc=0;
		bool CF=true;
		Regs::RegFile reg;

		template<typename Instr>
		void run(Instr);

		template<typename T>
		T imm(){
			T v=0;
			for(size_t i:std::views::iota(0uz,sizeof(T))){
				v|=mem.get(++pc)<<i;
			}
			return v;
		}
		template<typename T>
		T pop(){
			T v=0;
			for(size_t i:std::views::iota(0uz,sizeof(T))){
				v|=static_cast<T>(mem.get(++sp))<<i;
			}
			return v;
		}
		template<typename T>
		void push(T v){
			for(size_t i:std::views::iota(0uz,sizeof(T))|std::views::reverse){
				mem.set(sp--,(v>>i)&0xff);
			}
		}

	};
} // SOASM

#endif //SOASM_SOISV1_MODEL_HPP
