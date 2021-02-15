#include <UDLua/DLua.h>

#include <ULuaW/ULuaW.h>

#include <UDRefl/UDRefl.h>
#include <UDRefl_ext/Bootstrap.h>

#include <exception>

using namespace Ubpa;

namespace Ubpa::details {
	namespace Meta {
		static constexpr auto add = TSTR("__add");
		static constexpr auto band = TSTR("__band");
		static constexpr auto bnot = TSTR("__bnot");
		static constexpr auto bor = TSTR("__bor");
		static constexpr auto bxor = TSTR("__bxor");
		static constexpr auto call = TSTR("__call");
		static constexpr auto concat = TSTR("__concat");
		static constexpr auto div = TSTR("__div");
		static constexpr auto eq = TSTR("__eq");
		static constexpr auto gc = TSTR("__gc");
		static constexpr auto idiv = TSTR("__idiv");
		static constexpr auto index = TSTR("__index");
		static constexpr auto le = TSTR("__le");
		static constexpr auto len = TSTR("__len");
		static constexpr auto lt = TSTR("__lt");
		static constexpr auto metatable = TSTR("__metatable");
		static constexpr auto mod = TSTR("__mod");
		static constexpr auto mode = TSTR("__mode");
		static constexpr auto mul = TSTR("__mul");
		static constexpr auto name = TSTR("__name");
		static constexpr auto newindex = TSTR("__newindex");
		static constexpr auto pairs = TSTR("__pairs");
		static constexpr auto pow = TSTR("__pow");
		static constexpr auto shl = TSTR("__shl");
		static constexpr auto shr = TSTR("__shr");
		static constexpr auto sub = TSTR("__sub");
		static constexpr auto tostring = TSTR("__tostring");
		static constexpr auto unm = TSTR("__unm");

		using t_add = decltype(add);
		using t_band = decltype(band);
		using t_bnot = decltype(bnot);
		using t_bor = decltype(bor);
		using t_bxor = decltype(bxor);
		using t_call = decltype(call);
		using t_concat = decltype(concat);
		using t_div = decltype(div);
		using t_eq = decltype(eq);
		using t_gc = decltype(gc);
		using t_idiv = decltype(idiv);
		using t_index = decltype(index);
		using t_le = decltype(le);
		using t_len = decltype(len);
		using t_lt = decltype(lt);
		using t_metatable = decltype(metatable);
		using t_mod = decltype(mod);
		using t_mode = decltype(mode);
		using t_mul = decltype(mul);
		using t_name = decltype(name);
		using t_newindex = decltype(newindex);
		using t_pairs = decltype(pairs);
		using t_pow = decltype(pow);
		using t_shl = decltype(shl);
		using t_shr = decltype(shr);
		using t_sub = decltype(sub);
		using t_tostring = decltype(tostring);
		using t_unm = decltype(unm);
	}

	namespace CppMeta {
		static constexpr auto op_bool = TSTR("__bool");
		static constexpr auto op_assign = TSTR("__assign");
		static constexpr auto op_deref = TSTR("__deref");
		static constexpr auto op_pre_inc = TSTR("__pre_inc");
		static constexpr auto op_pre_dec = TSTR("__pre_dec");
		static constexpr auto op_post_inc = TSTR("__post_inc");
		static constexpr auto op_post_dec = TSTR("__post_dec");

		static constexpr auto tuple_size = TSTR("tuple_size");
		static constexpr auto tuple_get = TSTR("tuple_get");

		static constexpr auto advance = TSTR("UDRefl::iterator_advance");
		static constexpr auto distance = TSTR("UDRefl::iterator_distance");
		static constexpr auto next = TSTR("UDRefl::iterator_next");
		static constexpr auto prev = TSTR("UDRefl::iterator_prev");

		static constexpr auto assign = TSTR("assign");
		static constexpr auto begin = TSTR("begin");
		static constexpr auto cbegin = TSTR("cbegin");
		static constexpr auto end = TSTR("end_");
		static constexpr auto cend = TSTR("cend");
		static constexpr auto rbegin = TSTR("rbegin");
		static constexpr auto crbegin = TSTR("crbegin");
		static constexpr auto rend = TSTR("rend");
		static constexpr auto crend = TSTR("crend");
		static constexpr auto at = TSTR("at");
		static constexpr auto data = TSTR("data");
		static constexpr auto front = TSTR("front");
		static constexpr auto back = TSTR("back");
		static constexpr auto empty = TSTR("empty");
		static constexpr auto size = TSTR("size");
		static constexpr auto max_size = TSTR("max_size");
		static constexpr auto resize = TSTR("resize");
		static constexpr auto capacity = TSTR("capacity");
		static constexpr auto bucket_count = TSTR("bucket_count");
		static constexpr auto reserve = TSTR("reserve");
		static constexpr auto shrink_to_fit = TSTR("shrink_to_fit");
		static constexpr auto clear = TSTR("clear");
		static constexpr auto insert = TSTR("insert");
		static constexpr auto insert_or_assign = TSTR("insert_or_assign");
		static constexpr auto erase = TSTR("erase");
		static constexpr auto push_front = TSTR("push_front");
		static constexpr auto pop_front = TSTR("pop_front");
		static constexpr auto push_back = TSTR("push_back");
		static constexpr auto pop_back = TSTR("pop_back");
		static constexpr auto swap = TSTR("swap");
		static constexpr auto merge = TSTR("merge");
		static constexpr auto extract = TSTR("extract");
		static constexpr auto count = TSTR("count");
		static constexpr auto find = TSTR("find");
		static constexpr auto lower_bound = TSTR("lower_bound");
		static constexpr auto upper_bound = TSTR("upper_bound");
		static constexpr auto equal_range = TSTR("equal_range");
		static constexpr auto key_comp = TSTR("key_comp");
		static constexpr auto value_comp = TSTR("value_comp");
		static constexpr auto hash_function = TSTR("hash_function");
		static constexpr auto key_eq = TSTR("key_eq");
		static constexpr auto get_allocator = TSTR("get_allocator");

		using t_op_bool = decltype(op_bool);
		using t_op_assign = decltype(op_assign);
		using t_op_deref = decltype(op_deref);
		using t_op_pre_inc = decltype(op_pre_inc);
		using t_op_pre_dec = decltype(op_pre_dec);
		using t_op_post_inc = decltype(op_post_inc);
		using t_op_post_dec = decltype(op_post_dec);

		using t_advance = decltype(advance);
		using t_distance = decltype(distance);
		using t_next = decltype(next);
		using t_prev = decltype(prev);

		using t_tuple_size = decltype(tuple_size);
		using t_tuple_get = decltype(tuple_get);

		using t_assign = decltype(assign);
		using t_begin = decltype(begin);
		using t_cbegin = decltype(cbegin);
		using t_end = decltype(end);
		using t_cend = decltype(cend);
		using t_rbegin = decltype(rbegin);
		using t_crbegin = decltype(crbegin);
		using t_rend = decltype(rend);
		using t_crend = decltype(crend);
		using t_at = decltype(at);
		using t_data = decltype(data);
		using t_front = decltype(front);
		using t_back = decltype(back);
		using t_empty = decltype(empty);
		using t_size = decltype(size);
		using t_max_size = decltype(max_size);
		using t_resize = decltype(resize);
		using t_capacity = decltype(capacity);
		using t_bucket_count = decltype(bucket_count);
		using t_reserve = decltype(reserve);
		using t_shrink_to_fit = decltype(shrink_to_fit);
		using t_clear = decltype(clear);
		using t_insert = decltype(insert);
		using t_insert_or_assign = decltype(insert_or_assign);
		using t_erase = decltype(erase);
		using t_push_front = decltype(push_front);
		using t_pop_front = decltype(pop_front);
		using t_push_back = decltype(push_back);
		using t_pop_back = decltype(pop_back);
		using t_swap = decltype(swap);
		using t_merge = decltype(merge);
		using t_extract = decltype(extract);
		using t_count = decltype(count);
		using t_find = decltype(find);
		using t_lower_bound = decltype(lower_bound);
		using t_upper_bound = decltype(upper_bound);
		using t_equal_range = decltype(equal_range);
		using t_key_comp = decltype(key_comp);
		using t_value_comp = decltype(value_comp);
		using t_hash_function = decltype(hash_function);
		using t_key_eq = decltype(key_eq);
		using t_get_allocator = decltype(get_allocator);
	}

