
#ifndef SOASM_SOISV1_REGS_HPP
#define SOASM_SOISV1_REGS_HPP

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
}

#endif //SOASM_SOISV1_REGS_HPP
