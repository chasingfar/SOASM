
#define X(type,name) type,
	using opts = std::tuple< X_OPTS instr_t>;
#undef X
#define X(type,name) +InstrSet::opt_width<type>()
	static constexpr size_t optw = 0 X_OPTS;
#undef X
#define X(type,name) type name:InstrSet::opt_width<type>();
	X_OPTS
#undef X
#define X(type,name) -InstrSet::opt_width<type>()
	instr_t id:sizeof(instr_t)*CHAR_BIT X_OPTS;
#undef X
#undef X_OPTS