	namespace CppMetaName {
		static constexpr auto ctor = TSTR("UDRefl::ctor");
		static constexpr auto dtor = TSTR("UDRefl::dtor");
		static constexpr auto operator_bool = TSTR("UDRefl::operator_bool");
		static constexpr auto operator_plus = TSTR("UDRefl::operator+");
		static constexpr auto operator_minus = TSTR("UDRefl::operator-");
		static constexpr auto operator_add = TSTR("UDRefl::operator+");
		static constexpr auto operator_sub = TSTR("UDRefl::operator-");
		static constexpr auto operator_mul = TSTR("UDRefl::operator*");
		static constexpr auto operator_div = TSTR("UDRefl::operator/");
		static constexpr auto operator_mod = TSTR("UDRefl::operator%");
		static constexpr auto operator_bnot = TSTR("UDRefl::operator~");
		static constexpr auto operator_band = TSTR("UDRefl::operator&");
		static constexpr auto operator_bor = TSTR("UDRefl::operator|");
		static constexpr auto operator_bxor = TSTR("UDRefl::operator^");
		static constexpr auto operator_lshift = TSTR("UDRefl::operator<<");
		static constexpr auto operator_rshift = TSTR("UDRefl::operator>>");
		static constexpr auto operator_pre_inc = TSTR("UDRefl::operator++");
		static constexpr auto operator_pre_dec = TSTR("UDRefl::operator--");
		static constexpr auto operator_post_inc = TSTR("UDRefl::operator++");
		static constexpr auto operator_post_dec = TSTR("UDRefl::operator--");
		static constexpr auto operator_assign = TSTR("UDRefl::operator=");
		static constexpr auto operator_assign_add = TSTR("UDRefl::operator+=");
		static constexpr auto operator_assign_sub = TSTR("UDRefl::operator-=");
		static constexpr auto operator_assign_mul = TSTR("UDRefl::operator*=");
		static constexpr auto operator_assign_div = TSTR("UDRefl::operator/=");
		static constexpr auto operator_assign_mod = TSTR("UDRefl::operator%=");
		static constexpr auto operator_assign_band = TSTR("UDRefl::operator&=");
		static constexpr auto operator_assign_bor = TSTR("UDRefl::operator|=");
		static constexpr auto operator_assign_bxor = TSTR("UDRefl::operator^=");
		static constexpr auto operator_assign_lshift = TSTR("UDRefl::operator<<=");
		static constexpr auto operator_assign_rshift = TSTR("UDRefl::operator>>=");
		static constexpr auto operator_eq = TSTR("UDRefl::operator==");
		static constexpr auto operator_ne = TSTR("UDRefl::operator!=");
		static constexpr auto operator_lt = TSTR("UDRefl::operator<");
		static constexpr auto operator_le = TSTR("UDRefl::operator<=");
		static constexpr auto operator_gt = TSTR("UDRefl::operator>");
		static constexpr auto operator_ge = TSTR("UDRefl::operator>=");
		static constexpr auto operator_and = TSTR("UDRefl::operator&&");
		static constexpr auto operator_or = TSTR("UDRefl::operator||");
		static constexpr auto operator_not = TSTR("UDRefl::operator!");
		static constexpr auto operator_subscript = TSTR("UDRefl::operator[]");
		static constexpr auto operator_deref = TSTR("UDRefl::operator*");
		static constexpr auto operator_ref = TSTR("UDRefl::operator&");
		static constexpr auto operator_member = TSTR("UDRefl::operator->");
		static constexpr auto operator_member_of_pointer = TSTR("UDRefl::operator->*");
		static constexpr auto operator_call = TSTR("UDRefl::operator()");
		static constexpr auto operator_comma = TSTR("UDRefl::operator,");
		static constexpr auto tuple_size = TSTR("UDRefl::tuple_size");
		static constexpr auto tuple_get = TSTR("UDRefl::tuple_get");
		static constexpr auto iterator_advance = TSTR("UDRefl::iterator_advance");
		static constexpr auto iterator_distance = TSTR("UDRefl::iterator_distance");
		static constexpr auto iterator_next = TSTR("UDRefl::iterator_next");
		static constexpr auto iterator_prev = TSTR("UDRefl::iterator_prev");
		static constexpr auto container_assign = TSTR("UDRefl::container_assign");
		static constexpr auto container_begin = TSTR("UDRefl::container_begin");
		static constexpr auto container_cbegin = TSTR("UDRefl::container_cbegin");
		static constexpr auto container_end = TSTR("UDRefl::container_end");
		static constexpr auto container_cend = TSTR("UDRefl::container_cend");
		static constexpr auto container_rbegin = TSTR("UDRefl::container_rbegin");
		static constexpr auto container_crbegin = TSTR("UDRefl::container_crbegin");
		static constexpr auto container_rend = TSTR("UDRefl::container_rend");
		static constexpr auto container_crend = TSTR("UDRefl::container_crend");
		static constexpr auto container_at = TSTR("UDRefl::container_at");
		static constexpr auto container_data = TSTR("UDRefl::container_data");
		static constexpr auto container_front = TSTR("UDRefl::container_front");
		static constexpr auto container_back = TSTR("UDRefl::container_back");
		static constexpr auto container_empty = TSTR("UDRefl::container_empty");
		static constexpr auto container_size = TSTR("UDRefl::container_size");
		static constexpr auto container_max_size = TSTR("UDRefl::container_max_size");
		static constexpr auto container_resize = TSTR("UDRefl::container_resize");
		static constexpr auto container_capacity = TSTR("UDRefl::container_capacity");
		static constexpr auto container_bucket_count = TSTR("UDRefl::container_bucket_count");
		static constexpr auto container_reserve = TSTR("UDRefl::container_reserve");
		static constexpr auto container_shrink_to_fit = TSTR("UDRefl::container_shrink_to_fit");
		static constexpr auto container_clear = TSTR("UDRefl::container_clear");
		static constexpr auto container_insert = TSTR("UDRefl::container_insert");
		static constexpr auto container_insert_or_assign = TSTR("UDRefl::container_insert_or_assign");
		static constexpr auto container_erase = TSTR("UDRefl::container_erase");
		static constexpr auto container_push_front = TSTR("UDRefl::container_push_front");
		static constexpr auto container_pop_front = TSTR("UDRefl::container_pop_front");
		static constexpr auto container_push_back = TSTR("UDRefl::container_push_back");
		static constexpr auto container_pop_back = TSTR("UDRefl::container_pop_back");
		static constexpr auto container_swap = TSTR("UDRefl::container_swap");
		static constexpr auto container_merge = TSTR("UDRefl::container_merge");
		static constexpr auto container_extract = TSTR("UDRefl::container_extract");
		static constexpr auto container_count = TSTR("UDRefl::container_count");
		static constexpr auto container_find = TSTR("UDRefl::container_find");
		static constexpr auto container_lower_bound = TSTR("UDRefl::container_lower_bound");
		static constexpr auto container_upper_bound = TSTR("UDRefl::container_upper_bound");
		static constexpr auto container_equal_range = TSTR("UDRefl::container_equal_range");
		static constexpr auto container_key_comp = TSTR("UDRefl::container_key_comp");
		static constexpr auto container_value_comp = TSTR("UDRefl::container_value_comp");
		static constexpr auto container_hash_function = TSTR("UDRefl::container_hash_function");
		static constexpr auto container_key_eq = TSTR("UDRefl::container_key_eq");
		static constexpr auto container_get_allocator = TSTR("UDRefl::container_get_allocator");

		using t_ctor = decltype(ctor);
		using t_dtor = decltype(dtor);
		using t_operator_bool = decltype(operator_bool);
		using t_operator_plus = decltype(operator_plus);
		using t_operator_minus = decltype(operator_minus);
		using t_operator_add = decltype(operator_add);
		using t_operator_sub = decltype(operator_sub);
		using t_operator_mul = decltype(operator_mul);
		using t_operator_div = decltype(operator_div);
		using t_operator_mod = decltype(operator_mod);
		using t_operator_bnot = decltype(operator_bnot);
		using t_operator_band = decltype(operator_band);
		using t_operator_bor = decltype(operator_bor);
		using t_operator_bxor = decltype(operator_bxor);
		using t_operator_lshift = decltype(operator_lshift);
		using t_operator_rshift = decltype(operator_rshift);
		using t_operator_pre_inc = decltype(operator_pre_inc);
		using t_operator_pre_dec = decltype(operator_pre_dec);
		using t_operator_post_inc = decltype(operator_post_inc);
		using t_operator_post_dec = decltype(operator_post_dec);
		using t_operator_assign = decltype(operator_assign);
		using t_operator_assign_add = decltype(operator_assign_add);
		using t_operator_assign_sub = decltype(operator_assign_sub);
		using t_operator_assign_mul = decltype(operator_assign_mul);
		using t_operator_assign_div = decltype(operator_assign_div);
		using t_operator_assign_mod = decltype(operator_assign_mod);
		using t_operator_assign_band = decltype(operator_assign_band);
		using t_operator_assign_bor = decltype(operator_assign_bor);
		using t_operator_assign_bxor = decltype(operator_assign_bxor);
		using t_operator_assign_lshift = decltype(operator_assign_lshift);
		using t_operator_assign_rshift = decltype(operator_assign_rshift);
		using t_operator_eq = decltype(operator_eq);
		using t_operator_ne = decltype(operator_ne);
		using t_operator_lt = decltype(operator_lt);
		using t_operator_le = decltype(operator_le);
		using t_operator_gt = decltype(operator_gt);
		using t_operator_ge = decltype(operator_ge);
		using t_operator_and = decltype(operator_and);
		using t_operator_or = decltype(operator_or);
		using t_operator_not = decltype(operator_not);
		using t_operator_subscript = decltype(operator_subscript);
		using t_operator_deref = decltype(operator_deref);
		using t_operator_ref = decltype(operator_ref);
		using t_operator_member = decltype(operator_member);
		using t_operator_member_of_pointer = decltype(operator_member_of_pointer);
		using t_operator_call = decltype(operator_call);
		using t_operator_comma = decltype(operator_comma);
		using t_tuple_size = decltype(tuple_size);
		using t_tuple_get = decltype(tuple_get);
		using t_iterator_advance = decltype(iterator_advance);
		using t_iterator_distance = decltype(iterator_distance);
		using t_iterator_next = decltype(iterator_next);
		using t_iterator_prev = decltype(iterator_prev);
		using t_container_assign = decltype(container_assign);
		using t_container_begin = decltype(container_begin);
		using t_container_cbegin = decltype(container_cbegin);
		using t_container_end = decltype(container_end);
		using t_container_cend = decltype(container_cend);
		using t_container_rbegin = decltype(container_rbegin);
		using t_container_crbegin = decltype(container_crbegin);
		using t_container_rend = decltype(container_rend);
		using t_container_crend = decltype(container_crend);
		using t_container_at = decltype(container_at);
		using t_container_data = decltype(container_data);
		using t_container_front = decltype(container_front);
		using t_container_back = decltype(container_back);
		using t_container_empty = decltype(container_empty);
		using t_container_size = decltype(container_size);
		using t_container_max_size = decltype(container_max_size);
		using t_container_resize = decltype(container_resize);
		using t_container_capacity = decltype(container_capacity);
		using t_container_bucket_count = decltype(container_bucket_count);
		using t_container_reserve = decltype(container_reserve);
		using t_container_shrink_to_fit = decltype(container_shrink_to_fit);
		using t_container_clear = decltype(container_clear);
		using t_container_insert = decltype(container_insert);
		using t_container_insert_or_assign = decltype(container_insert_or_assign);
		using t_container_erase = decltype(container_erase);
		using t_container_push_front = decltype(container_push_front);
		using t_container_pop_front = decltype(container_pop_front);
		using t_container_push_back = decltype(container_push_back);
		using t_container_pop_back = decltype(container_pop_back);
		using t_container_swap = decltype(container_swap);
		using t_container_merge = decltype(container_merge);
		using t_container_extract = decltype(container_extract);
		using t_container_count = decltype(container_count);
		using t_container_find = decltype(container_find);
		using t_container_lower_bound = decltype(container_lower_bound);
		using t_container_upper_bound = decltype(container_upper_bound);
		using t_container_equal_range = decltype(container_equal_range);
		using t_container_key_comp = decltype(container_key_comp);
		using t_container_value_comp = decltype(container_value_comp);
		using t_container_hash_function = decltype(container_hash_function);
		using t_container_key_eq = decltype(container_key_eq);
		using t_container_get_allocator = decltype(container_get_allocator);
	}
}

