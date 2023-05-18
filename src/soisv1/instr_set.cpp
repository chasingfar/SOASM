#include "model.hpp"
#include "instr_set.hpp"

using namespace SOASM::SOISv1;
using namespace LE;

template<> void Context::run_instr(Unknown instr) {
	pc++;
}
template<> void Context::run_instr(Init instr) {
	sp=0xFFFF;
	pc=0;
}
template<> void Context::run_instr(LoadFar instr,int16_t offset){
	push<u8>(mem[reg[instr.from]+offset]);
	pc++;
}
template<> void Context::run_instr(SaveFar instr,int16_t offset) {
	mem[reg[instr.to]+offset]=pop<u8>();
	pc++;
}
template<> void Context::run_instr(LoadNear instr,int8_t offset) {
	push<u8>(mem[reg[instr.from]+offset]);
	pc++;
}
template<> void Context::run_instr(SaveNear instr,int8_t offset) {
	mem[reg[instr.to]+offset]=pop<u8>();
	pc++;
}
template<> void Context::run_instr(Load instr) {
	push<u8>(mem[reg[instr.from]]);
	pc++;
}
template<> void Context::run_instr(Save instr) {
	mem[reg[instr.to]]=pop<u8>();
	pc++;
}
template<> void Context::run_instr(SaveImm instr,uint8_t val) {
	mem[reg[instr.to]]=val;
	pc++;
}
template<> void Context::run_instr(Push instr) {
	push<u8>(reg[instr.from]);
	pc++;
}
template<> void Context::run_instr(Pop instr) {
	reg[instr.to]=pop<u8>();
	pc++;
}

inline std::pair<uint8_t,bool> shift_left(uint8_t v,bool carry=false){
	return {(v<<1)|(carry?1:0),(v&0x80)!=0};
}
inline std::pair<uint8_t,bool> shift_right(uint8_t v,bool carry=false){
	return {(v>>1)|(carry?0x80:0),(v&1)!=0};
}
inline std::pair<uint8_t,bool> add(uint8_t l,uint8_t r,bool carry=false){
	auto res=(carry?1u:0u)+l+r;
	return {res,(res&0x100)!=0};
}
inline std::pair<uint8_t,bool> sub(uint8_t l,uint8_t r,bool carry=true){
	return add(l,~r,carry);
}

template<> void Context::run_instr(Calc instr) {
	uint8_t lhs,rhs,val;
#define ARG_1 rhs=pop<u8>();
#define ARG_2 ARG_1 lhs=pop<u8>();
#define CALC_1(fn,name)  case Calc::FN::fn: ARG_1 std::tie(val,CF)=name(rhs);break;
#define CALC_1C(fn,name) case Calc::FN::fn: ARG_1 std::tie(val,CF)=name(rhs,CF);break;
#define CALC_2(fn,name)  case Calc::FN::fn: ARG_2 std::tie(val,CF)=name(lhs,rhs);break;
#define CALC_2C(fn,name) case Calc::FN::fn: ARG_2 std::tie(val,CF)=name(lhs,rhs,CF);break;
	switch (instr.fn){
		CALC_1( SHL,shift_left)
		CALC_1( SHR,shift_right)
		CALC_1C(RCL,shift_left)
		CALC_1C(RCR,shift_right)
		CALC_2( ADD,add)
		CALC_2( SUB,sub)
		CALC_2C(ADC,add)
		CALC_2C(SUC,sub)
		}
#undef ARG_1
#undef ARG_2
#undef CALC_1
#undef CALC_1C
#undef CALC_2
#undef CALC_2C
	push<u8>(val);
	pc++;
}
template<> void Context::run_instr(Logic instr) {

#define ARG_1 uint8_t rhs=pop<u8>();
#define ARG_2 ARG_1 uint8_t lhs=pop<u8>();
#define LOGIC_1(fn,name)  case Logic::FN::fn: {ARG_1 push<u8>(name rhs);break;}
#define LOGIC_2(fn,name)  case Logic::FN::fn: {ARG_2 push<u8>(lhs name rhs);break;}
	switch (instr.fn){
		LOGIC_1(NOT,~)
		LOGIC_2(AND,&)
		LOGIC_2(OR ,|)
		LOGIC_2(XOR,^)
	}
#undef ARG_1
#undef ARG_2
#undef LOGIC_1
#undef LOGIC_2
	pc++;
}
template<> void Context::run_instr(BranchCF instr,uint16_t addr) {
	pc=CF?addr:pc+1;
}
template<> void Context::run_instr(BranchZero instr,uint16_t addr) {
	pc=(pop<u8>()==0)?addr:pc+1;
}
template<> void Context::run_instr(Jump instr,uint16_t addr) {
	pc=addr;
}
template<> void Context::run_instr(ImmVal instr,uint8_t val) {
	push<u8>(val);
	pc++;
}
template<> void Context::run_instr(Call instr,uint16_t addr) {
	push<u16>(++pc);
	pc=addr;
}
template<> void Context::run_instr(CallPtr instr) {
	push<u16>(++pc);
	pc=pop<u16>();
}
template<> void Context::run_instr(Return instr) {
	pc=pop<u16>();
}
template<> void Context::run_instr(Adjust instr,int16_t offset) {
	sp+=offset;
	pc++;
}
template<> void Context::run_instr(Enter instr) {
	push<u16>(reg[instr.bp]);
	reg[instr.bp]=sp;
	pc++;
}
template<> void Context::run_instr(Leave instr) {
	sp=reg[instr.bp];
	reg[instr.bp]=pop<u16>();
	pc++;
}
template<> void Context::run_instr(Halt instr) {
	// halt();
}
template<> void Context::run_instr(INTCall instr) {
	//if(!arg.isINT()){
	//	 inc(MReg16::PC);
	//}
	//stack_push(MReg16::PC);
	//load_imm(MReg16::PC,arg.isINT());
	//jump(MReg16::TMP);
}
