#include "model.hpp"

using namespace SOASM::SOISv1;

bool Context::run() {
	auto pc_old=pc;
	auto instr_data=mem.get_bytes<InstrSet::raw::size>(pc);
	std::visit([&]<typename T>(T instr_obj){
		if constexpr(0==T::args_t::num){
			run_instr(instr_obj);
		}else{
			auto arg_bytes=mem.get_bytes<T::args_t::size>(pc+1);
			auto arg_raws=T::args_t::from_bytes(arg_bytes);
			pc+=T::args_t::size;
			[&]<size_t ...I>(std::index_sequence<I...>){
				run_instr(instr_obj,std::get<I>(arg_raws)...);
			}(std::make_index_sequence<T::args_t::num>{});
		}
	},InstrSet::get_instr(instr_data));
	return pc_old!=pc;
}