namespace Ubpa::details {
	struct ArgStack {
		void* argptr_buffer[UDRefl::MaxArgNum];
		std::aligned_storage_t<sizeof(Type), alignof(Type)> argType_buffer[UDRefl::MaxArgNum];
		std::uint64_t copied_args_buffer[UDRefl::MaxArgNum];
		std::size_t num_copied_args = 0;
	};

	struct CallHandle {
		Type type;
		Name method_name;
	};

	struct Invalid {}; // for f_meta

	template<typename T>
	T auto_get(LuaStateView L, int idx) {
		if constexpr (std::is_reference_v<T>)
			static_assert(always_false<T>);
		else if constexpr (std::is_integral_v<T>)
			return static_cast<T>(L.checkinteger(idx));
		else if constexpr (std::is_floating_point_v<T>)
			return static_cast<T>(L.checknumber(idx));
		else if constexpr (std::is_null_pointer_v<T>) {
			L.checktype(idx, LUA_TNIL);
			return std::nullptr_t;
		}
		else if constexpr (std::is_same_v<std::decay_t<T>, const char*>)
			return L.checkstring(idx);
		else if constexpr (std::is_same_v<std::decay_t<T>, std::string_view>) {
			size_t len;
			auto value = L.checklstring(idx, &len);
			return std::string_view{ value, len };
		}
		else if constexpr (std::is_pointer_v<T> && std::is_void_v<std::remove_pointer_t<T>>) {
			L.checktype(idx, LUA_TLIGHTUSERDATA);
			return L.touserdata(-idx);
		}
		else if constexpr (std::is_same_v<T, Name> || std::is_same_v<T, Type>) {
			int type = L.type(idx);
			switch (type)
			{
			case LUA_TSTRING:
				return T{ auto_get<std::string_view>(L, idx) };
			case LUA_TUSERDATA:
				return *(T*)L.checkudata(idx, type_name<T>().Data());
			default:
				L.error("auto_get : The %dth argument (%s) isn't convertible to %s.",
					idx, L.typename_(idx), type_name<T>().Data());
				return T{};
			}
		}
		else
			return *(T*)L.checkudata(idx, type_name<T>().Data());
	}

	template<typename Ret>
	void push(LuaStateView L, Ret rst) {
		if constexpr (std::is_reference_v<Ret>)
			static_assert(always_false<Ret>);
		else if constexpr (std::is_integral_v<Ret>) {
			if constexpr (std::is_same_v<std::decay_t<Ret>, bool>)
				L.pushboolean(rst);
			else
				L.pushinteger(rst);
		}
		else if constexpr (std::is_floating_point_v<Ret>)
			L.pushnumber(rst);
		else if constexpr (std::is_null_pointer_v<Ret>)
			L.pushnil();
		else if constexpr (std::is_same_v<std::decay_t<Ret>, const char*>)
			L.pushstring(rst);
		else if constexpr (std::is_same_v<std::decay_t<Ret>, std::string_view>)
			L.pushlstring(rst.data(), rst.size());
		else if constexpr (std::is_pointer_v<Ret> && std::is_same_v<void, std::remove_pointer_t<Ret>>)
			L.pushlightuserdata(rst);
		else {
			void* buffer = L.newuserdata(sizeof(Ret));
			new (buffer) Ret{ std::move(rst) };
			if (!L.getmetatable(type_name<Ret>().Data())) {
				int success = L.newmetatable(type_name<Ret>().Data());
				assert(success);
				if constexpr (!std::is_trivially_destructible_v<Ret>) {
					L.pushcfunction(wrap_dtor<Ret>());
					L.setfield(-2, "__gc");
				}
			}
			L.setmetatable(-2);
		}
	}

	// result is error code
	// 0 : ok
	// 1 : error (error message is at the top of the stack)
	int FillArgStack(LuaStateView L, ArgStack& stack, int begin, int cnt) {
		assert(cnt >= 0);
		if (cnt > UDRefl::MaxArgNum) {
			L.pushfstring("Ubpa::details::FillArgStack : The number of arguments (%d) is greater than UDRefl::MaxArgNum (%d).",
				cnt, static_cast<int>(UDRefl::MaxArgNum));
			return 1;
		}

		auto& copied_args_buffer = stack.copied_args_buffer;
		auto& argptr_buffer = stack.argptr_buffer;
		auto& argType_buffer = stack.argType_buffer;
		auto& num_copied_args = stack.num_copied_args;

		for (int i{ 0 }; i < cnt; i++) {
			int arg = begin + i;
			int type = L.type(arg);
			switch (type)
			{
			case LUA_TBOOLEAN:
			{
				auto arg_buffer = &copied_args_buffer[num_copied_args++];
				argptr_buffer[i] = arg_buffer;
				UDRefl::buffer_as<Type>(&argType_buffer[i]) = Type_of<bool>;
				UDRefl::buffer_as<bool>(arg_buffer) = static_cast<bool>(L.toboolean(arg));
				break;
			}
			case LUA_TNUMBER:
				if (L.isinteger(arg)) {
					auto arg_buffer = &copied_args_buffer[num_copied_args++];
					argptr_buffer[i] = arg_buffer;
					UDRefl::buffer_as<Type>(&argType_buffer[i]) = Type_of<lua_Integer>;
					UDRefl::buffer_as<lua_Integer>(arg_buffer) = static_cast<lua_Integer>(L.tointeger(arg));
				}
				else if (L.isnumber(arg)) {
					auto arg_buffer = &copied_args_buffer[num_copied_args++];
					argptr_buffer[i] = arg_buffer;
					UDRefl::buffer_as<Type>(&argType_buffer[i]) = Type_of<lua_Number>;
					UDRefl::buffer_as<lua_Number>(arg_buffer) = static_cast<lua_Number>(L.tonumber(arg));
				}
				else
					assert(false);
				break;
			case LUA_TSTRING:
			{
				auto arg_buffer = &copied_args_buffer[num_copied_args++];
				argptr_buffer[i] = arg_buffer;
				UDRefl::buffer_as<Type>(&argType_buffer[i]) = Type_of<const char*>;
				UDRefl::buffer_as<const char*>(arg_buffer) = L.tostring(arg);
				break;
			}
			case LUA_TUSERDATA:
				if (void* udata = L.testudata(arg, type_name<UDRefl::ObjectView>().Data())) {
					UDRefl::ObjectView arg = *static_cast<UDRefl::ObjectView*>(udata);
					auto ref_arg = arg.AddLValueReferenceWeak();
					argptr_buffer[i] = ref_arg.GetPtr();
					UDRefl::buffer_as<Type>(&argType_buffer[i]) = ref_arg.GetType();
				}
				else if (void* udata = L.testudata(arg, type_name<UDRefl::SharedObject>().Data())) {
					UDRefl::ObjectView arg = *static_cast<UDRefl::SharedObject*>(udata);
					auto ref_arg = arg.AddLValueReferenceWeak();
					argptr_buffer[i] = ref_arg.GetPtr();
					UDRefl::buffer_as<Type>(&argType_buffer[i]) = ref_arg.GetType();
				}
				else if (void* udata = L.testudata(arg, type_name<Name>().Data())) {
					auto* arg = static_cast<Name*>(udata);
					argptr_buffer[i] = arg;
					UDRefl::buffer_as<Type>(&argType_buffer[i]) = Type_of<Name>;
				}
				else if (void* udata = L.testudata(arg, type_name<Type>().Data())) {
					auto* arg = static_cast<Type*>(udata);
					argptr_buffer[i] = arg;
					UDRefl::buffer_as<Type>(&argType_buffer[i]) = Type_of<Type>;
				}
				else if (!L.getmetatable(arg)) {
					argptr_buffer[i] = L.touserdata(arg);
					UDRefl::buffer_as<Type>(&argType_buffer[i]) = Type_of<void*>;
				}
				else {
					int success = L.getfield(-1, "__name");
					const char* udata_name = "UNKNOWN";
					if (success)
						udata_name = L.tostring(-1);
					L.pushfstring("Ubpa::details::FillArgStack : ArgStack doesn't support %s (%s).",
						L.typename_(arg),
						udata_name
					);
					return 1;
				}
				break;
			default:
				L.pushfstring("Ubpa::details::FillArgStack : ArgStack doesn't support %s.", L.typename_(arg) );
				return 1;
			}
		}

		return 0;
	}

