
#ifndef SOASM_ASM_HPP
#define SOASM_ASM_HPP

#include "instr.hpp"
#include <iostream>
#include <format>
#include <variant>

namespace SOASM{
	template<typename InstrSet>
	static auto disassemble(std::span<uint8_t> data,size_t start_addr=0){
		std::vector<std::tuple<size_t,std::span<uint8_t>,std::string>> ret;
		for (size_t pc = 0; pc < data.size(); ++pc) {
			std::visit([&]<typename T>(T instr){
				auto arg_raws=T::args_t::from_bytes(data.subspan(pc+InstrSet::raw::size));
				ret.emplace_back(start_addr+pc,data.subspan(pc,T::size),std::format("{} {}",instr.to_string(),arg_raws));
				pc+=T::args_t::size;
			},InstrSet::get_instr(data.subspan(pc)));
		}
		return ret;
	}
	struct CodeBlock{
		Label start;
		Code body{};
		Label end;

		Code to_code() const{
			return {start,body,end};
		}
	};
	struct DataBlock{
		Label start;
		data_t body{};
		Label end;

		Code to_code() const{
			return {start,body,end};
		}
	};
}
#endif //SOASM_ASM_HPP
