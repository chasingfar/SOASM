
#ifndef SOASM_MEMORY_HPP
#define SOASM_MEMORY_HPP

#include <cstddef>
#include <memory>
#include <map>
#include "util/accessors_proxy.hpp"

namespace SOASM::Models{
	template<size_t Size>
	struct Memory:Util::AccessorsProxy<Memory<Size>>{
		const std::array<uint8_t,Size>& base;
		std::map<size_t,uint8_t> overlay{};
		Memory(const std::array<uint8_t,Size>& mem):base{mem}{}
		uint8_t get(size_t addr) const{
			auto it=overlay.find(addr);
			if(it==overlay.end()){
				return base[addr];
			}
			return it->second;
		}
		void set(size_t addr,uint8_t v){
			overlay[addr]=v;
		}
	};
} // SOASM

#endif //SOASM_MEMORY_HPP