	int f_SharedObject_new_MethodPtr(lua_State* L_) {
		LuaStateView L{ L_ };

		int L_argnum = L.gettop();

		UDRefl::SharedObject methodptr_obj;
		LuaRef func_ref;
		Type result_type = Type_of<void>;
		Type object_type;
		UDRefl::ParamList list;

		// [object type, ]function[, result type = Type_of<void>]
		// [, ParamList = {}]

		if (L_argnum == 1) { // function
			L.checktype(1, LUA_TFUNCTION);
			func_ref = std::move(LuaRef{ L });
		}
		else if (L_argnum == 2) { // object type + function / function + result type
			if (L.type(1) == LUA_TFUNCTION) {
				// function + result type
				object_type = auto_get<Type>(L, 2);
				L.pop(1);
				func_ref = std::move(LuaRef{ L });
			}
			else {
				// object type + function
				object_type = auto_get<Type>(L, 1);
				func_ref = std::move(LuaRef{ L });
			}
		}
		else if (L_argnum == 3) {
			object_type = auto_get<Type>(L, 1);
			result_type = auto_get<Type>(L, 3);
			L.pop(1);
			L.checktype(2, LUA_TFUNCTION);
			func_ref = std::move(LuaRef{ L });
		}
		else {
			return L.error(
				"%s::new_MethodPtr :"
				"The number of arguments (%d) is invalid. The function needs 1~4 arguments([object type, ]function[, result type][, ParamList]).",
				type_name<UDRefl::SharedObject>().Data(), L_argnum);
		}
		
		UDRefl::MethodFlag flag;
		if (!object_type.GetID().Valid())
			flag = UDRefl::MethodFlag::Static;
		else if (object_type.IsConst())
			flag = UDRefl::MethodFlag::Const;
		else
			flag = UDRefl::MethodFlag::Variable;

		methodptr_obj = {
			Type_of<UDRefl::MethodPtr>,
			std::make_shared<UDRefl::MethodPtr>(
				UDRefl::MethodPtr::Func{ [object_type, result_type, fref = std::make_shared<LuaRef>(std::move(func_ref))] (void* obj, void* result_buffer, UDRefl::ArgsView args) mutable {
					auto L = fref->GetView();
					int top = L.gettop();
					fref->Get();
					const int n = static_cast<int>(args.GetParamList().size());
					int callargnum;
					if (object_type.Valid()) {
						callargnum = n + 1;
						L.checkstack(callargnum);
						push(L, UDRefl::ObjectView{ object_type, obj });
					}
					else {
						callargnum = n;
						L.checkstack(callargnum);
					}

					for (std::size_t i = 0; i < n; i++)
						push(L, args.At(i));
					int error = L.pcall(callargnum, LUA_MULTRET, 0);
					if (error) {
						std::stringstream ss;
						ss << type_name<UDRefl::SharedObject>().View() << "::new_MethodPtr::lambda:\n" << auto_get<std::string_view>(L, -1);
						std::string str = ss.str();
						std::exception except{ str.data() };
						L.pop(1);
						throw except;
					}

					if (!result_buffer || !result_type || result_type.IsVoid())
						return;

					int result_construct_argnum = L.gettop() - top;
					ArgStack argstack;
					{ // fill argstack
						int error = details::FillArgStack(L, argstack, top + 1, result_construct_argnum);
						if (error) {
							std::stringstream ss;
							ss << type_name<UDRefl::SharedObject>().View() << "::new_MethodPtr::lambda:\n" << auto_get<std::string_view>(L, -1);
							std::string str = ss.str();
							std::exception except{ str.data() };
							L.pop(1);
							throw except;
						}
					}

					{ // construct result
						bool success = UDRefl::Mngr.Construct(
							UDRefl::ObjectView{ result_type, result_buffer },
							std::span<const Type>{reinterpret_cast<Type*>(argstack.argType_buffer), static_cast<std::size_t>(result_construct_argnum)},
							static_cast<UDRefl::ArgPtrBuffer>(argstack.argptr_buffer)
						);
						if (!success) {
							std::stringstream ss;
							ss << type_name<UDRefl::SharedObject>().View() << "::new_MethodPtr::lambda: Construct fail.";
							std::string str = ss.str();
							std::exception except{ str.data() };
							L.pop(1);
							throw except;
						}
					}
				}},
				flag,
				result_type,
				std::move(list)
			)
		};

		void* buffer = L.newuserdata(sizeof(UDRefl::SharedObject));
		L.setmetatable(type_name<UDRefl::SharedObject>().Data());
		new(buffer)UDRefl::SharedObject{ std::move(methodptr_obj) };

		return 1;
	}

	template<typename T>
	constexpr lua_CFunction wrap_dtor() {
		return [](lua_State* L_) -> int {
			LuaStateView L{ L_ };
			auto* ptr = (T*)L.checkudata(1, type_name<T>().Data());
			ptr->~T();
			// std::cout << "[debug] dtor: " << type_name<T>().View() << std::endl;
			return 0;
		};
	}

	template<auto funcptr, typename Obj, std::size_t... Ns>
	void caller(LuaStateView L, Obj* obj, std::index_sequence<Ns...>) {
		using FuncObj = decltype(funcptr);
		using Traits = FuncTraits<FuncObj>;
		using ArgList = typename Traits::ArgList;
		using Ret = typename Traits::Return;
		if constexpr (std::is_void_v<Ret>)
			(obj->*funcptr)(auto_get<At_t<ArgList, Ns>>(L, 2 + Ns)...);
		else {
			auto ret = (obj->*funcptr)(auto_get<At_t<ArgList, Ns>>(L, 2 + Ns)...);
			push<Ret>(L, std::forward<Ret>(ret));
		}
	};

	template<auto funcptr, std::size_t... Ns>
	void caller(LuaStateView L, std::index_sequence<Ns...>) {
		using FuncObj = decltype(funcptr);
		using Traits = FuncTraits<FuncObj>;
		using ArgList = typename Traits::ArgList;
		using Ret = typename Traits::Return;
		if constexpr (std::is_void_v<Ret>)
			funcptr(auto_get<At_t<ArgList, Ns>>(L, 1 + Ns)...);
		else {
			auto ret = funcptr(auto_get<At_t<ArgList, Ns>>(L, 1 + Ns)...);
			push<Ret>(L, std::forward<Ret>(ret));
		}
	};

	template<auto funcptr, typename CustomObj, typename FuncName>
	constexpr lua_CFunction wrap(FuncName = {}) noexcept {
		using FuncObj = decltype(funcptr);
		using Traits = FuncTraits<FuncObj>;
		using ArgList = typename Traits::ArgList;
		using Ret = typename Traits::Return;
		return static_cast<lua_CFunction>([](lua_State* L_) -> int {
			LuaStateView L{ L_ };
			int n = L.gettop();
			if constexpr (std::is_member_function_pointer_v<FuncObj>) {
				using Obj = std::conditional_t<std::is_void_v<CustomObj>, typename Traits::Object, CustomObj>;

				if (n != 1 + Length_v<ArgList>)
					return L.error("%s::%s : The number of arguments is invalid. The function needs (object + %I) arguments.",
						type_name<Obj>().Data(),
						FuncName::Data(),
						static_cast<lua_Integer>(Length_v<ArgList>));

				auto* obj = (Obj*)L.checkudata(1, type_name<Obj>().Data());
				caller<funcptr>(L, obj, std::make_index_sequence<Length_v<ArgList>>{});
			}
			else if constexpr (is_function_pointer_v<FuncObj>) {
				using Obj = CustomObj;
				if (n != Length_v<ArgList>)
					return L.error("%s::%s : The number of arguments is invalid. The function needs %I arguments (no object).",
						type_name<Obj>().Data(),
						FuncName::Data(),
						static_cast<lua_Integer>(Length_v<ArgList>));

				caller<funcptr>(L, std::make_index_sequence<Length_v<ArgList>>{});
			}
			else
				static_assert(always_false<FuncObj>);

			if (std::is_void_v<Ret>)
				return 0;
			else
				return 1;
		});
	}
}

