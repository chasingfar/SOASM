# SOASM
Simple but Okay Assembler

### Define a instruction
 * opts is a apart of instruction will affect CPU function
 * args is outside of instruction such as immediate value or function address
```cpp
struct LoadFar:Instr<LoadFar,LE::i16>{
	//static constexpr raw_t reserve_id=0xCD;// optional reserve some special id 
	static constexpr std::string_view name="LoadFar";
		
	#define X_OPTS X(Reg16,from)
	#include "../x_opts.inc"
};
```
### Define a instruction set
InstrSet will determine instruction's id automatically. 
```cpp
using InstrSet=InstrSet::InstrSet<
		u8::type,//raw type of instruction
		Unknown,//default instruction
		LoadFar,SaveFar,...//instruction list
		>;
```
### Write assembly
```cpp
using namespace SOASM::SOISv1;
SOASM::Label::tbl_t LT;//Label table
SOASM::ASM<InstrSet>::Code program{
	//InstrName{opts...}(args...)
	ImmVal{}(3),
	Pop{.to=Reg::A}(),
	ImmVal{}(0),
	Pop{.to=Reg::B}(),
	LT["start"],//define new Label
	Push{.from=Reg::A}(),
	BranchZero{}(LT["end"]),//read Label as argument

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
std::vector<uint8_t> out=program.assemble();
```
