
#ifndef SOASM_SOISV1_INSTR_SET_HPP
#define SOASM_SOISV1_INSTR_SET_HPP

#include <cstddef>
#include "../instr_set.hpp"
#include "regs.hpp"

namespace SOASM::SOISv1{
	using namespace InstrArgs;
	using namespace Regs;
	using instr_t = uint8_t;

	template<typename T,typename ...Args>
	struct Instr{
		using args = std::tuple<Args...>;
		struct InstrArg{
			using type = T;
			T instr;
			std::vector<instr_t> args;
			auto operator()(instr_t id){
				instr.id=id;
				args.insert(args.begin(),std::bit_cast<instr_t>(instr));
				return args;
			}
		};
		auto operator()(Args... args){
			InstrArg instr{*static_cast<T*>(this)};
			(std::ranges::move(args.get_bytes(), std::back_inserter(instr.args)),...);
			return instr;
		}
	};

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
		enum struct FN:instr_t{
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
		enum struct FN:instr_t{
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

	
	struct INTCall:Instr<INTCall>{
		static constexpr instr_t reserve_id=0xCD;
		static constexpr std::string_view name="INTCall";
		
		#define X_OPTS
		#include "../x_opts.inc"
	};
	
	struct Init:Instr<Init>{
		static constexpr instr_t reserve_id=0x00;
		static constexpr std::string_view name="Init";
		
		#define X_OPTS
		#include "../x_opts.inc"
	};
	
	struct Halt:Instr<Halt>{
		static constexpr instr_t reserve_id=0xFF;
		static constexpr std::string_view name="Halt";
		
		#define X_OPTS
		#include "../x_opts.inc"
	};

	struct Unknown:Instr<Unknown>{
		static constexpr std::string_view name="Unknown";
		
		#define X_OPTS
		#include "../x_opts.inc"
	};
	using InstrSet=InstrSet::InstrSet<
		instr_t,Unknown,
		Init,
		INTCall,
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
		Halt
	>;
} // SOASM::SOISv1

#endif //SOASM_SOISV1_INSTR_SET_HPP
