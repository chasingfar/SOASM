#include "model.hpp"
#include "instr_set.hpp"

using namespace SOASM::SOISv1;

template<> void Context::run(Unknown instr) {
	pc++;
}
template<> void Context::run(Init instr) {
	sp=0xFFFF;
	pc=0;
}
template<> void Context::run(LoadFar instr){
	push(mem.get(imm<uint16_t>()+reg.get(instr.from)));
	pc++;
}
template<> void Context::run(SaveFar instr) {
	mem.set(imm<uint16_t>()+reg.get(instr.to),pop<uint8_t>());
	pc++;
}
template<> void Context::run(LoadNear instr) {
	push(mem.get(imm<int8_t>()+reg.get(instr.from)));
	pc++;
}
template<> void Context::run(SaveNear instr) {
	mem.set(imm<int8_t>()+reg.get(instr.to),pop<uint8_t>());
	pc++;
}
template<> void Context::run(Load instr) {
	push(mem.get(reg.get(instr.from)));
	pc++;
}
template<> void Context::run(Save instr) {
	mem.set(reg.get(instr.to),pop<uint8_t>());
	pc++;
}
template<> void Context::run(SaveImm instr) {
	mem.set(reg.get(instr.to),imm<uint8_t>());
	pc++;
}
template<> void Context::run(Push instr) {
	push(reg.get(instr.from));
	pc++;
}
template<> void Context::run(Pop instr) {
	reg.set(instr.to,pop<uint8_t>());
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

template<> void Context::run(Calc instr) {
	uint8_t lhs,rhs,val;
#define ARG_1 rhs=pop<uint8_t>();
#define ARG_2 ARG_1 lhs=pop<uint8_t>();
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
	push(val);
	pc++;
}
template<> void Context::run(Logic instr) {

#define ARG_1 uint8_t rhs=pop<uint8_t>();
#define ARG_2 ARG_1 uint8_t lhs=pop<uint8_t>();
#define LOGIC_1(fn,name)  case Logic::FN::fn: {ARG_1 push(name rhs);break;}
#define LOGIC_2(fn,name)  case Logic::FN::fn: {ARG_2 push(lhs name rhs);break;}
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
template<> void Context::run(BranchCF instr) {
	auto addr=imm<uint16_t>();
	pc=CF?addr:pc+1;
}
template<> void Context::run(BranchZero instr) {
	auto addr=imm<uint16_t>();
	pc=(pop<uint8_t>()==0)?addr:pc+1;
}
template<> void Context::run(Jump instr) {
	pc=imm<uint16_t>();
}
template<> void Context::run(ImmVal instr) {
	push(imm<uint8_t>());
	pc++;
}
template<> void Context::run(Call instr) {
	auto addr=imm<uint16_t>();
	push(++pc);
	pc=addr;
}
template<> void Context::run(CallPtr instr) {
	push(++pc);
	pc=pop<uint16_t>();
}
template<> void Context::run(Return instr) {
	pc=pop<uint16_t>();
}
template<> void Context::run(Adjust instr) {
	sp+=imm<int16_t>();
	pc++;
}
template<> void Context::run(Enter instr) {
	push(reg.get(instr.bp));
	reg.set(instr.bp,sp);
	pc++;
}
template<> void Context::run(Leave instr) {
	sp=reg.get(instr.bp);
	reg.set(instr.bp,pop<uint16_t>());
	pc++;
}
template<> void Context::run(Halt instr) {
	// halt();
}
template<> void Context::run(INTCall instr) {
	//if(!arg.isINT()){
	//	 inc(MReg16::PC);
	//}
	//stack_push(MReg16::PC);
	//load_imm(MReg16::PC,arg.isINT());
	//jump(MReg16::TMP);
}