template<typename Functor, typename MetaName, typename CppMetaName = Ubpa::TStrC_of<>, int LArgNum = -1, typename Ret = Ubpa::details::Invalid>
static int f_meta(lua_State * L_) {
	LuaStateView L{ L_ };

	int L_argnum = L.gettop();
	if constexpr (LArgNum != -1) {
		if (L_argnum != LArgNum) {
			return L.error("%s::%s : The number of arguments is invalid. The function needs %d argument.",
				type_name<Functor>().Data(), MetaName::Data(), LArgNum);
		}
	}
	else {
		if (L_argnum <= 0) {
			return L.error("%s::%s : The number of arguments is invalid. The function needs >= 1 argument.",
				type_name<Functor>().Data(), MetaName::Data());
		}
	}

	UDRefl::ObjectView ptr;
	Name method_name;
	int argnum = L_argnum - 1;

	const auto& functor = *(Functor*)L.checkudata(1, type_name<Functor>().Data());

	if constexpr (UDRefl::IsObjectOrView_v<Functor>) {
		ptr = UDRefl::ObjectView{ functor.GetType(), functor.GetPtr() };
		method_name = { CppMetaName::View() };
	}
	else {
		static_assert(std::is_same_v<CppMetaName, Ubpa::TStrC_of<>>);
		ptr = { functor.type };
		method_name = functor.method_name;
		if (L_argnum >= 2) {
			if (void* udata = L.testudata(2, type_name<UDRefl::ObjectView>().Data())) {
				UDRefl::ObjectView obj = *static_cast<UDRefl::ObjectView*>(udata);
				if (obj.GetType() == functor.type) {
					ptr = obj;
					--argnum;
				}
			}
			else if (void* udata = L.testudata(2, type_name<UDRefl::SharedObject>().Data())) {
				UDRefl::ObjectView obj = *static_cast<UDRefl::SharedObject*>(udata);
				if (obj.GetType() == functor.type) {
					ptr = obj;
					--argnum;
				}
			}
		}
	}

	if (argnum > UDRefl::MaxArgNum) {
		return L.error("%s::%s : The number of arguments (%d) is greater than UDRefl::MaxArgNum (%d).",
			type_name<Functor>().Data(), MetaName::Data(), argnum, static_cast<int>(UDRefl::MaxArgNum));
	}

	details::ArgStack argstack;

	int actual_argnum;
	if constexpr (UDRefl::NameIDRegistry::Meta::operator_post_inc.Is(CppMetaName::View()) || UDRefl::NameIDRegistry::Meta::operator_post_dec.Is(CppMetaName::View())) {
		static_assert(LArgNum == 1);
		assert(argnum == 0);
		auto arg_buffer = &argstack.copied_args_buffer[argstack.num_copied_args++];
		argstack.argptr_buffer[0] = arg_buffer;
		UDRefl::buffer_as<Type>(&argstack.argType_buffer[0]) = Type_of<int>;
		UDRefl::buffer_as<int>(&argstack.copied_args_buffer[0]) = 0;
		actual_argnum = 1;
	}
	else
		actual_argnum = argnum;

	{ // fill argstack
		int error = details::FillArgStack(L, argstack, L_argnum - argnum + 1, argnum);
		if (error) {
			return L.error("%s::new : \n%s",
				type_name<UDRefl::SharedObject>().Data(), L.tostring(-1));
		}
	}

	if constexpr (!std::is_same_v<Ret, details::Invalid>) {
		Type result_type = ptr.IsInvocable(method_name,
			std::span<const Type>{reinterpret_cast<Type*>(argstack.argType_buffer),
			static_cast<std::size_t>(actual_argnum)});
		if (result_type.Is<Ret>()) {
			return L.error("%s::%s : The function isn't invocable with arguments or it's return type isn't %s.",
				type_name<Functor>().Data(),
				MetaName::Data(),
				type_name<Ret>().Data()
			);
		}

		if constexpr (std::is_void_v<Ret>) {
			ptr.InvokeRet<void>(
				method_name,
				std::span<const Type>{reinterpret_cast<Type*>(argstack.argType_buffer), static_cast<std::size_t>(actual_argnum)},
				static_cast<UDRefl::ArgPtrBuffer>(argstack.argptr_buffer)
			);
			return 0;
		}
		else {
			Ret rst = ptr.InvokeRet<Ret>(
				method_name,
				std::span<const Type>{reinterpret_cast<Type*>(argstack.argType_buffer), static_cast<std::size_t>(actual_argnum)},
				static_cast<UDRefl::ArgPtrBuffer>(argstack.argptr_buffer)
			);

			details::push<Ret>(L, std::move(rst));

			return 1;
		}
	}
	else {
		UDRefl::SharedObject rst = ptr.DMInvoke(
			method_name,
			std::span<const Type>{reinterpret_cast<Type*>(argstack.argType_buffer), static_cast<std::size_t>(actual_argnum)},
			static_cast<UDRefl::ArgPtrBuffer>(argstack.argptr_buffer)
		);

		if (!rst.GetType()) {
			return L.error("%s::%s : The function isn't invocable with arguments.",
				type_name<Functor>().Data(),
				MetaName::Data()
			);
		}

		if (rst.GetType().Is<void>())
			return 0;

		void* buffer = L.newuserdata(sizeof(UDRefl::SharedObject));
		new(buffer)UDRefl::SharedObject{ std::move(rst) };
		L.getmetatable(type_name<UDRefl::SharedObject>().Data());
		L.setmetatable(-2);

		return 1;
	}
}

template<typename T>
static int f_T_new(lua_State* L_) {
	LuaStateView L{ L_ };
	int size = L.gettop();
	if (size == 0) {
		void* buffer = L.newuserdata(sizeof(T));
		new (buffer) T{};
	}
	else if (size == 1) {
		int type = L.type(-1);
		switch (type)
		{
		case LUA_TSTRING:
		{
			size_t len;
			const char* str = L.tolstring(1, &len);
			void* buffer = L.newuserdata(sizeof(T));
			std::string_view sv{ str, len };
			T t;
			if constexpr (std::is_same_v<T, Name>)
				t = UDRefl::Mngr.nregistry.Register(sv);
			else
				t = UDRefl::Mngr.tregistry.Register(sv);
			new (buffer) Type{ t };
			break;
		}
		default:
			return L.error("%s::new : The type of argument#1 is invalid. The function needs 0 argument / a string.", type_name<T>().Data());
		}
	}
	else
		return L.error("%s::new : The number of arguments is invalid. The function needs 0 argument / a string.", type_name<T>().Data());

	L.getmetatable(type_name<T>().Data());
	L.setmetatable(-2);
	return 1;
}

template<typename Obj>
static int f_Obj_new(lua_State* L_) {
	LuaStateView L{ L_ };
	int size = L.gettop();
	if (size == 0) {
		void* buffer = L.newuserdata(sizeof(Obj));
		new (buffer) Obj;
	}
	else if (size == 1) {
		int type = L.type(-1);
		switch (type)
		{
		case LUA_TUSERDATA:
		{
			auto* id = (Type*)L.checkudata(1, type_name<Type>().Data());
			void* buffer = L.newuserdata(sizeof(Obj));
			new (buffer) Obj{ *id };
			break;
		}
		case LUA_TNIL:
		{
			void* buffer = L.newuserdata(sizeof(Obj));
			new (buffer) Obj{ nullptr };
			break;
		}
		default:
			return L.error("%s::new : The type of argument#1 is invalid. The function needs 0 argument / a Type/nil.", type_name<Obj>().Data());
		}
	}
	else
		return L.error("%s::new : The number of arguments is invalid. The function needs 0 argument / a Type/nil.", type_name<Obj>().Data());

	L.getmetatable(type_name<Obj>().Data());
	L.setmetatable(-2);
	return 1;
}

template<typename Obj>
static int f_Obj_AsNumber(lua_State* L_) {
	LuaStateView L{ L_ };

	if (L.gettop() != 1)
		return L.error("%s::AsNumber : The number of arguments is invalid. The function needs 1 argument (object).", type_name<Obj>().Data());

	auto ptr = static_cast<UDRefl::ObjectView>(*(Obj*)L.checkudata(1, type_name<Obj>().Data()));

	if (!ptr.GetPtr())
		return L.error("%s::AsNumber : The object is nil.", type_name<Obj>().Data());

	ptr = ptr.RemoveConstReference();

	std::string_view tname = ptr.GetType().GetName();

	if (tname == type_name<bool>().View())
		L.pushboolean(ptr.As<bool>());
	else if (type_name_is_integral(tname)) {
		lua_Integer value;
		UDRefl::ObjectView{ value }.AInvoke<void>(UDRefl::NameIDRegistry::Meta::ctor, ptr);
		L.pushinteger(value);
	}
	else if (type_name_is_floating_point(tname)) {
		lua_Number value;
		UDRefl::ObjectView{ value }.AInvoke<void>(UDRefl::NameIDRegistry::Meta::ctor, ptr);
		L.pushnumber(value);
	}
	else {
		return L.error("%s::AsNumber : The type (%s) can't convert to a number.",
			type_name<Obj>().Data(),
			tname.data());
	}

	return 1;
}

template<typename Obj>
static int f_Obj_tostring(lua_State* L_) {
	LuaStateView L{ L_ };

	if (L.gettop() != 1)
		return L.error("%s::__tostring : The number of arguments is invalid. The function needs 1 argument (object).", type_name<Obj>().Data());

	auto& ptr = static_cast<UDRefl::ObjectView&>(*(Obj*)L.checkudata(1, type_name<Obj>().Data()));

	if (!ptr.GetPtr())
		return L.error("%s::__tostring : The object is nil.", type_name<Obj>().Data());

	if (!ptr.IsInvocable<std::stringstream&>(UDRefl::NameIDRegistry::Meta::operator_rshift)) {
		return L.error("%s::__tostring : The type (%s) can't convert to a string.",
			type_name<Obj>().Data(),
			ptr.GetType().GetName().data());
	}

	std::stringstream ss;
	ss << ptr;
	auto str = ss.str();

	L.pushlstring(str.data(), str.size());

	return 1;
}

