#include "model.hpp"

using namespace SOASM::SOISv1;

template<typename T,size_t I>
using arg_raw=std::tuple_element_t<I,typename T::args>;
template<typename T,size_t I>
using arg_raw_t=arg_raw<T,I>::type;
template<typename T>
static constexpr size_t arg_raw_num=std::tuple_size_v<typename T::args>;

bool Context::run() {
	auto pc_old=pc;
	InstrSet::visit([&]<typename T>(T instr_obj){
		if constexpr(0==arg_raw_num<T>){
			run_instr(instr_obj);
		}else{
			[&]<size_t ...I>(std::index_sequence<I...>){
				std::array<std::variant<arg_raw_t<T,I>...>,arg_raw_num<T>> args{
					arg_raw_t<T,I>{0}...
				};
				((args[I]=imm<arg_raw<T,I>>()),...);
				run_instr(instr_obj,std::get<arg_raw_t<T,I>>(args[I])...);
			}(std::make_index_sequence<arg_raw_num<T>>{});
		}
	},mem.get(pc));
	return pc_old!=pc;
}