
#ifndef SOASM_TYPES_HPP
#define SOASM_TYPES_HPP

#include <cstddef>
#include <optional>
#include <memory>
#include <vector>
#include <functional>
#include <variant>
#include <map>
#include <ranges>
#include <algorithm>
#include <string>

namespace SOASM{
	struct Lazy{
		using val_t=std::optional<size_t>;
		std::shared_ptr<val_t> ptr;
		size_t offset;
		std::function<unsigned long long(size_t,size_t)> fn;
		uint8_t operator()(size_t pc) const{
			return (fn(ptr->value(),pc)>>offset)&0xffull;
		}
	};
	using may_lazy_t=std::variant<uint8_t,Lazy>;
	using data_t=std::vector<may_lazy_t>;
	using bytes_t=std::vector<uint8_t>;

	struct Label{
		using val_t=std::optional<size_t>;
		using tbl_t=std::map<std::string,Label>;
		static tbl_t tbl;

		std::shared_ptr<val_t> ptr;
		explicit Label(val_t addr={}) : ptr(std::make_shared<val_t>(addr)) {}
		explicit Label(std::string name,val_t addr={}) : Label(addr) {
			tbl[name]=*this;
		}
		const Label& set(size_t v) const {
			*ptr = v;
			return *this;
		}
		[[nodiscard]] val_t get() const {
			return *ptr;
		}
		operator Lazy(){
			return lazy();
		}
		[[nodiscard]] Lazy lazy() const{
			return Lazy{ptr,0,[](size_t addr,size_t pc){return addr;}};
		}
		[[nodiscard]] Lazy offset() const{
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
			Int(const Lazy& lazy):val(lazy){}
			Int(const Label& label):val(label.lazy()){}
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

	template<typename T>
	concept CanToCode=requires (T x){x.to_code();};

	struct Code{
		using val_type=std::variant<uint8_t,Lazy,Label>;

		std::vector<val_type> codes;
		void add(std::integral auto val){
			codes.emplace_back(static_cast<uint8_t>(val&0xffu));
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
		void add(CanToCode auto&& code){
			add(code.to_code());
		}
		template<typename ...Ts>
		Code(Ts&&... code){
			(add(code),...);
		}
		auto begin(){return codes.begin();}
		auto end(){return codes.end();}
		[[nodiscard]] auto begin() const{return codes.begin();}
		[[nodiscard]] auto end() const{return codes.end();}

		static size_t count(const val_type& code);
		[[nodiscard]] size_t size() const;

		[[nodiscard]] data_t resolve(size_t start=0,uint8_t padding=0xff) const;
		static bytes_t assemble(data_t data);
		[[nodiscard]] bytes_t assemble(size_t start=0,uint8_t padding=0xff) const{
			return assemble(resolve(start,padding));
		}
	};

}

#endif //SOASM_TYPES_HPP