template<typename Obj>
static int f_Obj_index(lua_State* L_) {
	LuaStateView L{ L_ };

	if (L.gettop() != 2)
		return L.error("%s::__index : The number of arguments is invalid. The function needs 2 argument (object + key).", type_name<Obj>().Data());

	int type = L.type(2);

	Name key;
	switch (type)
	{
	case LUA_TUSERDATA:
	{
		if (auto* pName = (Name*)L.testudata(2, type_name<Name>().Data()))
			key = *pName;
		else
			return f_meta<Obj, details::Meta::t_index, details::CppMetaName::t_operator_subscript, 2>(L_);

		break;
	}
	case LUA_TSTRING:
	{
		size_t len;
		const char* key_name = L.checklstring(2, &len);
		key = Name{ std::string_view{key_name, len} };
		break;
	}
	default:
		return f_meta<Obj, details::Meta::t_index, details::CppMetaName::t_operator_subscript, 2>(L_);
	}
	
	UDRefl::ObjectView ptr = *(Obj*)L.checkudata(1, type_name<Obj>().Data());

	if (key.Is("this")) {
		L.pushvalue(1);
		return 1;
	}
	else if (auto key_obj = ptr.Var(key); key_obj.GetPtr()) {
		auto* buffer = L.newuserdata(sizeof(UDRefl::ObjectView));
		new(buffer)UDRefl::ObjectView{ key_obj };
		L.getmetatable(type_name<UDRefl::ObjectView>().Data());
		L.setmetatable(-2);
	}
	else if (ptr.ContainsMethod(key)) {
		auto* buffer = L.newuserdata(sizeof(details::CallHandle));
		new(buffer)details::CallHandle{ ptr.GetType(), key };
		L.getmetatable(type_name<details::CallHandle>().Data());
		L.setmetatable(-2);
	}
	else {
		if (!key) {
			return L.error("%s::__index : %s can't index a non-var and non-method Name.",
				type_name<Obj>().Data(),
				ptr.GetType().GetName().data());
		}

		int success = L.getmetatable(-2); // Obj's metatable
		assert(success);
		int contain = L.getfield(-1, key.GetView().data());
		if (!contain) {
			return L.error("%s::__index : %s not contain %s.",
				type_name<Obj>().Data(),
				ptr.GetType().GetName().data(),
				key.GetView().data());
		}
	}
	return 1;
}

template<typename Obj>
static int f_Obj_newindex(lua_State* L_) {
	LuaStateView L{ L_ };
	if (L.gettop() != 3) {
		return L.error("%s::__newindex : The number of arguments is invalid. The function needs 3 argument (object, key, value).",
			type_name<Obj>().Data());
	}

	// stack : ptr, key, value
	L.getmetatable(type_name<UDRefl::ObjectView>().Data());
	L.getmetatable(type_name<Obj>().Data());
	L.getfield(-2, details::CppMeta::op_assign.Data());
	L.getfield(-2, details::Meta::index.Data());
	L.rotate(1, -2);
	// stack : value, ..., __assign, __index, ptr, key
	{
		int error = L.pcall(2, 1, 0);
		if (error) {
			return L.error("%s::__newindex::__index: %s",
				type_name<Obj>().Data(), L.tostring(-1));
		}
	}
	// stack : value, ..., __assign, __index result (member ptr)
	L.rotate(1, -1);
	// stack : ..., __assign, __index result (member ptr), value
	{
		int error = L.pcall(2, 1, 0);
		if (error) {
			return L.error("%s::__newindex::__assign: %s",
				type_name<Obj>().Data(), L.tostring(-1));
		}
	}
	return 0;
}

template<typename Obj>
static int f_range_next(lua_State* L_) {
	LuaStateView L{ L_ };
	const int argnum = L.gettop();

	if (argnum != 1 && argnum != 2) {
		return L.error("range_next : The number of arguments is invalid. The function needs 1/2 argument (end_iter[, iter/nil]).");
	}

	UDRefl::ObjectView ptr = *(Obj*)L.checkudata(1, type_name<Obj>().Data());
	UDRefl::SharedObject end_iter = ptr.end();

	if (!end_iter.GetType()) {
		return L.error("range_next : The type (%s) can't invoke end.",
			type_name<Obj>().Data(),
			ptr.GetType().GetName().data());
	}
	if (argnum == 1)
		L.pushnil();

	int type = L.type(2);
	switch (type)
	{
	case LUA_TNIL:
	{
		UDRefl::SharedObject iter = ptr.begin();
		if (!iter.GetType()) {
			return L.error("range_next : The type (%s) can't invoke begin.",
				type_name<Obj>().Data(),
				ptr.GetType().GetName().data());
		}
		if (iter == end_iter)
			L.pushnil();
		else
			details::push<UDRefl::SharedObject>(L, std::move(iter));
		return 1;
	}
	case LUA_TUSERDATA:
	{
		UDRefl::ObjectView iter = *(UDRefl::SharedObject*)L.checkudata(2, type_name<UDRefl::SharedObject>().Data());
		UDRefl::SharedObject rst = ++iter;
		if (!rst.GetType()) {
			return L.error("range_next : The type (%s) can't invoke operator++().",
				type_name<Obj>().Data(),
				iter.GetType().GetName().data());
		}
		if (iter == end_iter)
			L.pushnil();
		return 1; // stack top is the iter / nil
	}
	default:
		return L.error("range_next : The second arguments must be a nil/iter.");
	}
}

template<typename Obj>
static int f_Obj_range(lua_State* L_) {
	LuaStateView L{ L_ };
	if (L.gettop() != 1) {
		return L.error("%s::range : The number of arguments is invalid. The function needs 1 argument (obj).",
			type_name<Obj>().Data());
	}
	L.checkudata(1, type_name<Obj>().Data());
	L.pushcfunction(&f_range_next<Obj>);
	L.pushvalue(1);
	L.pushnil();
	return 3;
}

static int f_SharedObject_new(lua_State* L_) {
	LuaStateView L{ L_ };
	const int L_argnum = L.gettop();
	if (L_argnum <= 0)
		return L.error("%s::new : The number of arguments is invalid.", type_name<UDRefl::SharedObject>().Data());

	Type type;
	int argtype = L.type(1);
	switch (argtype)
	{
	case LUA_TSTRING:
		type = Type{ details::auto_get<std::string_view>(L, 1) };
		break;
	case LUA_TUSERDATA:
		type = *static_cast<Type*>(L.checkudata(1, type_name<Type>().Data()));
		break;
	default:
		return L.error("%s::new : The function doesn't support %s.",
			type_name<UDRefl::SharedObject>().Data(),
			L.typename_(1)
		);
	}

	const int argnum = L_argnum - 1;

	details::ArgStack argstack;
	int error = details::FillArgStack(L, argstack, L_argnum - argnum + 1, argnum);

	if (error) {
		return L.error("%s::new : \n%s",
			type_name<UDRefl::SharedObject>().Data(), L.tostring(-1));
	}

	UDRefl::SharedObject obj = UDRefl::Mngr.MakeShared(
		type,
		std::span<const Type>{reinterpret_cast<Type*>(argstack.argType_buffer), static_cast<std::size_t>(argnum)},
		static_cast<UDRefl::ArgPtrBuffer>(argstack.argptr_buffer)
	);

	if (!obj.GetType())
		return L.error("%s::new : Fail.", type_name<UDRefl::SharedObject>().Data());

	auto* buffer = L.newuserdata(sizeof(UDRefl::SharedObject));
	new(buffer)UDRefl::SharedObject{ std::move(obj) };

	L.getmetatable(type_name<UDRefl::SharedObject>().Data());
	L.setmetatable(-2);

	return 1;
}

static const struct luaL_Reg lib_Name[] = {
	"new", f_T_new<Name>,
	NULL , NULL
};

static const struct luaL_Reg meta_Name[] = {
	"GetID", details::wrap<&Name::GetID, Name>(TSTR("GetID")),
	"GetView", details::wrap<&Name::GetView, Name>(TSTR("GetView")),
	"Valid", details::wrap<&Name::Valid, Name>(TSTR("Valid")),
	"Is", details::wrap<&Name::Is, Name>(TSTR("Is")),
	NULL, NULL
};

static const struct luaL_Reg lib_Type[] = {
	"new", f_T_new<Type>,
	NULL , NULL
};

static const struct luaL_Reg meta_Type[] = {
	"GetID", details::wrap<&Type::GetID, Type>(TSTR("GetID")),
	"GetName", details::wrap<&Type::GetName, Type>(TSTR("GetName")),
	"Valid", details::wrap<&Type::Valid, Type>(TSTR("Valid")),
	"Is", details::wrap<MemFuncOf<Type, bool(std::string_view)const noexcept>::get(&Type::Is), Type>(TSTR("Is")),
	NULL, NULL
};

static const struct luaL_Reg lib_ObjectView[] = {
	"new"     , f_Obj_new<UDRefl::ObjectView>,
	NULL      , NULL
};

