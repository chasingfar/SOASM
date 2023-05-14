#include <soasm/soisv1.hpp>
#include "src/asm.hpp"
#include "src/soisv1/model.hpp"
#include <iostream>

using namespace SOASM;

int main(){
    using namespace SOISv1;
    std::array<uint8_t,1<<16> mem;
    Label::tbl_t LT;
	ASM<SOISv1::InstrSet>::Code program{
		ImmVal{}(3),
		Pop{.to=Reg::A}(),
		ImmVal{}(0),
		Pop{.to=Reg::B}(),
		LT["start"],
		Push{.from=Reg::A}(),
		BranchZero{}(LT["end"]),

		Push{.from=Reg::A}(),
		Push{.from=Reg::B}(),
		Calc{.fn=Calc::FN::ADD}(),
		Pop{.to=Reg::B}(),

		Push{.from=Reg::A}(),
		ImmVal{}(1),
		Calc{.fn=Calc::FN::SUB}(),
		Pop{.to=Reg::A}(),

		Jump{}(LT["start"]),
		LT["end"],
		Halt{}(),
	};
    std::ranges::move(program.assemble(),mem.begin());

    Context ctx{mem};
	for(int i=0;i<50;i++){
		if(!ctx.run()){
        	std::cout<<"halt"<<std::endl;
			break;
		}
        std::cout<<i<<":";
        for(auto v:ctx.reg.regs){
            std::cout<<" "<<(int)v;
        }
        std::cout<<std::endl;
	}
    return 0;
}