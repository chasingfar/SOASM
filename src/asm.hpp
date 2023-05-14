
#ifndef SOASM_ASM_HPP
#define SOASM_ASM_HPP

#include <cstddef>
#include <map>
#include <ranges>
#include <vector>
#include <algorithm>
#include <functional>
#include <string>
#include "util/overloaded.hpp"

namespace SOASM{
	struct Lazy{
		std::shared_ptr<size_t> ptr;
		size_t offset;
		std::function<unsigned long long(size_t,size_t)> fn;
		uint8_t operator()(size_t pc) const{
			return (fn(*ptr,pc)>>offset)&0xffull;
		}
	};
	using may_lazy_t=std::variant<uint8_t,Lazy>;
	using data_t=std::vector<may_lazy_t>;
	using bytes_t=std::vector<uint8_t>;

	struct Label{
		using tbl_t=std::map<std::string,Label>;
		static tbl_t tbl; 

		std::shared_ptr<size_t> ptr;
		explicit Label(size_t addr=0) : ptr(std::make_shared<size_t>(addr)) {}
		explicit Label(std::string name,size_t addr=0) : ptr(std::make_shared<size_t>(addr)) {
			tbl[name]=*this;
		}
		void set(size_t v) const {
			*ptr = v;
		}
		operator Lazy(){
			return lazy();
		}
		Lazy lazy(){
			return Lazy{ptr,0,[](size_t addr,size_t pc){return addr;}};
		}
		Lazy offset(){
			return Lazy{ptr,0,[](size_t addr,size_t pc){return addr-pc;}};
		}
	};

	namespace RawTypes{
		template<bool IsBE,typename Raw,bool IsSigned=std::is_signed_v<Raw>,size_t Size=sizeof(Raw)>
		struct Int{
			using type=Raw;
			static constexpr bool is_big_endian=IsBE;
			static constexpr bool is_signed=IsSigned;
			static constexpr size_t size=Size;
			static constexpr auto offsets=std::views::iota(0uz,Size)|std::views::transform([](size_t i){return 8*(IsBE?Size-i-1:i);});

			std::variant<unsigned long long,Lazy> val;
			Int(std::integral auto val):val(static_cast<unsigned long long>(val)){}
			Int(std::span<uint8_t> bytes):val(from_bytes(bytes)){}
			Int(Lazy lazy):val(lazy){}
			Int(Label label):val(label.lazy()){}
			static auto from_bytes(std::span<uint8_t> bytes){
				unsigned long long v=0;
				for(auto&& [byte_val,offset]:std::views::zip(bytes,offsets)){
					v|=(byte_val&0xffull)<<offset;
				}
				return v;
			}
			static auto to_bytes(unsigned long long v){
				std::array<uint8_t,Size> bytes;
				for(auto&& [byte_val,offset]:std::views::zip(bytes,offsets)){
					byte_val=static_cast<uint8_t>((v>>offset)&0xff);
				}
				return bytes;
			}
			static auto to_bytes(const Lazy& v){
				std::array<Lazy,Size> bytes;
				for(auto&& [byte_val,offset]:std::views::zip(bytes,offsets)){
					byte_val=v;
					byte_val.offset=offset;
				}
				return bytes;
			}
			auto may_lazys() const{
				return std::visit([](auto v){
					std::array<may_lazy_t,Size> bytes;
					std::ranges::move(to_bytes(v),bytes.begin());
					return bytes;
				},val);
			}
		};

		using i8 = Int<false, int8_t>;
		using u8 = Int<false,uint8_t>;
		namespace LE{
			using i16 = Int<false, int16_t>;
			using i32 = Int<false, int32_t>;
			using i64 = Int<false, int64_t>;
			using u16 = Int<false,uint16_t>;
			using u32 = Int<false,uint32_t>;
			using u64 = Int<false,uint64_t>;
		} // LE
		namespace BE{
			using i16 = Int<true, int16_t>;
			using i32 = Int<true, int32_t>;
			using i64 = Int<true, int64_t>;
			using u16 = Int<true,uint16_t>;
			using u32 = Int<true,uint32_t>;
			using u64 = Int<true,uint64_t>;
		} // BE
	} // RawTypes

	template<typename Raw,typename Instr,typename ...Args>
	struct InstrBase{
		using raw_t = Raw::type;
		using args = std::tuple<Args...>;
		static constexpr size_t size = (sizeof(Raw::size) + ... + Args::size);

		template<typename InstrSet>
		void set_id(){
			static_cast<Instr*>(this)->id=InstrSet::template get_id<Instr>();
		}
		Raw to_raw() const{
			return std::bit_cast<raw_t>(*this);
		}
		auto operator()(Args... args){
			Instr instr=*static_cast<Instr*>(this);
			data_t data{};
			(std::ranges::move(args.may_lazys(), std::back_inserter(data)),...);
			return std::pair{instr,data};
		}
	};

	template<typename T>
	concept CanToCode=requires (T x){x.to_code();};

	template<typename InstrSet>
	struct ASM{
		struct Code{
			using val_type=std::variant<uint8_t,Lazy,Label>;
			using arg_type=std::variant<uint8_t,Lazy,Label,bytes_t,data_t,Code>;
			std::vector<val_type> codes;
			void add(std::integral auto val){
				codes.emplace_back(val&0xff);
			}
			template<typename T> requires std::same_as<T,Lazy> || std::same_as<T,Label>
			void add(const T& val){
				codes.emplace_back(val);
			}
			void add(std::ranges::range auto&& range){
				std::ranges::move(range|std::views::transform([](auto c){
					return std::visit([](auto v)->val_type{return v;},c);
				}), std::back_inserter(codes));
			}
			void add(const CanToCode auto& code){
				std::ranges::move(code.to_code().codes, std::back_inserter(codes));
			}
			template<typename Instr>
			void add(std::pair<Instr,data_t> code){
				auto [instr,data]=code;
				instr.template set_id<InstrSet>();
				add(instr.to_raw().may_lazys());
				add(data);
			}
			template<typename ...Ts>
			Code(Ts... code){
				(add(code),...);
			}
			auto begin(){return codes.begin();}
			auto end(){return codes.end();}
			auto begin() const{return codes.begin();}
			auto end() const{return codes.end();}

			static size_t count(const val_type& code){
				if(std::get_if<Label>(&code)){
					return 0;
				}
				return 1;
			}
			size_t size() const{
				size_t sum=0;
				for (const auto &code:codes) {
					sum+=count(code);
				}
				return sum;
			}

			data_t resolve(size_t start=0) const{
				data_t data{};
				data.reserve(size());
				for (const auto &code:codes) {
					std::visit(Util::overloaded{
						[&](const auto&  fn)    { data.emplace_back(fn); },
						[&](const Label& label) { label.set(start+data.size()); },
					}, code);
				}
				return data;
			}
			static bytes_t assemble(data_t data){
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
			bytes_t assemble(size_t start=0) const{
				return assemble(resolve(start));
			}
			static auto disassemble(bytes_t data){

			}
		};
	};
}
#endif //SOASM_ASM_HPP
