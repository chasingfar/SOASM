
#ifndef SOASM_SOISV1_REGS_HPP
#define SOASM_SOISV1_REGS_HPP

#include <utility>
#include <cstdint>

namespace SOASM::SOISv1::Regs{
	enum struct Reg:uint8_t{
		A  ,B  ,
		C  ,D  ,
		E  ,F  ,
		L  ,H  ,
	};
	enum struct Reg16:uint8_t{
		BA,
		DC,
		FE,
		HL,
	};
	
	inline auto toL(Reg16 reg16){
		return static_cast<Reg>((std::to_underlying(reg16)<<1)+0);
	}
	inline auto toH(Reg16 reg16){
		return static_cast<Reg>((std::to_underlying(reg16)<<1)+1);
	}


	struct RegFile{
		uint8_t regs[8];
		uint8_t get(Reg reg) const{
			return regs[std::to_underlying(reg)];
		}
		void set(Reg reg,uint8_t v){
			regs[std::to_underlying(reg)]=v;
		}
		uint16_t get(Reg16 reg16) const{
			return static_cast<uint16_t>(get(toH(reg16))<<8)|get(toL(reg16));
		}
		void set(Reg16 reg16,uint16_t v){
			set(toL(reg16),v&0xff);
			set(toH(reg16),(v>>8)&0xff);
		}
	};
}

#endif //SOASM_SOISV1_REGS_HPP
