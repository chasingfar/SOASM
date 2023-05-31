
#ifndef SOASM_ASM_HPP
#define SOASM_ASM_HPP

#include "types.hpp"
#include "util/overloaded.hpp"
#include <iostream>
#include <format>
#include <variant>

namespace SOASM{

	template<typename T>
	concept CanToCode=requires (T x){x.to_code();};

	template<typename InstrSet>
	struct ASM{
		struct Code{
			using val_type=std::variant<uint8_t,Lazy,Label>;
			using arg_type=std::variant<uint8_t,Lazy,Label,bytes_t,data_t,Code>;
			std::vector<val_type> codes;
			void add(std::integral auto val){
				codes.emplace_back(static_cast<uint8_t>(val&0xffu));
			}
			template<typename T> requires std::same_as<T,Lazy> || std::same_as<T,Label>
			void add(const T& val){
				codes.emplace_back(val);
			}
			void add(std::ranges::range auto&& range){
				std::ranges::move(range|std::views::transform([](auto c){
					return std::visit([](auto v)->val_type{return v;},c);
				}), std::back_inserter(codes));
			}
			void add(const CanToCode auto& code){
				std::ranges::move(code.to_code().codes, std::back_inserter(codes));
			}
			template<typename Instr>
			void add(std::pair<Instr,data_t> code){
				auto [instr,data]=code;
				instr.template set_id<InstrSet>();
				add(instr.to_raw().may_lazys());
				add(data);
			}
			template<typename ...Ts>
			Code(Ts... code){
				(add(code),...);
			}
			auto begin(){return codes.begin();}
			auto end(){return codes.end();}
			auto begin() const{return codes.begin();}
			auto end() const{return codes.end();}

			static size_t count(const val_type& code){
				if(std::get_if<Label>(&code)){
					return 0;
				}
				return 1;
			}
			size_t size() const{
				size_t sum=0;
				for (const auto &code:codes) {
					sum+=count(code);
				}
				return sum;
			}

			data_t resolve(size_t start=0,uint8_t padding=0xff) const{
				data_t data{};
				data.reserve(size());
				for (const auto &code:codes) {
					std::visit(Util::overloaded{
						[&](const auto&  fn)    { data.emplace_back(fn); },
						[&](const Label& label) {
							if(auto addr=label.get();addr){
								data.resize(*addr-start,padding);
							}else{
								label.set(start+data.size());
							}
						},
					}, code);
				}
				return data;
			}
			static bytes_t assemble(data_t data){
				bytes_t bytes;
				bytes.reserve(data.size());
				for (auto &code:data) {
					bytes.emplace_back(std::visit(Util::overloaded{
						[&](const Lazy &lazy) { return lazy(bytes.size()); },
						[&](uint8_t v) { return v; },
					}, code));
				}
				return bytes;
			}
			bytes_t assemble(size_t start=0) const{
				return assemble(resolve(start));
			}
			static auto disassemble(std::span<uint8_t> data){
				std::vector<std::tuple<size_t,std::span<uint8_t>,std::string>> ret;
				for (size_t pc = 0; pc < data.size(); ++pc) {
					std::visit([&]<typename T>(T instr){
						auto arg_raws=T::args_t::from_bytes(data.subspan(pc+InstrSet::raw::size));
						ret.emplace_back(pc,data.subspan(pc,T::size),std::format("{} {}",instr.to_string(),arg_raws));
						pc+=T::args_t::size;
					},InstrSet::get_instr(data.subspan(pc)));//data[pc]
				}
				return ret;
			}
		};
	};
}
#endif //SOASM_ASM_HPP
