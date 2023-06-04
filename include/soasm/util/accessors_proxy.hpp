#ifndef UTIL_ACCESSORS_PROXY_HPP
#define UTIL_ACCESSORS_PROXY_HPP

namespace Util{
	template<typename Ref>
	struct AccessorsProxy{
		template<typename Key>
		struct Proxy{
			Ref& ref;
			Key key;
			operator decltype(ref.get(key))() const{
				return ref.get(key);
			}
			template<typename Value>
			Proxy& operator =(Value&& value){
				ref.set(key,value);
				return *this;
			}
		};
		template<typename Key>
		Proxy<Key> operator [](Key key){
			return {*static_cast<Ref*>(this),key};
		}
		template<typename Key>
		Proxy<Key> operator [](Key key) const{
			return {*static_cast<Ref*>(this),key};
		}
	};
}

#endif //UTIL_ACCESSORS_PROXY_HPP
