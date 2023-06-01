
#ifndef SOASM_SOISV1_INSTR_SET_HPP
#define SOASM_SOISV1_INSTR_SET_HPP

#include <cstddef>
#include "../types.hpp"
#include "../instr_set_util.hpp"
#include "regs.hpp"

namespace SOASM::SOISv1{
	using namespace RawTypes;
	using namespace Regs;

	template<typename T,typename ...Args>
	using Instr=InstrBase<u8,T,Args...>;

	struct LoadFar:Instr<LoadFar,LE::i16>{//load value(8) from address(16) with offset(16) and push to stack
		static constexpr std::string_view name="LoadFar";
		
		#define X_OPTS X(Reg16,from)
		#include "../x_opts.inc"
    };
	struct LoadNear:Instr<LoadNear,i8>{//load value(8) from address(16) with offset(8) and push to stack
		static constexpr std::string_view name="LoadNear";
		
		#define X_OPTS X(Reg16,from)
		#include "../x_opts.inc"
	};
	struct Load:Instr<Load>{//load value(8) from address(16) and push to stack
		static constexpr std::string_view name="Load";
		
		#define X_OPTS X(Reg16,from)
		#include "../x_opts.inc"
	};
	
	struct SaveFar:Instr<SaveFar,LE::i16>{//pop value(8) from stack and save to address(16) with offset(16)
		static constexpr std::string_view name="SaveFar";
		
		#define X_OPTS X(Reg16,to)
		#include "../x_opts.inc"
	};
	struct SaveNear:Instr<SaveNear,i8>{//pop value(8) from stack and save to address(16) with offset(8)
		static constexpr std::string_view name="SaveNear";
		
		#define X_OPTS X(Reg16,to)
		#include "../x_opts.inc"
	};
	struct Save:Instr<Save>{//pop value(8) from stack and save to address(16)
		static constexpr std::string_view name="Save";
		
		#define X_OPTS X(Reg16,to)
		#include "../x_opts.inc"
	};
	struct SaveImm:Instr<SaveImm,u8>{//save immediate value(8) to address(16)
		static constexpr std::string_view name="SaveImm";
		
		#define X_OPTS X(Reg16,to)
		#include "../x_opts.inc"
	};

	struct ImmVal:Instr<ImmVal,u8>{//push immediate value to stack
		static constexpr std::string_view name="ImmVal";
		
		#define X_OPTS
		#include "../x_opts.inc"
	};
	struct Calc:Instr<Calc>{
		static constexpr std::string_view name="Calc";
		enum struct FN:raw_t{
			SHL, SHR,
			RCL, RCR,
			ADD, SUB,
			ADC, SUC,
		};
		
		#define X_OPTS X(FN,fn)
		#include "../x_opts.inc"
	};
	struct Logic:Instr<Logic>{
		static constexpr std::string_view name="Logic";
		enum struct FN:raw_t{
			NOT,
			AND,
			OR ,
			XOR,
		};
		
		#define X_OPTS X(FN,fn)
		#include "../x_opts.inc"
	};
	
	struct Push:Instr<Push>{
		static constexpr std::string_view name="Push";
		
		#define X_OPTS X(Reg,from)
		#include "../x_opts.inc"
	};
	struct Pop:Instr<Pop>{
		static constexpr std::string_view name="Pop";
		
		#define X_OPTS X(Reg,to)
		#include "../x_opts.inc"
	};
	
	struct BranchZero:Instr<BranchZero,LE::u16>{
		static constexpr std::string_view name="BranchZero";
		
		#define X_OPTS
		#include "../x_opts.inc"
	};
	
	struct BranchCF:Instr<BranchCF,LE::u16>{
		static constexpr std::string_view name="BranchCF";
		
		#define X_OPTS
		#include "../x_opts.inc"
	};
	
	struct Jump:Instr<Jump,LE::u16>{
		static constexpr std::string_view name="Jump";
		
		#define X_OPTS
		#include "../x_opts.inc"
	};

	struct Call:Instr<Call,LE::u16>{
		static constexpr std::string_view name="Call";
		
		#define X_OPTS
		#include "../x_opts.inc"
	};
	
	struct CallPtr:Instr<CallPtr>{
		static constexpr std::string_view name="CallPtr";
		
		#define X_OPTS
		#include "../x_opts.inc"
	};
	
	struct Return:Instr<Return>{
		static constexpr std::string_view name="Return";
		
		#define X_OPTS
		#include "../x_opts.inc"
	};
	
	struct Enter:Instr<Enter>{
		static constexpr std::string_view name="Enter";
		
		#define X_OPTS X(Reg16,bp)
		#include "../x_opts.inc"
	};
	
	struct Adjust:Instr<Adjust,LE::i16>{
		static constexpr std::string_view name="Adjust";
		
		#define X_OPTS
		#include "../x_opts.inc"
	};
	
	struct Leave:Instr<Leave>{
		static constexpr std::string_view name="Leave";
		
		#define X_OPTS X(Reg16,bp)
		#include "../x_opts.inc"
	};

	struct NOP:Instr<NOP>{
		static constexpr std::string_view name="NOP";
		
		#define X_OPTS
		#include "../x_opts.inc"
	};

	struct Reset:Instr<Reset>{
		static constexpr raw_t reserve_id=0x00;
		static constexpr std::string_view name="Reset";
		
		enum struct Val:raw_t{
			RST0,RST1,RST2,RST3,
			RST4,RST5,RST6,RST7,
		};

		#define X_OPTS X(Val,val)
		#include "../x_opts.inc"
	};
	
	struct Halt:Instr<Halt>{
		static constexpr raw_t reserve_id=0xFF;
		static constexpr std::string_view name="Halt";
		
		#define X_OPTS
		#include "../x_opts.inc"
	};

	struct Unknown:Instr<Unknown>{
		static constexpr std::string_view name="Unknown";
		
		#define X_OPTS
		#include "../x_opts.inc"
	};
	using InstrSet=InstrSetUtil::InstrSet<
		Unknown,
		Reset,
		LoadFar,SaveFar,
		LoadNear,SaveNear,
		Load,Save,
		SaveImm,
		Push,Pop,
		Calc,Logic,
		BranchCF,BranchZero,
		ImmVal,
		Jump,Call,Return,
		Adjust,Enter,Leave,CallPtr,
		NOP,
		Halt
	>;
} // SOASM::SOISv1
namespace SOASM{
	static inline Code instr_to_code(auto instr,const data_t& arg){
		return {instr.template set_id<SOISv1::InstrSet>().to_raw(),arg};
	}
}
#endif //SOASM_SOISV1_INSTR_SET_HPP