static const struct luaL_Reg meta_ObjectView[] = {
	"GetType", details::wrap<&UDRefl::ObjectView::GetType, UDRefl::ObjectView>(TSTR("GetType")),
	"GetPtr", details::wrap<&UDRefl::ObjectView::GetPtr, UDRefl::ObjectView>(TSTR("GetPtr")),
	"AsNumber", f_Obj_AsNumber<UDRefl::ObjectView>,

	"__index", &f_Obj_index<UDRefl::ObjectView>,
	"__newindex",& f_Obj_newindex<UDRefl::ObjectView>,
	"__tostring", &f_Obj_tostring<UDRefl::ObjectView>,
	"__call",  &f_meta<UDRefl::ObjectView, details::Meta::t_call, details::CppMetaName::t_operator_call>,
	"__add", &f_meta<UDRefl::ObjectView, details::Meta::t_add, details::CppMetaName::t_operator_add, 2>,
	"__band", &f_meta<UDRefl::ObjectView, details::Meta::t_band, details::CppMetaName::t_operator_band, 2>,
	"__bnot", &f_meta<UDRefl::ObjectView, details::Meta::t_bnot, details::CppMetaName::t_operator_bnot, 1>,
	"__bor", &f_meta<UDRefl::ObjectView, details::Meta::t_bor, details::CppMetaName::t_operator_bor, 2>,
	"__bxor", &f_meta<UDRefl::ObjectView, details::Meta::t_bxor, details::CppMetaName::t_operator_bxor, 2>,
	"__div", &f_meta<UDRefl::ObjectView, details::Meta::t_div, details::CppMetaName::t_operator_div, 2>,
	"__eq", &f_meta<UDRefl::ObjectView, details::Meta::t_eq, details::CppMetaName::t_operator_eq, 2, bool>,
	"__le", &f_meta<UDRefl::ObjectView, details::Meta::t_le, details::CppMetaName::t_operator_le, 2, bool>,
	"__lt", &f_meta<UDRefl::ObjectView, details::Meta::t_lt, details::CppMetaName::t_operator_lt, 2, bool>,
	"__mod", &f_meta<UDRefl::ObjectView, details::Meta::t_mod, details::CppMetaName::t_operator_mod, 2>,
	"__mul", &f_meta<UDRefl::ObjectView, details::Meta::t_mul, details::CppMetaName::t_operator_mul, 2>,
	"__shl", &f_meta<UDRefl::ObjectView, details::Meta::t_shl, details::CppMetaName::t_operator_lshift, 2>,
	"__shr", &f_meta<UDRefl::ObjectView, details::Meta::t_shr, details::CppMetaName::t_operator_rshift, 2>,
	"__sub", &f_meta<UDRefl::ObjectView, details::Meta::t_sub, details::CppMetaName::t_operator_sub, 2>,
	"__unm", &f_meta<UDRefl::ObjectView, details::Meta::t_unm, details::CppMetaName::t_operator_minus, 2>,

	"__bool",& f_meta<UDRefl::ObjectView, details::CppMeta::t_op_bool, details::CppMetaName::t_operator_bool, 2, bool>,
	"__assign",& f_meta<UDRefl::ObjectView, details::CppMeta::t_op_assign, details::CppMetaName::t_operator_assign, 2>,
	"__deref", &f_meta<UDRefl::ObjectView, details::CppMeta::t_op_deref, details::CppMetaName::t_operator_deref, 1>,
	"__pre_inc", &f_meta<UDRefl::ObjectView, details::CppMeta::t_op_pre_inc, details::CppMetaName::t_operator_pre_inc, 1>,
	"__pre_dec", &f_meta<UDRefl::ObjectView, details::CppMeta::t_op_pre_dec, details::CppMetaName::t_operator_pre_dec, 1>,
	"__post_inc", &f_meta<UDRefl::ObjectView, details::CppMeta::t_op_post_inc, details::CppMetaName::t_operator_post_inc, 1>,
	"__post_dec", &f_meta<UDRefl::ObjectView, details::CppMeta::t_op_post_dec, details::CppMetaName::t_operator_post_dec, 1>,

	"advance",& f_meta<UDRefl::ObjectView, details::CppMeta::t_advance, details::CppMetaName::t_iterator_advance, 2, void>,
	"distance",& f_meta<UDRefl::ObjectView, details::CppMeta::t_distance, details::CppMetaName::t_iterator_distance, 2, std::size_t>,
	"next",& f_meta<UDRefl::ObjectView, details::CppMeta::t_next, details::CppMetaName::t_iterator_next>,
	"prev",& f_meta<UDRefl::ObjectView, details::CppMeta::t_prev, details::CppMetaName::t_iterator_prev>,

	"tuple_size",& f_meta<UDRefl::ObjectView, details::CppMeta::t_tuple_size, details::CppMetaName::t_tuple_size, 1, std::size_t>,
	"tuple_get",& f_meta<UDRefl::ObjectView, details::CppMeta::t_tuple_get, details::CppMetaName::t_tuple_get, 2, UDRefl::ObjectView>,

	"begin",& f_meta<UDRefl::ObjectView, details::CppMeta::t_begin, details::CppMetaName::t_container_begin, 1>,
	"cbegin",& f_meta<UDRefl::ObjectView, details::CppMeta::t_cbegin, details::CppMetaName::t_container_cbegin, 1>,
	"end_",& f_meta<UDRefl::ObjectView, details::CppMeta::t_end, details::CppMetaName::t_container_end, 1>,
	"cend",& f_meta<UDRefl::ObjectView, details::CppMeta::t_cend, details::CppMetaName::t_container_cend, 1>,
	"rbegin",& f_meta<UDRefl::ObjectView, details::CppMeta::t_rbegin, details::CppMetaName::t_container_rbegin, 1>,
	"crbegin",& f_meta<UDRefl::ObjectView, details::CppMeta::t_crbegin, details::CppMetaName::t_container_crbegin, 1>,
	"rend",& f_meta<UDRefl::ObjectView, details::CppMeta::t_rend, details::CppMetaName::t_container_rend, 1>,
	"crend",& f_meta<UDRefl::ObjectView, details::CppMeta::t_crend, details::CppMetaName::t_container_crend, 1>,
	"at",& f_meta<UDRefl::ObjectView, details::CppMeta::t_at, details::CppMetaName::t_container_at, 2>,
	"data",& f_meta<UDRefl::ObjectView, details::CppMeta::t_data, details::CppMetaName::t_container_data, 1>,
	"front",& f_meta<UDRefl::ObjectView, details::CppMeta::t_front, details::CppMetaName::t_container_front, 1>,
	"back",& f_meta<UDRefl::ObjectView, details::CppMeta::t_back, details::CppMetaName::t_container_back, 1>,
	"empty",& f_meta<UDRefl::ObjectView, details::CppMeta::t_empty, details::CppMetaName::t_container_empty, 1, bool>,
	"size",& f_meta<UDRefl::ObjectView, details::CppMeta::t_size, details::CppMetaName::t_container_size, 1, std::size_t>,
	"capacity",& f_meta<UDRefl::ObjectView, details::CppMeta::t_capacity, details::CppMetaName::t_container_capacity, 1, std::size_t>,
	"bucket_count",& f_meta<UDRefl::ObjectView, details::CppMeta::t_bucket_count, details::CppMetaName::t_container_bucket_count, 1, std::size_t>,
	"count",& f_meta<UDRefl::ObjectView, details::CppMeta::t_count, details::CppMetaName::t_container_count, 2, std::size_t>,
	"find",& f_meta<UDRefl::ObjectView, details::CppMeta::t_find, details::CppMetaName::t_container_find, 2>,
	"lower_bound",& f_meta<UDRefl::ObjectView, details::CppMeta::t_lower_bound, details::CppMetaName::t_container_lower_bound, 2>,
	"upper_bound",& f_meta<UDRefl::ObjectView, details::CppMeta::t_upper_bound, details::CppMetaName::t_container_upper_bound, 2>,
	"equal_range",& f_meta<UDRefl::ObjectView, details::CppMeta::t_equal_range, details::CppMetaName::t_container_equal_range, 2>,
	"key_comp",& f_meta<UDRefl::ObjectView, details::CppMeta::t_key_comp, details::CppMetaName::t_container_key_comp, 1>,
	"value_comp",& f_meta<UDRefl::ObjectView, details::CppMeta::t_value_comp, details::CppMetaName::t_container_value_comp, 1>,
	"hash_function",& f_meta<UDRefl::ObjectView, details::CppMeta::t_hash_function, details::CppMetaName::t_container_hash_function, 1>,
	"key_eq",& f_meta<UDRefl::ObjectView, details::CppMeta::t_key_eq, details::CppMetaName::t_container_key_eq, 1>,
	"get_allocator",& f_meta<UDRefl::ObjectView, details::CppMeta::t_get_allocator, details::CppMetaName::t_container_get_allocator, 1>,

	"range", f_Obj_range<UDRefl::ObjectView>,

	NULL      , NULL
};

static const struct luaL_Reg lib_SharedObject[] = {
	"new", f_SharedObject_new,
	"new_MethodPtr", details::f_SharedObject_new_MethodPtr,
	NULL, NULL
};

