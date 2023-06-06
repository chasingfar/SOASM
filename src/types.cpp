#include <soasm/types.hpp>
#include <soasm/util/overloaded.hpp>

using namespace SOASM;

Label::tbl_t Label::tbl{};

size_t Code::count(const Code::val_type &code) {
	if(std::get_if<Label>(&code)){
		return 0;
	}
	return 1;
}

size_t Code::size() const {
	size_t sum=0;
	for (const auto &code:codes) {
		sum+=count(code);
	}
	return sum;
}

data_t Code::resolve(size_t start, uint8_t padding) const {
	data_t data{};
	data.reserve(size());
	for (const auto &code:codes) {
		std::visit(Util::overloaded{
				[&](const auto&  fn)    { data.emplace_back(fn); },
				[&](const Label& label) {
					if(auto addr=label.get();addr){
						data.resize(*addr-start,padding);
					}else{
						label.set(start+data.size());
					}
				},
		}, code);
	}
	return data;
}

bytes_t Code::assemble(data_t data) {
	bytes_t bytes;
	bytes.reserve(data.size());
	for (auto &code:data) {
		bytes.emplace_back(std::visit(Util::overloaded{
				[&](const Lazy &lazy) { return lazy(bytes.size()); },
				[&](uint8_t v) { return v; },
		}, code));
	}
	return bytes;
}
