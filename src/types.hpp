
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
		val_t get() const {
			return *ptr;
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

	template<typename ...Args>
	struct InstrArgs{
		static constexpr size_t num = sizeof...(Args);
		static constexpr size_t size = (0 + ... + Args::size);
		template<size_t I>
		using raw=std::tuple_element_t<I,std::tuple<Args...>>;
		template<typename T,size_t I>
		using raw_t=raw<I>::type;

		using raws_t=std::tuple<typename Args::type...>;

		static raws_t from_bytes(std::span<uint8_t> data){
			if constexpr(num==0){
				return {};
			}else{
				return [&]<size_t ...I>(std::index_sequence<I...>)->raws_t{
					std::array<size_t,num+1> offsets{0};
					((offsets[I+1]=offsets[I]+raw<I>::size),...);
					return {Args::from_bytes(data.subspan(offsets[I]))...};
				}(std::make_index_sequence<num>{});
			}
		}
	};

	template<typename Raw,typename Instr,typename ...Args>
	struct InstrBase{
		using raw = Raw;
		using raw_t = Raw::type;
		using args_t = InstrArgs<Args...>;
		static constexpr size_t size = Raw::size + args_t::size;
		struct Record{
			Instr instr;
			args_t::raws_t args;
		};

		template<typename InstrSet>
		void set_id(){
			static_cast<Instr*>(this)->id=InstrSet::template get_id<Instr>();
		}
		Raw to_raw() const{
			return std::bit_cast<raw_t>(*this);
		}
		static Instr from_bytes(std::span<uint8_t,raw::size> data){
			return std::bit_cast<Instr>(static_cast<raw_t>(Raw::from_bytes(data)));
		}
		auto operator()(Args... args){
			Instr instr=*static_cast<Instr*>(this);
			data_t data{};
			(std::ranges::move(args.may_lazys(), std::back_inserter(data)),...);
			return std::pair{instr,data};
		}
	};
}

#endif //SOASM_TYPES_HPP