static const struct luaL_Reg meta_SharedObject[] = {
	"GetType", details::wrap<&UDRefl::SharedObject::GetType, UDRefl::SharedObject>(TSTR("GetType")),
	"GetPtr", details::wrap<&UDRefl::SharedObject::GetPtr, UDRefl::SharedObject>(TSTR("GetPtr")),
	"AsNumber", f_Obj_AsNumber<UDRefl::SharedObject>,

	"__gc", details::wrap_dtor<UDRefl::SharedObject>(),
	"__index", &f_Obj_index<UDRefl::SharedObject>,
	"__newindex",&f_Obj_newindex<UDRefl::SharedObject>,
	"__tostring", &f_Obj_tostring<UDRefl::SharedObject>,
	"__call", &f_meta<UDRefl::SharedObject, details::Meta::t_call, details::CppMetaName::t_operator_call>,
	"__add", &f_meta<UDRefl::SharedObject, details::Meta::t_add, details::CppMetaName::t_operator_add, 2>,
	"__band",&f_meta<UDRefl::SharedObject, details::Meta::t_band, details::CppMetaName::t_operator_band, 2>,
	"__bnot",&f_meta<UDRefl::SharedObject, details::Meta::t_bnot, details::CppMetaName::t_operator_bnot, 1>,
	"__bor",&f_meta<UDRefl::SharedObject, details::Meta::t_bor, details::CppMetaName::t_operator_bor, 2>,
	"__div",&f_meta<UDRefl::SharedObject, details::Meta::t_div, details::CppMetaName::t_operator_div, 2>,
	"__eq",&f_meta<UDRefl::SharedObject, details::Meta::t_eq, details::CppMetaName::t_operator_eq, 2, bool>,
	"__le",&f_meta<UDRefl::SharedObject, details::Meta::t_le, details::CppMetaName::t_operator_le, 2, bool>,
	"__lt",&f_meta<UDRefl::SharedObject, details::Meta::t_lt, details::CppMetaName::t_operator_lt, 2, bool>,
	"__mod",&f_meta<UDRefl::SharedObject, details::Meta::t_mod, details::CppMetaName::t_operator_mod, 2>,
	"__mul",&f_meta<UDRefl::SharedObject, details::Meta::t_mul, details::CppMetaName::t_operator_mul, 2>,
	"__pow", &f_meta<UDRefl::SharedObject, details::Meta::t_pow, details::CppMetaName::t_operator_bxor, 2>,
	"__shl",&f_meta<UDRefl::SharedObject, details::Meta::t_shl, details::CppMetaName::t_operator_lshift, 2>,
	"__shr",&f_meta<UDRefl::SharedObject, details::Meta::t_shr, details::CppMetaName::t_operator_rshift, 2>,
	"__sub",&f_meta<UDRefl::SharedObject, details::Meta::t_sub, details::CppMetaName::t_operator_sub, 2>,
	"__unm", &f_meta<UDRefl::SharedObject, details::Meta::t_unm, details::CppMetaName::t_operator_minus, 2>,

	"__bool",& f_meta<UDRefl::SharedObject, details::CppMeta::t_op_bool, details::CppMetaName::t_operator_bool, 2, bool>,
	"__assign",& f_meta<UDRefl::SharedObject, details::CppMeta::t_op_assign, details::CppMetaName::t_operator_assign, 2>,
	"__deref", &f_meta<UDRefl::SharedObject, details::CppMeta::t_op_deref, details::CppMetaName::t_operator_deref, 1>,
	"__pre_inc", &f_meta<UDRefl::SharedObject, details::CppMeta::t_op_pre_inc, details::CppMetaName::t_operator_pre_inc, 1>,
	"__pre_dec", &f_meta<UDRefl::SharedObject, details::CppMeta::t_op_pre_dec, details::CppMetaName::t_operator_pre_dec, 1>,
	"__post_inc", &f_meta<UDRefl::SharedObject, details::CppMeta::t_op_post_inc, details::CppMetaName::t_operator_post_inc, 1>,
	"__post_dec", &f_meta<UDRefl::SharedObject, details::CppMeta::t_op_post_dec, details::CppMetaName::t_operator_post_dec, 1>,

	"advance",& f_meta<UDRefl::SharedObject, details::CppMeta::t_advance, details::CppMetaName::t_iterator_advance, 2, void>,
	"distance",& f_meta<UDRefl::SharedObject, details::CppMeta::t_distance, details::CppMetaName::t_iterator_distance, 2, std::size_t>,
	"next",& f_meta<UDRefl::SharedObject, details::CppMeta::t_next, details::CppMetaName::t_iterator_next>,
	"prev",& f_meta<UDRefl::SharedObject, details::CppMeta::t_prev, details::CppMetaName::t_iterator_prev>,

	"tuple_size",& f_meta<UDRefl::SharedObject, details::CppMeta::t_tuple_size, details::CppMetaName::t_tuple_size, 1, std::size_t>,
	"tuple_get",& f_meta<UDRefl::SharedObject, details::CppMeta::t_tuple_get, details::CppMetaName::t_tuple_get, 2, UDRefl::ObjectView>,

	"begin",& f_meta<UDRefl::SharedObject, details::CppMeta::t_begin, details::CppMetaName::t_container_begin, 1>,
	"cbegin",& f_meta<UDRefl::SharedObject, details::CppMeta::t_cbegin, details::CppMetaName::t_container_cbegin, 1>,
	"end_",& f_meta<UDRefl::SharedObject, details::CppMeta::t_end, details::CppMetaName::t_container_end, 1>,
	"cend",& f_meta<UDRefl::SharedObject, details::CppMeta::t_cend, details::CppMetaName::t_container_cend, 1>,
	"rbegin",& f_meta<UDRefl::SharedObject, details::CppMeta::t_rbegin, details::CppMetaName::t_container_rbegin, 1>,
	"crbegin",& f_meta<UDRefl::SharedObject, details::CppMeta::t_crbegin, details::CppMetaName::t_container_crbegin, 1>,
	"rend",& f_meta<UDRefl::SharedObject, details::CppMeta::t_rend, details::CppMetaName::t_container_rend, 1>,
	"crend",& f_meta<UDRefl::SharedObject, details::CppMeta::t_crend, details::CppMetaName::t_container_crend, 1>,
	"at",& f_meta<UDRefl::SharedObject, details::CppMeta::t_at, details::CppMetaName::t_container_at, 2>,
	"data",& f_meta<UDRefl::SharedObject, details::CppMeta::t_data, details::CppMetaName::t_container_data, 1>,
	"front",& f_meta<UDRefl::SharedObject, details::CppMeta::t_front, details::CppMetaName::t_container_front, 1>,
	"back",& f_meta<UDRefl::SharedObject, details::CppMeta::t_back, details::CppMetaName::t_container_back, 1>,
	"empty",& f_meta<UDRefl::SharedObject, details::CppMeta::t_empty, details::CppMetaName::t_container_empty, 1, bool>,
	"size",& f_meta<UDRefl::SharedObject, details::CppMeta::t_size, details::CppMetaName::t_container_size, 1, std::size_t>,
	"capacity",& f_meta<UDRefl::SharedObject, details::CppMeta::t_capacity, details::CppMetaName::t_container_capacity, 1, std::size_t>,
	"bucket_count",& f_meta<UDRefl::SharedObject, details::CppMeta::t_bucket_count, details::CppMetaName::t_container_bucket_count, 1, std::size_t>,
	"count",& f_meta<UDRefl::SharedObject, details::CppMeta::t_count, details::CppMetaName::t_container_count, 2, std::size_t>,
	"find",& f_meta<UDRefl::SharedObject, details::CppMeta::t_find, details::CppMetaName::t_container_find, 2>,
	"lower_bound",& f_meta<UDRefl::SharedObject, details::CppMeta::t_lower_bound, details::CppMetaName::t_container_lower_bound, 2>,
	"upper_bound",& f_meta<UDRefl::SharedObject, details::CppMeta::t_upper_bound, details::CppMetaName::t_container_upper_bound, 2>,
	"equal_range",& f_meta<UDRefl::SharedObject, details::CppMeta::t_equal_range, details::CppMetaName::t_container_equal_range, 2>,
	"key_comp",& f_meta<UDRefl::SharedObject, details::CppMeta::t_key_comp, details::CppMetaName::t_container_key_comp, 1>,
	"value_comp",& f_meta<UDRefl::SharedObject, details::CppMeta::t_value_comp, details::CppMetaName::t_container_value_comp, 1>,
	"hash_function",& f_meta<UDRefl::SharedObject, details::CppMeta::t_hash_function, details::CppMetaName::t_container_hash_function, 1>,
	"key_eq",& f_meta<UDRefl::SharedObject, details::CppMeta::t_key_eq, details::CppMetaName::t_container_key_eq, 1>,
	"get_allocator",& f_meta<UDRefl::SharedObject, details::CppMeta::t_get_allocator, details::CppMetaName::t_container_get_allocator, 1>,

	"range", f_Obj_range<UDRefl::SharedObject>,

	NULL                 , NULL
};

static void init_CallHandle(lua_State* L_) {
	LuaStateView L{ L_ };

	L.newmetatable(type_name<details::CallHandle>().Data());
	L.pushcfunction(&f_meta<details::CallHandle, details::Meta::t_call>);
	L.setfield(-2, "__call");

	L.pop(1);
}

static int luaopen_Name(lua_State* L_) {
	LuaStateView L{ L_ };
	L.newmetatable(type_name<Name>().Data());
	L.setfuncs(meta_Name, 0);
	L.pushvalue(-1);
	L.setfield(-2, "__index");
	L.newlib(lib_Name);
	return 1;
}

static int luaopen_Type(lua_State* L_) {
	LuaStateView L{ L_ };
	L.newmetatable(type_name<Type>().Data());
	L.setfuncs(meta_Type, 0);
	L.pushvalue(-1);
	L.setfield(-2, "__index");
	L.newlib(lib_Type);
	return 1;
}

static int luaopen_ObjectView(lua_State* L_) {
	LuaStateView L{ L_ };
	L.newmetatable(type_name<UDRefl::ObjectView>().Data());
	L.setfuncs(meta_ObjectView, 0);
	L.newlib(lib_ObjectView);
	{ // register Global
		void* buffer = L.newuserdata(sizeof(UDRefl::ObjectView));
		new(buffer) UDRefl::ObjectView{ UDRefl::Global };
		L.setmetatable(type_name<UDRefl::ObjectView>().Data());
		L.setfield(-2, "Global");
	}
	{ // register ReflMngr
		UDRefl::ext::Bootstrap();
		void* buffer = L.newuserdata(sizeof(UDRefl::ObjectView));
		new(buffer) UDRefl::ObjectView{ Type_of<UDRefl::ReflMngr>, &UDRefl::Mngr };
		L.setmetatable(type_name<UDRefl::ObjectView>().Data());
		L.setfield(-2, "ReflMngr");
	}
	return 1;
}

static int luaopen_SharedObject(lua_State* L_) {
	LuaStateView L{ L_ };
	L.newmetatable(type_name<UDRefl::SharedObject>().Data());
	L.setfuncs(meta_SharedObject, 0);
	L.newlib(lib_SharedObject);
	return 1;
}

static const luaL_Reg UDRefl_libs[] = {
  {"Name", luaopen_Name},
  {"Type", luaopen_Type},
  {"ObjectView", luaopen_ObjectView},
  {"SharedObject", luaopen_SharedObject},
  {NULL, NULL}
};

void luaopen_UDRefl_libs(lua_State* L_) {
	init_CallHandle(L_);

	LuaStateView L{ L_ };
	const luaL_Reg* lib;
	// "require" functions from 'UDRefl_libs' and set results to global table
	for (lib = UDRefl_libs; lib->func; lib++) {
		L.requiref(lib->name, lib->func, 1);
		lua_pop(L, 1); // remove lib
	}
}
