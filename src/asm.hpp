
#ifndef SOASM_ASM_HPP
#define SOASM_ASM_HPP

#include "types.hpp"
#include "util/overloaded.hpp"
#include <iostream>
#include <format>
#include <variant>

namespace SOASM{
	template<typename InstrSet>
	struct ASM{
		static auto disassemble(std::span<uint8_t> data){
			std::vector<std::tuple<size_t,std::span<uint8_t>,std::string>> ret;
			for (size_t pc = 0; pc < data.size(); ++pc) {
				std::visit([&]<typename T>(T instr){
					auto arg_raws=T::args_t::from_bytes(data.subspan(pc+InstrSet::raw::size));
					ret.emplace_back(pc,data.subspan(pc,T::size),std::format("{} {}",instr.to_string(),arg_raws));
					pc+=T::args_t::size;
				},InstrSet::get_instr(data.subspan(pc)));
			}
			return ret;
		}
	};
}
#endif //SOASM_ASM_HPP
