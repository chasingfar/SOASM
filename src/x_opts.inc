
#define X(type,name) type,
	using opts = std::tuple< X_OPTS raw_t>;
#undef X
#define X(type,name) +InstrSet::opt_width<type>()
	static constexpr size_t optw = 0 X_OPTS;
#undef X
#define X(type,name) type name:InstrSet::opt_width<type>();
	X_OPTS
#undef X
#define X(type,name) -InstrSet::opt_width<type>()
	raw_t id:sizeof(raw_t)*CHAR_BIT X_OPTS;
#undef X
#undef X_OPTS