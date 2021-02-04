#include <UDLua/DLua.h>

#include <ULuaW/LuaStateView.h>

#include <UDRefl/UDRefl.h>

#include <lua.hpp>

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
		static constexpr auto op_assign = TSTR("__assign");
		static constexpr auto op_deref = TSTR("__deref");
		static constexpr auto op_pre_inc = TSTR("__pre_inc");
		static constexpr auto op_pre_dec = TSTR("__pre_dec");
		static constexpr auto op_post_inc = TSTR("__post_inc");
		static constexpr auto op_post_dec = TSTR("__post_dec");

		static constexpr auto tuple_size = TSTR("tuple_size");
		static constexpr auto tuple_get = TSTR("tuple_get");

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

		using t_op_assign = decltype(op_assign);
		using t_op_deref = decltype(op_deref);
		using t_op_pre_inc = decltype(op_pre_inc);
		using t_op_pre_dec = decltype(op_pre_dec);
		using t_op_post_inc = decltype(op_post_inc);
		using t_op_post_dec = decltype(op_post_dec);

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

	template<typename T>
	struct LibName;
}
template<>
struct Ubpa::details::LibName<UDRefl::ObjectPtr> {
	static constexpr const char value[] = "ObjectPtr";
};
template<>
struct Ubpa::details::LibName<UDRefl::ConstObjectPtr> {
	static constexpr const char value[] = "ConstObjectPtr";
};
template<>
struct Ubpa::details::LibName<UDRefl::SharedObject> {
	static constexpr const char value[] = "SharedObject";
};
template<>
struct Ubpa::details::LibName<UDRefl::SharedConstObject> {
	static constexpr const char value[] = "SharedConstObject";
};

namespace Ubpa::details {
	struct CallHandle {
		TypeID typeID;
		StrID methodID;
	};

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

	template<typename Arg>
	auto get_arg(LuaStateView L, int arg) {
		if constexpr (std::is_reference_v<Arg>)
			static_assert(always_false<Arg>);
		else if constexpr (std::is_integral_v<Arg>)
			return L.checkinteger(arg);
		else if constexpr (std::is_floating_point_v<Arg>)
			return L.checknumber(arg);
		else if constexpr (std::is_null_pointer_v<Arg>) {
			L.checktype(arg, LUA_TNIL);
			return std::nullptr_t;
		}
		else if constexpr (std::is_same_v<std::decay_t<Arg>, const char*>)
			return L.checkstring(arg);
		else if constexpr (std::is_same_v<std::decay_t<Arg>, std::string_view>) {
			size_t len;
			auto value = L.checklstring(arg, &len);
			return std::string_view{ value, len };
		}
		else if constexpr (std::is_pointer_v<Arg> && std::is_void_v<std::remove_pointer_t<Arg>>) {
			L.checktype(arg, LUA_TLIGHTUSERDATA);
			return L.touserdata(-arg);
		}
		else
			return *(Arg*)L.checkudata(arg, type_name<Arg>().Data());
	}

	template<typename Ret>
	void push_rst(LuaStateView L, Ret rst) {
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

	template<auto funcptr, typename Obj, std::size_t... Ns>
	void caller(LuaStateView L, Obj* obj, std::index_sequence<Ns...>) {
		using FuncPtr = decltype(funcptr);
		using Traits = FuncTraits<FuncPtr>;
		using ArgList = typename Traits::ArgList;
		using Ret = typename Traits::Return;
		if constexpr (std::is_void_v<Ret>)
			(obj->*funcptr)(get_arg<At_t<ArgList, Ns>>(L, 2 + Ns)...);
		else {
			auto ret = (obj->*funcptr)(get_arg<At_t<ArgList, Ns>>(L, 2 + Ns)...);
			push_rst<Ret>(L, std::forward<Ret>(ret));
		}
	};

	template<auto funcptr, std::size_t... Ns>
	void caller(LuaStateView L, std::index_sequence<Ns...>) {
		using FuncPtr = decltype(funcptr);
		using Traits = FuncTraits<FuncPtr>;
		using ArgList = typename Traits::ArgList;
		using Ret = typename Traits::Return;
		if constexpr (std::is_void_v<Ret>)
			funcptr(get_arg<At_t<ArgList, Ns>>(L, 1 + Ns)...);
		else {
			auto ret = funcptr(get_arg<At_t<ArgList, Ns>>(L, 1 + Ns)...);
			push_rst<Ret>(L, std::forward<Ret>(ret));
		}
	};

	template<auto funcptr, typename CustomObj, typename FuncName>
	constexpr lua_CFunction wrap(FuncName = {}) noexcept {
		using FuncPtr = decltype(funcptr);
		using Traits = FuncTraits<FuncPtr>;
		using ArgList = typename Traits::ArgList;
		using Ret = typename Traits::Return;
		return static_cast<lua_CFunction>([](lua_State* L_) -> int {
			LuaStateView L{ L_ };
			int n = L.gettop();
			if constexpr (std::is_member_function_pointer_v<FuncPtr>) {
				using Obj = std::conditional_t<std::is_void_v<CustomObj>, typename Traits::Object, CustomObj>;

				if (n != 1 + Length_v<ArgList>)
					return L.error("%s::%s : The number of arguments is invalid. The function needs (object + %I) arguments.",
						type_name<Obj>().Data(),
						FuncName::Data(),
						static_cast<lua_Integer>(Length_v<ArgList>));

				auto* obj = (Obj*)L.checkudata(1, type_name<Obj>().Data());
				caller<funcptr>(L, obj, std::make_index_sequence<Length_v<ArgList>>{});
			}
			else if constexpr (is_function_pointer_v<FuncPtr>) {
				using Obj = CustomObj;
				if (n != Length_v<ArgList>)
					return L.error("%s::%s : The number of arguments is invalid. The function needs %I arguments (no object).",
						type_name<Obj>().Data(),
						FuncName::Data(),
						static_cast<lua_Integer>(Length_v<ArgList>));

				caller<funcptr>(L, std::make_index_sequence<Length_v<ArgList>>{});
			}
			else
				static_assert(always_false<FuncPtr>);

			if (std::is_void_v<Ret>)
				return 0;
			else
				return 1;
		});
	}
}

template<typename Functor, typename MetaName, std::size_t ID = static_cast<std::size_t>(-1), int LArgNum = -1, bool ExplictRet = false, typename Ret = void>
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

	using Ptr = std::conditional_t<std::is_same_v<Functor, UDRefl::ConstObjectPtr> || std::is_same_v<Functor, UDRefl::SharedConstObject>,
		UDRefl::ConstObjectPtr,
		UDRefl::ObjectPtr>;
	Ptr ptr;
	StrID methodID;
	int argnum = L_argnum - 1;

	const auto& functor = *(Functor*)L.checkudata(1, type_name<Functor>().Data());

	if constexpr (UDRefl::IsObjectOrPtr_v<Functor>) {
		ptr = functor;
		methodID = StrID{ ID };
	}
	else {
		static_assert(ID == static_cast<std::size_t>(-1));
		ptr = Ptr{ functor.typeID };
		methodID = functor.methodID;
		if (L_argnum >= 2) {
			if (void* udata = L.testudata(2, type_name<UDRefl::ConstObjectPtr>().Data())) {
				const auto& obj = *static_cast<UDRefl::ConstObjectPtr*>(udata);
				if (obj.GetID() == functor.typeID) {
					ptr = UDRefl::ConstCast(obj.AddConstLValueReference());
					--argnum;
				}
			}
			else if (void* udata = L.testudata(2, type_name<UDRefl::ObjectPtr>().Data())) {
				const auto& obj = *static_cast<UDRefl::ObjectPtr*>(udata);
				if (obj.GetID() == functor.typeID) {
					ptr = obj.AddLValueReference();
					--argnum;
				}
			}
			else if (void* udata = L.testudata(2, type_name<UDRefl::SharedConstObject>().Data())) {
				const auto& obj = *static_cast<UDRefl::SharedConstObject*>(udata);
				if (obj.GetID() == functor.typeID) {
					ptr = UDRefl::ConstCast(obj->AddConstLValueReference());
					--argnum;
				}
			}
			else if (void* udata = L.testudata(2, type_name<UDRefl::SharedObject>().Data())) {
				const auto& obj = *static_cast<UDRefl::SharedObject*>(udata);
				if (obj.GetID() == functor.typeID) {
					ptr = obj->AddLValueReference();
					--argnum;
				}
			}
		}
	}

	if (argnum > UDRefl::MaxArgNum) {
		return L.error("%s::%s : The number of arguments (%d) is greater than UDRefl::MaxArgNum (%d).",
			type_name<Functor>().Data(), MetaName::Data(), argnum, static_cast<int>(UDRefl::MaxArgNum));
	}

	void* argptr_buffer[UDRefl::MaxArgNum];
	std::size_t argTypeID_buffer[UDRefl::MaxArgNum];
	std::uint64_t copied_args_buffer[UDRefl::MaxArgNum];
	std::size_t num_copied_args = 0;

	if constexpr (ID == UDRefl::StrIDRegistry::MetaID::operator_post_inc.GetValue() || ID == UDRefl::StrIDRegistry::MetaID::operator_post_dec.GetValue()) {
		static_assert(LArgNum == 1);
		assert(argnum == 0);
		auto arg_buffer = &copied_args_buffer[num_copied_args++];
		argptr_buffer[0] = arg_buffer;
		argTypeID_buffer[0] = TypeID_of<int>.GetValue();
		UDRefl::buffer_as<int>(arg_buffer) = 0;
	}

	for (std::size_t i{ 0 }; i < argnum; i++) {
		int arg = static_cast<int>(i) + L_argnum - argnum + 1;
		int type = L.type(arg);
		switch (type)
		{
		case LUA_TBOOLEAN:
		{
			auto arg_buffer = &copied_args_buffer[num_copied_args++];
			argptr_buffer[i] = arg_buffer;
			argTypeID_buffer[i] = TypeID_of<bool>.GetValue();
			UDRefl::buffer_as<bool>(arg_buffer) = static_cast<bool>(L.toboolean(arg));
		}
		break;
		case LUA_TNUMBER:
			if (L.isinteger(arg)) {
				auto arg_buffer = &copied_args_buffer[num_copied_args++];
				argptr_buffer[i] = arg_buffer;
				argTypeID_buffer[i] = TypeID_of<lua_Integer>.GetValue();
				UDRefl::buffer_as<lua_Integer>(arg_buffer) = static_cast<lua_Integer>(L.tointeger(arg));
			}
			else if (L.isnumber(arg)) {
				auto arg_buffer = &copied_args_buffer[num_copied_args++];
				argptr_buffer[i] = arg_buffer;
				argTypeID_buffer[i] = TypeID_of<lua_Number>.GetValue();
				UDRefl::buffer_as<lua_Number>(arg_buffer) = static_cast<lua_Number>(L.tonumber(arg));
			}
			else
				assert(false);
			break;
		//	// TODO
		//case LUA_TSTRING:
		//	assert(false);
		//	break;
		case LUA_TUSERDATA:
			if (void* udata = L.testudata(-1, type_name<UDRefl::ConstObjectPtr>().Data())) {
				const auto& rhs = *static_cast<UDRefl::ConstObjectPtr*>(udata);
				auto ref_rhs = rhs.AddConstLValueReference();
				argptr_buffer[i] = const_cast<void*>(ref_rhs.GetPtr());
				argTypeID_buffer[i] = ref_rhs.GetID().GetValue();
			}
			else if (void* udata = L.testudata(-1, type_name<UDRefl::ObjectPtr>().Data())) {
				const auto& rhs = *static_cast<UDRefl::ObjectPtr*>(udata);
				auto ref_rhs = rhs.AddLValueReference();
				argptr_buffer[i] = const_cast<void*>(ref_rhs.GetPtr());
				argTypeID_buffer[i] = ref_rhs.GetID().GetValue();
			}
			else if (void* udata = L.testudata(-1, type_name<UDRefl::SharedConstObject>().Data())) {
				const auto& rhs = *static_cast<UDRefl::SharedConstObject*>(udata);
				auto ref_rhs = rhs->AddConstLValueReference();
				argptr_buffer[i] = const_cast<void*>(ref_rhs.GetPtr());
				argTypeID_buffer[i] = ref_rhs.GetID().GetValue();
			}
			else if (void* udata = L.testudata(-1, type_name<UDRefl::SharedObject>().Data())) {
				const auto& rhs = *static_cast<UDRefl::SharedObject*>(udata);
				auto ref_rhs = rhs->AddLValueReference();
				argptr_buffer[i] = const_cast<void*>(ref_rhs.GetPtr());
				argTypeID_buffer[i] = ref_rhs.GetID().GetValue();
			}
			else if (!L.getmetatable(arg)) {
				argptr_buffer[i] = L.touserdata(arg);
				argTypeID_buffer[i] = TypeID_of<void*>.GetValue();
			}
			break;
		default:
			return L.error("%s::%s : The function doesn't support %s.",
				type_name<Functor>().Data(),
				MetaName::Data(),
				L.typename_(arg)
			);
		}
	}

	if constexpr (ExplictRet) {
		auto invocable_rst = ptr->IsInvocable(methodID, std::span<const TypeID>{reinterpret_cast<TypeID*>(argTypeID_buffer), static_cast<std::size_t>(argnum)});
		if (!invocable_rst || invocable_rst.result_desc.typeID != TypeID_of<Ret>) {
			return L.error("%s::%s : The function isn't invocable with arguments or it's return type isn't %s.",
				type_name<Functor>().Data(),
				MetaName::Data(),
				type_name<Ret>().Data()
			);
		}

		if constexpr (std::is_void_v<Ret>) {
			ptr->InvokeRet<void>(
				methodID,
				std::span<const TypeID>{reinterpret_cast<TypeID*>(argTypeID_buffer), static_cast<std::size_t>(argnum)},
				static_cast<UDRefl::ArgPtrBuffer>(argptr_buffer)
			);
			return 0;
		}
		else {
			Ret rst = ptr->InvokeRet<Ret>(
				methodID,
				std::span<const TypeID>{reinterpret_cast<TypeID*>(argTypeID_buffer), static_cast<std::size_t>(argnum)},
				static_cast<UDRefl::ArgPtrBuffer>(argptr_buffer)
			);

			details::push_rst<Ret>(L, std::move(rst));

			return 1;
		}
	}
	else {
		UDRefl::SharedObject rst = ptr->MInvoke(
			methodID,
			std::span<const TypeID>{reinterpret_cast<TypeID*>(argTypeID_buffer), static_cast<std::size_t>(argnum)},
			static_cast<UDRefl::ArgPtrBuffer>(argptr_buffer)
		);

		if (!rst.GetID().Valid()) {
			return L.error("%s::%s : The function isn't invocable with arguments.",
				type_name<Functor>().Data(),
				MetaName::Data()
			);
		}

		if (rst.GetID().Is<void>())
			return 0;

		void* buffer = L.newuserdata(sizeof(UDRefl::SharedObject));
		new(buffer)UDRefl::SharedObject{ std::move(rst) };
		L.getmetatable(type_name<UDRefl::SharedObject>().Data());
		L.setmetatable(-2);

		return 1;
	}
}

template<typename ID>
static int f_ID_new(lua_State* L_) {
	LuaStateView L{ L_ };
	int size = L.gettop();
	if (size == 0) {
		auto ptr = L.newuserdata(sizeof(ID));
		new (ptr) ID;
	}
	else if (size == 1) {
		int type = L.type(-1);
		switch (type)
		{
		case LUA_TNUMBER:
		{
			lua_Integer value = L.checkinteger(1);
			auto ptr = L.newuserdata(sizeof(ID));
			new (ptr) ID{ static_cast<std::size_t>(value) };
		}
			break;
		case LUA_TSTRING:
		{
			size_t len;
			const char* str = L.tolstring(1, &len);
			void* buffer = L.newuserdata(sizeof(ID));
			new (buffer) ID{ std::string_view{ str, len } };
		}
			break;
		default:
			return L.error("%s::new : The type of argument#1 is invalid. The function needs 0 argument / a string / a integer.", type_name<ID>().Data());
		}
	}
	else
		return L.error("%s::new : The number of arguments is invalid. The function needs 0 argument / a string / a integer.", type_name<ID>().Data());

	L.getmetatable(type_name<ID>().Data());
	L.setmetatable(-2);
	return 1;
}

template<typename Ptr>
static int f_Ptr_new(lua_State* L_) {
	LuaStateView L{ L_ };
	int size = L.gettop();
	if (size == 0) {
		auto ptr = L.newuserdata(sizeof(Ptr));
		new (ptr) Ptr;
	}
	else if (size == 1) {
		int type = L.type(-1);
		switch (type)
		{
		case LUA_TUSERDATA:
		{
			auto* id = (TypeID*)L.checkudata(1, type_name<TypeID>().Data());
			auto ptr = L.newuserdata(sizeof(Ptr));
			new (ptr) Ptr{ *id };
		}
		break;
		default:
			return L.error("%s::new : The type of argument#1 is invalid. The function needs 0 argument / a TypeID.", type_name<Ptr>().Data());
		}
	}
	else
		return L.error("%s::new : The number of arguments is invalid. The function needs 0 argument / a TypeID.", type_name<Ptr>().Data());

	L.getmetatable(type_name<Ptr>().Data());
	L.setmetatable(-2);
	return 1;
}

template<typename CPtr>
static int f_CPtr_AsNumber(lua_State* L_) {
	LuaStateView L{ L_ };

	if (L.gettop() != 1)
		return L.error("%s::AsNumber : The number of arguments is invalid. The function needs 1 argument (object).", type_name<CPtr>().Data());

	auto ptr = static_cast<UDRefl::ConstObjectPtr>(*(CPtr*)L.checkudata(1, type_name<CPtr>().Data()));

	if (!ptr.Valid())
		return L.error("%s::AsNumber : The object is nil.", type_name<CPtr>().Data());

	if (UDRefl::DereferenceProperty_IsReference(UDRefl::Mngr->GetDereferenceProperty(ptr.GetID())))
		ptr = ptr.DereferenceAsConst();

	std::string_view tname = ptr.TypeName();

	if (tname == type_name<bool>().View())
		L.pushboolean(ptr.As<bool>());
	else if (type_name_is_integral(tname)) {
		lua_Integer value;
		UDRefl::Ptr(value)->AInvoke<void>(UDRefl::StrIDRegistry::MetaID::ctor, ptr);
		L.pushinteger(value);
	}
	else if (type_name_is_floating_point(tname)) {
		lua_Number value;
		UDRefl::Ptr(value)->AInvoke<void>(UDRefl::StrIDRegistry::MetaID::ctor, ptr);
		L.pushnumber(value);
	}
	else {
		
		return L.error("%s::AsNumber : The type (%s) can't convert to a number.",
			type_name<CPtr>().Data(),
			tname.data());
	}

	return 1;
}

template<typename CPtr>
static int f_CPtr_tostring(lua_State* L_) {
	LuaStateView L{ L_ };

	if (L.gettop() != 1)
		return L.error("%s::__tostring : The number of arguments is invalid. The function needs 1 argument (object).", type_name<CPtr>().Data());

	auto ptr = static_cast<UDRefl::ConstObjectPtr>(*(CPtr*)L.checkudata(1, type_name<CPtr>().Data()));

	if (!ptr.Valid())
		return L.error("%s::__tostring : The object is nil.", type_name<CPtr>().Data());

	if (UDRefl::DereferenceProperty_IsReference(UDRefl::Mngr->GetDereferenceProperty(ptr.GetID())))
		ptr = ptr.DereferenceAsConst();

	if (!ptr.IsInvocable<std::stringstream&>(UDRefl::StrIDRegistry::MetaID::operator_rshift)) {
		std::string_view tname = UDRefl::Mngr->tregistry.Nameof(ptr.DereferenceAsConst().GetID());
		return L.error("%s::__tostring : The type (%s) can't convert to a string.",
			type_name<CPtr>().Data(),
			tname.data());
	}

	std::stringstream ss;
	ss << ptr;
	auto str = ss.str();

	L.pushlstring(str.data(), str.size());

	return 1;
}

template<typename Ptr>
static int f_Ptr_index(lua_State* L_) {
	LuaStateView L{ L_ };

	if (L.gettop() != 2)
		return L.error("%s::__index : The number of arguments is invalid. The function needs 2 argument (object + key).", type_name<Ptr>().Data());

	int type = L.type(2);

	StrID key_ID;
	const char* key = nullptr;
	switch (type)
	{
	case LUA_TUSERDATA:
	{
		if (auto* pID = (StrID*)L.testudata(2, type_name<StrID>().Data()))
			key_ID = *pID;
		else
			return f_meta<Ptr, details::Meta::t_index, UDRefl::StrIDRegistry::MetaID::operator_subscript.GetValue(), 2>(L_);

		break;
	}
	case LUA_TSTRING:
	{
		size_t len;
		key = L.checklstring(2, &len);
		key_ID = StrID{ std::string_view{key, len} };
		break;
	}
	default:
		return f_meta<Ptr, details::Meta::t_index, UDRefl::StrIDRegistry::MetaID::operator_subscript.GetValue(), 2>(L_);
	}
	
	const auto& ptr = *(Ptr*)L.checkudata(1, type_name<Ptr>().Data());

	if constexpr (std::is_same_v<Ptr, UDRefl::ObjectPtr> || std::is_same_v<Ptr, UDRefl::SharedObject>) {
		if (auto keyptr = ptr->RWVar(key_ID); keyptr.Valid()) {
			auto* buffer = L.newuserdata(sizeof(UDRefl::ObjectPtr));
			new(buffer)UDRefl::ObjectPtr{ keyptr };
			L.getmetatable(type_name<UDRefl::ObjectPtr>().Data());
			L.setmetatable(-2);
			return 1;
		}
	}
	
	if (auto keyptr = ptr->RVar(key_ID); keyptr.Valid()) {
		auto* buffer = L.newuserdata(sizeof(UDRefl::ConstObjectPtr));
		new(buffer)UDRefl::ConstObjectPtr{ keyptr };
		L.getmetatable(type_name<UDRefl::ConstObjectPtr>().Data());
		L.setmetatable(-2);
	}
	else if (ptr->ContainsMethod(key_ID)) {
		auto* buffer = L.newuserdata(sizeof(details::CallHandle));
		new(buffer)details::CallHandle{ ptr.GetID(), key_ID };
		L.getmetatable(type_name<details::CallHandle>().Data());
		L.setmetatable(-2);
	}
	else {
		if (!key) {
			key = UDRefl::Mngr->nregistry.Nameof(key_ID).data();
			if (!key) {
				std::string_view tname = UDRefl::Mngr->tregistry.Nameof(ptr->DereferenceAsConst().GetID());
				return L.error("%s::__index : %s can't index a non-var and non-method StrID, and ReflMngr's nregistry doesn't contains the StrID.",
					type_name<Ptr>().Data(),
					tname.data());
			}
		}

		int success = L.getmetatable(-2); // Ptr's metatable
		assert(success);
		int contain = L.getfield(-1, key);
		if (!contain) {
			std::string_view tname = UDRefl::Mngr->tregistry.Nameof(ptr->DereferenceAsConst().GetID());
			return L.error("%s::__index : %s not contain %s.",
				type_name<Ptr>().Data(),
				tname.data(),
				key);
		}
	}
	return 1;
}

template<typename Ptr>
static int f_Ptr_newindex(lua_State* L_) {
	LuaStateView L{ L_ };
	if (L.gettop() != 3) {
		return L.error("%s::__newindex : The number of arguments is invalid. The function needs 3 argument (object, key, value).",
			type_name<Ptr>().Data());
	}

	// stack : ptr, key, value
	L.getmetatable(type_name<UDRefl::ObjectPtr>().Data());
	L.getmetatable(type_name<Ptr>().Data());
	L.getfield(-2, details::CppMeta::op_assign.Data());
	L.getfield(-2, details::Meta::index.Data());
	L.rotate(1, -2);
	// stack : value, ..., __assign, __index, ptr, key
	{
		int error = L.pcall(2, 1, 0);
		if (error) {
			return L.error("%s::__newindex::__index: %s",
				type_name<Ptr>().Data(), L.tostring(-1));
		}
	}
	// stack : value, ..., __assign, __index result (member ptr)
	L.rotate(1, -1);
	// stack : ..., __assign, __index result (member ptr), value
	{
		int error = L.pcall(2, 1, 0);
		if (error) {
			return L.error("%s::__newindex::__assign: %s",
				type_name<Ptr>().Data(), L.tostring(-1));
		}
	}
	return 1;
}

static const struct luaL_Reg lib_StrID[] = {
	"new", f_ID_new<StrID>,
	NULL , NULL
};

static const struct luaL_Reg meta_StrID[] = {
	"GetValue", details::wrap<&StrID::GetValue, StrID>(TSTR("GetValue")),
	"Valid"   , details::wrap<&StrID::Valid,    StrID>(TSTR("Valid")),
	"Is"      , details::wrap<&StrID::Is,       StrID>(TSTR("Is")),
	NULL      , NULL
};

static const struct luaL_Reg lib_TypeID[] = {
	"new", f_ID_new<TypeID>,
	NULL , NULL
};

static const struct luaL_Reg meta_TypeID[] = {
	"GetValue", details::wrap<&TypeID::GetValue, TypeID>(TSTR("GetValue")),
	"Valid"   , details::wrap<&TypeID::Valid,    TypeID>(TSTR("Valid")),
	"Is"      , details::wrap<&IDBase::Is,       TypeID>(TSTR("Is")),
	NULL      , NULL
};

static const struct luaL_Reg lib_ConstObjectPtr[] = {
	"new"     , f_Ptr_new<UDRefl::ConstObjectPtr>,
	NULL      , NULL
};
static const struct luaL_Reg meta_ConstObjectPtr[] = {
	"GetID"   , details::wrap<&UDRefl::ConstObjectPtr::GetID,    UDRefl::ConstObjectPtr>(TSTR("GetID")),
	"Valid"   , details::wrap<&UDRefl::ConstObjectPtr::Valid,    UDRefl::ConstObjectPtr>(TSTR("Valid")),
	"GetType" , details::wrap<&UDRefl::ConstObjectPtr::GetType,  UDRefl::ConstObjectPtr>(TSTR("GetType")),
	"TypeName", details::wrap<&UDRefl::ConstObjectPtr::TypeName, UDRefl::ConstObjectPtr>(TSTR("TypeName")),
	"AsNumber", f_CPtr_AsNumber<UDRefl::ConstObjectPtr>,

	"__index", &f_Ptr_index<UDRefl::ConstObjectPtr>,
	"__tostring", &f_CPtr_tostring<UDRefl::ConstObjectPtr>,
	"__call",  &f_meta<UDRefl::ConstObjectPtr, details::Meta::t_call, UDRefl::StrIDRegistry::MetaID::operator_call.GetValue()>,
	"__add", &f_meta<UDRefl::ConstObjectPtr, details::Meta::t_add, UDRefl::StrIDRegistry::MetaID::operator_add.GetValue(), 2>,
	"__band", &f_meta<UDRefl::ConstObjectPtr, details::Meta::t_band, UDRefl::StrIDRegistry::MetaID::operator_band.GetValue(), 2>,
	"__bnot", &f_meta<UDRefl::ConstObjectPtr, details::Meta::t_bnot, UDRefl::StrIDRegistry::MetaID::operator_bnot.GetValue(), 1>,
	"__bor", &f_meta<UDRefl::ConstObjectPtr, details::Meta::t_bor, UDRefl::StrIDRegistry::MetaID::operator_bor.GetValue(), 2>,
	"__bxor", &f_meta<UDRefl::ConstObjectPtr, details::Meta::t_bxor, UDRefl::StrIDRegistry::MetaID::operator_bxor.GetValue(), 2>,
	"__div", &f_meta<UDRefl::ConstObjectPtr, details::Meta::t_div, UDRefl::StrIDRegistry::MetaID::operator_div.GetValue(), 2>,
	"__eq", &f_meta<UDRefl::ConstObjectPtr, details::Meta::t_eq, UDRefl::StrIDRegistry::MetaID::operator_eq.GetValue(), 2, true, bool>,
	"__le", &f_meta<UDRefl::ConstObjectPtr, details::Meta::t_le, UDRefl::StrIDRegistry::MetaID::operator_le.GetValue(), 2, true, bool>,
	"__lt", &f_meta<UDRefl::ConstObjectPtr, details::Meta::t_lt, UDRefl::StrIDRegistry::MetaID::operator_lt.GetValue(), 2, true, bool>,
	"__mod", &f_meta<UDRefl::ConstObjectPtr, details::Meta::t_mod, UDRefl::StrIDRegistry::MetaID::operator_mod.GetValue(), 2>,
	"__mul", &f_meta<UDRefl::ConstObjectPtr, details::Meta::t_mul, UDRefl::StrIDRegistry::MetaID::operator_mul.GetValue(), 2>,
	"__shl", &f_meta<UDRefl::ConstObjectPtr, details::Meta::t_shl, UDRefl::StrIDRegistry::MetaID::operator_lshift.GetValue(), 2>,
	"__shr", &f_meta<UDRefl::ConstObjectPtr, details::Meta::t_shr, UDRefl::StrIDRegistry::MetaID::operator_rshift.GetValue(), 2>,
	"__sub", &f_meta<UDRefl::ConstObjectPtr, details::Meta::t_sub, UDRefl::StrIDRegistry::MetaID::operator_sub.GetValue(), 2>,
	"__unm", &f_meta<UDRefl::ConstObjectPtr, details::Meta::t_unm, UDRefl::StrIDRegistry::MetaID::operator_minus.GetValue(), 2>,

	"__assign",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_op_assign, UDRefl::StrIDRegistry::MetaID::operator_assign.GetValue(), 2>,
	"__deref", &f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_op_deref, UDRefl::StrIDRegistry::MetaID::operator_deref.GetValue(), 1>,
	"__pre_inc", &f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_op_pre_inc, UDRefl::StrIDRegistry::MetaID::operator_pre_inc.GetValue(), 1>,
	"__pre_dec", &f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_op_pre_dec, UDRefl::StrIDRegistry::MetaID::operator_pre_dec.GetValue(), 1>,
	"__post_inc", &f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_op_post_inc, UDRefl::StrIDRegistry::MetaID::operator_post_inc.GetValue(), 1>,
	"__post_dec", &f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_op_post_dec, UDRefl::StrIDRegistry::MetaID::operator_post_dec.GetValue(), 1>,

	"tuple_size",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_tuple_size, UDRefl::StrIDRegistry::MetaID::tuple_size.GetValue(), 1, true, std::size_t>,
	"tuple_get",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_tuple_get, UDRefl::StrIDRegistry::MetaID::tuple_get.GetValue(), 2, true, UDRefl::ConstObjectPtr>,

	"begin",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_begin, UDRefl::StrIDRegistry::MetaID::container_begin.GetValue(), 1>,
	"cbegin",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_cbegin, UDRefl::StrIDRegistry::MetaID::container_cbegin.GetValue(), 1>,
	"end_",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_end, UDRefl::StrIDRegistry::MetaID::container_end.GetValue(), 1>,
	"cend",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_cend, UDRefl::StrIDRegistry::MetaID::container_cend.GetValue(), 1>,
	"rbegin",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_rbegin, UDRefl::StrIDRegistry::MetaID::container_rbegin.GetValue(), 1>,
	"crbegin",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_crbegin, UDRefl::StrIDRegistry::MetaID::container_crbegin.GetValue(), 1>,
	"rend",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_rend, UDRefl::StrIDRegistry::MetaID::container_rend.GetValue(), 1>,
	"crend",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_crend, UDRefl::StrIDRegistry::MetaID::container_crend.GetValue(), 1>,
	"at",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_at, UDRefl::StrIDRegistry::MetaID::container_at.GetValue(), 2>,
	"data",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_data, UDRefl::StrIDRegistry::MetaID::container_data.GetValue(), 1>,
	"front",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_front, UDRefl::StrIDRegistry::MetaID::container_front.GetValue(), 1>,
	"back",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_back, UDRefl::StrIDRegistry::MetaID::container_back.GetValue(), 1>,
	"empty",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_empty, UDRefl::StrIDRegistry::MetaID::container_empty.GetValue(), 1>,
	"size",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_size, UDRefl::StrIDRegistry::MetaID::container_size.GetValue(), 1>,
	"capacity",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_capacity, UDRefl::StrIDRegistry::MetaID::container_capacity.GetValue(), 1>,
	"bucket_count",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_bucket_count, UDRefl::StrIDRegistry::MetaID::container_bucket_count.GetValue(), 1>,
	"count",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_count, UDRefl::StrIDRegistry::MetaID::container_count.GetValue(), 2>,
	"find",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_find, UDRefl::StrIDRegistry::MetaID::container_find.GetValue(), 2>,
	"lower_bound",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_lower_bound, UDRefl::StrIDRegistry::MetaID::container_lower_bound.GetValue(), 2>,
	"upper_bound",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_upper_bound, UDRefl::StrIDRegistry::MetaID::container_upper_bound.GetValue(), 2>,
	"equal_range",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_equal_range, UDRefl::StrIDRegistry::MetaID::container_equal_range.GetValue(), 2>,
	"key_comp",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_key_comp, UDRefl::StrIDRegistry::MetaID::container_key_comp.GetValue(), 1>,
	"value_comp",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_value_comp, UDRefl::StrIDRegistry::MetaID::container_value_comp.GetValue(), 1>,
	"hash_function",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_hash_function, UDRefl::StrIDRegistry::MetaID::container_hash_function.GetValue(), 1>,
	"key_eq",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_key_eq, UDRefl::StrIDRegistry::MetaID::container_key_eq.GetValue(), 1>,
	"get_allocator",& f_meta<UDRefl::ConstObjectPtr, details::CppMeta::t_get_allocator, UDRefl::StrIDRegistry::MetaID::container_get_allocator.GetValue(), 1>,

	NULL      , NULL
};

static const struct luaL_Reg lib_ObjectPtr[] = {
	"new"     , f_Ptr_new<UDRefl::ObjectPtr>,

	NULL      , NULL
};
static const struct luaL_Reg meta_ObjectPtr[] = {
	"GetID"   , details::wrap<&UDRefl::ObjectPtr::GetID,    UDRefl::ObjectPtr>(TSTR("GetID")),
	"Valid"   , details::wrap<&UDRefl::ObjectPtr::Valid,    UDRefl::ObjectPtr>(TSTR("Valid")),
	"GetType" , details::wrap<&UDRefl::ObjectPtr::GetType,  UDRefl::ObjectPtr>(TSTR("GetType")),
	"TypeName", details::wrap<&UDRefl::ObjectPtr::TypeName, UDRefl::ObjectPtr>(TSTR("TypeName")),
	"AsNumber", f_CPtr_AsNumber<UDRefl::ObjectPtr>,

	"__index", &f_Ptr_index<UDRefl::ObjectPtr>,
	"__newindex",& f_Ptr_newindex<UDRefl::ObjectPtr>,
	"__tostring", &f_CPtr_tostring<UDRefl::ObjectPtr>,
	"__call", &f_meta<UDRefl::ObjectPtr, details::Meta::t_call, UDRefl::StrIDRegistry::MetaID::operator_call.GetValue()>,
	"__add", &f_meta<UDRefl::ObjectPtr, details::Meta::t_add, UDRefl::StrIDRegistry::MetaID::operator_add.GetValue(), 2>,
	"__band", &f_meta<UDRefl::ObjectPtr, details::Meta::t_band, UDRefl::StrIDRegistry::MetaID::operator_band.GetValue(), 2>,
	"__bnot", &f_meta<UDRefl::ObjectPtr, details::Meta::t_bnot, UDRefl::StrIDRegistry::MetaID::operator_bnot.GetValue(), 1>,
	"__bor", &f_meta<UDRefl::ObjectPtr, details::Meta::t_bor, UDRefl::StrIDRegistry::MetaID::operator_bor.GetValue(), 2>,
	"__div", &f_meta<UDRefl::ObjectPtr, details::Meta::t_div, UDRefl::StrIDRegistry::MetaID::operator_div.GetValue(), 2>,
	"__eq", &f_meta<UDRefl::ObjectPtr, details::Meta::t_eq, UDRefl::StrIDRegistry::MetaID::operator_eq.GetValue(), 2, true, bool>,
	"__le", &f_meta<UDRefl::ObjectPtr, details::Meta::t_le, UDRefl::StrIDRegistry::MetaID::operator_le.GetValue(), 2, true, bool>,
	"__lt", &f_meta<UDRefl::ObjectPtr, details::Meta::t_lt, UDRefl::StrIDRegistry::MetaID::operator_lt.GetValue(), 2, true, bool>,
	"__mod", &f_meta<UDRefl::ObjectPtr, details::Meta::t_mod, UDRefl::StrIDRegistry::MetaID::operator_mod.GetValue(), 2>,
	"__mul", &f_meta<UDRefl::ObjectPtr, details::Meta::t_mul, UDRefl::StrIDRegistry::MetaID::operator_mul.GetValue(), 2>,
	"__pow", &f_meta<UDRefl::ObjectPtr, details::Meta::t_pow, UDRefl::StrIDRegistry::MetaID::operator_bxor.GetValue(), 2>,
	"__shl", &f_meta<UDRefl::ObjectPtr, details::Meta::t_shl, UDRefl::StrIDRegistry::MetaID::operator_lshift.GetValue(), 2>,
	"__shr", &f_meta<UDRefl::ObjectPtr, details::Meta::t_shr, UDRefl::StrIDRegistry::MetaID::operator_rshift.GetValue(), 2>,
	"__sub", &f_meta<UDRefl::ObjectPtr, details::Meta::t_sub, UDRefl::StrIDRegistry::MetaID::operator_sub.GetValue(), 2>,
	"__unm", &f_meta<UDRefl::ObjectPtr, details::Meta::t_unm, UDRefl::StrIDRegistry::MetaID::operator_minus.GetValue(), 2>,

	"__assign",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_op_assign, UDRefl::StrIDRegistry::MetaID::operator_assign.GetValue(), 2>,
	"__deref", &f_meta<UDRefl::ObjectPtr, details::CppMeta::t_op_deref, UDRefl::StrIDRegistry::MetaID::operator_deref.GetValue(), 1>,
	"__pre_inc", &f_meta<UDRefl::ObjectPtr, details::CppMeta::t_op_pre_inc, UDRefl::StrIDRegistry::MetaID::operator_pre_inc.GetValue(), 1>,
	"__pre_dec", &f_meta<UDRefl::ObjectPtr, details::CppMeta::t_op_pre_dec, UDRefl::StrIDRegistry::MetaID::operator_pre_dec.GetValue(), 1>,
	"__post_inc", &f_meta<UDRefl::ObjectPtr, details::CppMeta::t_op_post_inc, UDRefl::StrIDRegistry::MetaID::operator_post_inc.GetValue(), 1>,
	"__post_dec", &f_meta<UDRefl::ObjectPtr, details::CppMeta::t_op_post_dec, UDRefl::StrIDRegistry::MetaID::operator_post_dec.GetValue(), 1>,

	"tuple_size",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_tuple_size, UDRefl::StrIDRegistry::MetaID::tuple_size.GetValue(), 1, true, std::size_t>,
	"tuple_get",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_tuple_get, UDRefl::StrIDRegistry::MetaID::tuple_get.GetValue(), 2, true, UDRefl::ObjectPtr>,

	"assign",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_assign, UDRefl::StrIDRegistry::MetaID::container_assign.GetValue()>,
	"begin",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_begin, UDRefl::StrIDRegistry::MetaID::container_begin.GetValue(), 1>,
	"cbegin",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_cbegin, UDRefl::StrIDRegistry::MetaID::container_cbegin.GetValue(), 1>,
	"end_",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_end, UDRefl::StrIDRegistry::MetaID::container_end.GetValue(), 1>,
	"cend",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_cend, UDRefl::StrIDRegistry::MetaID::container_cend.GetValue(), 1>,
	"rbegin",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_rbegin, UDRefl::StrIDRegistry::MetaID::container_rbegin.GetValue(), 1>,
	"crbegin",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_crbegin, UDRefl::StrIDRegistry::MetaID::container_crbegin.GetValue(), 1>,
	"rend",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_rend, UDRefl::StrIDRegistry::MetaID::container_rend.GetValue(), 1>,
	"crend",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_crend, UDRefl::StrIDRegistry::MetaID::container_crend.GetValue(), 1>,
	"at",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_at, UDRefl::StrIDRegistry::MetaID::container_at.GetValue(), 2>,
	"data",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_data, UDRefl::StrIDRegistry::MetaID::container_data.GetValue(), 1>,
	"front",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_front, UDRefl::StrIDRegistry::MetaID::container_front.GetValue(), 1>,
	"back",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_back, UDRefl::StrIDRegistry::MetaID::container_back.GetValue(), 1>,
	"empty",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_empty, UDRefl::StrIDRegistry::MetaID::container_empty.GetValue(), 1>,
	"size",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_size, UDRefl::StrIDRegistry::MetaID::container_size.GetValue(), 1>,
	"max_size",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_max_size, UDRefl::StrIDRegistry::MetaID::container_max_size.GetValue(), 1>,
	"resize",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_resize, UDRefl::StrIDRegistry::MetaID::container_resize.GetValue(), 1>,
	"capacity",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_capacity, UDRefl::StrIDRegistry::MetaID::container_capacity.GetValue(), 1>,
	"bucket_count",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_bucket_count, UDRefl::StrIDRegistry::MetaID::container_bucket_count.GetValue(), 1>,
	"reserve",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_reserve, UDRefl::StrIDRegistry::MetaID::container_reserve.GetValue(), 2>,
	"shrink_to_fit",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_shrink_to_fit, UDRefl::StrIDRegistry::MetaID::container_shrink_to_fit.GetValue(), 1>,
	"clear",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_clear, UDRefl::StrIDRegistry::MetaID::container_clear.GetValue(), 1>,
	"insert",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_insert, UDRefl::StrIDRegistry::MetaID::container_insert.GetValue()>,
	"insert_or_assign",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_insert_or_assign, UDRefl::StrIDRegistry::MetaID::container_insert_or_assign.GetValue()>,
	"erase",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_erase, UDRefl::StrIDRegistry::MetaID::container_erase.GetValue()>,
	"push_front",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_push_front, UDRefl::StrIDRegistry::MetaID::container_push_front.GetValue(), 2>,
	"pop_front",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_pop_front, UDRefl::StrIDRegistry::MetaID::container_pop_front.GetValue(), 2>,
	"push_back",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_push_back, UDRefl::StrIDRegistry::MetaID::container_push_back.GetValue(), 2>,
	"pop_back",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_pop_back, UDRefl::StrIDRegistry::MetaID::container_pop_back.GetValue(), 2>,
	"swap",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_swap, UDRefl::StrIDRegistry::MetaID::container_swap.GetValue(), 2>,
	"merge",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_merge, UDRefl::StrIDRegistry::MetaID::container_merge.GetValue(), 2>,
	"extract",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_extract, UDRefl::StrIDRegistry::MetaID::container_extract.GetValue(), 2>,
	"count",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_count, UDRefl::StrIDRegistry::MetaID::container_count.GetValue(), 2>,
	"find",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_find, UDRefl::StrIDRegistry::MetaID::container_find.GetValue(), 2>,
	"lower_bound",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_lower_bound, UDRefl::StrIDRegistry::MetaID::container_lower_bound.GetValue(), 2>,
	"upper_bound",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_upper_bound, UDRefl::StrIDRegistry::MetaID::container_upper_bound.GetValue(), 2>,
	"equal_range",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_equal_range, UDRefl::StrIDRegistry::MetaID::container_equal_range.GetValue(), 2>,
	"key_comp",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_key_comp, UDRefl::StrIDRegistry::MetaID::container_key_comp.GetValue(), 1>,
	"value_comp",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_value_comp, UDRefl::StrIDRegistry::MetaID::container_value_comp.GetValue(), 1>,
	"hash_function",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_hash_function, UDRefl::StrIDRegistry::MetaID::container_hash_function.GetValue(), 1>,
	"key_eq",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_key_eq, UDRefl::StrIDRegistry::MetaID::container_key_eq.GetValue(), 1>,
	"get_allocator",& f_meta<UDRefl::ObjectPtr, details::CppMeta::t_get_allocator, UDRefl::StrIDRegistry::MetaID::container_get_allocator.GetValue(), 1>,

	NULL      , NULL
};

static const struct luaL_Reg lib_SharedConstObject[] = {
	"new", f_Ptr_new<UDRefl::SharedConstObject>,
	NULL , NULL
};
static const struct luaL_Reg meta_SharedConstObject[] = {
	"GetID"      , details::wrap<&UDRefl::SharedConstObject::GetID,       UDRefl::SharedConstObject>(TSTR("GetID")),
	"Valid"      , details::wrap<&UDRefl::SharedConstObject::Valid,       UDRefl::SharedConstObject>(TSTR("Valid")),
	"AsObjectPtr", details::wrap<&UDRefl::SharedConstObject::AsObjectPtr, UDRefl::SharedConstObject>(TSTR("GetType")),
	"AsNumber"   , f_CPtr_AsNumber<UDRefl::SharedConstObject>,

	"__gc", details::wrap_dtor<UDRefl::SharedConstObject>(),
	"__index", &f_Ptr_index<UDRefl::SharedConstObject>,
	"__tostring", &f_CPtr_tostring<UDRefl::SharedConstObject>,
	"__call", &f_meta<UDRefl::SharedConstObject, details::Meta::t_call, UDRefl::StrIDRegistry::MetaID::operator_call.GetValue()>,
	"__add", &f_meta<UDRefl::SharedConstObject, details::Meta::t_add, UDRefl::StrIDRegistry::MetaID::operator_add.GetValue(), 2>,
	"__band",&f_meta<UDRefl::SharedConstObject, details::Meta::t_band, UDRefl::StrIDRegistry::MetaID::operator_band.GetValue(), 2>,
	"__bnot",&f_meta<UDRefl::SharedConstObject, details::Meta::t_bnot, UDRefl::StrIDRegistry::MetaID::operator_bnot.GetValue(), 1>,
	"__bor",&f_meta<UDRefl::SharedConstObject, details::Meta::t_bor, UDRefl::StrIDRegistry::MetaID::operator_bor.GetValue(), 2>,
	"__div",&f_meta<UDRefl::SharedConstObject, details::Meta::t_div, UDRefl::StrIDRegistry::MetaID::operator_div.GetValue(), 2>,
	"__eq",&f_meta<UDRefl::SharedConstObject, details::Meta::t_eq, UDRefl::StrIDRegistry::MetaID::operator_eq.GetValue(), 2, true, bool>,
	"__le",&f_meta<UDRefl::SharedConstObject, details::Meta::t_le, UDRefl::StrIDRegistry::MetaID::operator_le.GetValue(), 2, true, bool>,
	"__lt",&f_meta<UDRefl::SharedConstObject, details::Meta::t_lt, UDRefl::StrIDRegistry::MetaID::operator_lt.GetValue(), 2, true, bool>,
	"__mod",&f_meta<UDRefl::SharedConstObject, details::Meta::t_mod, UDRefl::StrIDRegistry::MetaID::operator_mod.GetValue(), 2>,
	"__mul",&f_meta<UDRefl::SharedConstObject, details::Meta::t_mul, UDRefl::StrIDRegistry::MetaID::operator_mul.GetValue(), 2>,
	"__pow", &f_meta<UDRefl::SharedConstObject, details::Meta::t_pow, UDRefl::StrIDRegistry::MetaID::operator_bxor.GetValue(), 2>,
	"__shl",&f_meta<UDRefl::SharedConstObject, details::Meta::t_shl, UDRefl::StrIDRegistry::MetaID::operator_lshift.GetValue(), 2>,
	"__shr",&f_meta<UDRefl::SharedConstObject, details::Meta::t_shr, UDRefl::StrIDRegistry::MetaID::operator_rshift.GetValue(), 2>,
	"__sub",&f_meta<UDRefl::SharedConstObject, details::Meta::t_sub, UDRefl::StrIDRegistry::MetaID::operator_sub.GetValue(), 2>,
	"__unm", &f_meta<UDRefl::SharedConstObject, details::Meta::t_unm, UDRefl::StrIDRegistry::MetaID::operator_minus.GetValue(), 2>,

	"__assign",& f_meta<UDRefl::SharedConstObject, details::CppMeta::t_op_assign, UDRefl::StrIDRegistry::MetaID::operator_assign.GetValue(), 2>,
	"__deref", &f_meta<UDRefl::SharedConstObject, details::CppMeta::t_op_deref, UDRefl::StrIDRegistry::MetaID::operator_deref.GetValue(), 1>,
	"__pre_inc", &f_meta<UDRefl::SharedConstObject, details::CppMeta::t_op_pre_inc, UDRefl::StrIDRegistry::MetaID::operator_pre_inc.GetValue(), 1>,
	"__pre_dec", &f_meta<UDRefl::SharedConstObject, details::CppMeta::t_op_pre_dec, UDRefl::StrIDRegistry::MetaID::operator_pre_dec.GetValue(), 1>,
	"__post_inc", &f_meta<UDRefl::SharedConstObject, details::CppMeta::t_op_post_inc, UDRefl::StrIDRegistry::MetaID::operator_post_inc.GetValue(), 1>,
	"__post_dec", &f_meta<UDRefl::SharedConstObject, details::CppMeta::t_op_post_dec, UDRefl::StrIDRegistry::MetaID::operator_post_dec.GetValue(), 1>,

	NULL         , NULL
};

static const struct luaL_Reg lib_SharedObject[] = {
	"new"                , f_Ptr_new<UDRefl::SharedObject>,

	NULL                 , NULL
};
static const struct luaL_Reg meta_SharedObject[] = {
	"GetID"              , details::wrap<&UDRefl::SharedObject::GetID,               UDRefl::SharedObject>(TSTR("GetID")),
	"Valid"              , details::wrap<&UDRefl::SharedObject::Valid,               UDRefl::SharedObject>(TSTR("Valid")),
	"AsSharedConstObject", details::wrap<&UDRefl::SharedObject::AsSharedConstObject, UDRefl::SharedObject>(TSTR("AsSharedConstObject")),
	"AsObjectPtr"        , details::wrap<&UDRefl::SharedObject::AsObjectPtr,         UDRefl::SharedObject>(TSTR("AsObjectPtr")),
	"AsConstObjectPtr"   , details::wrap<&UDRefl::SharedObject::AsConstObjectPtr,    UDRefl::SharedObject>(TSTR("AsConstObjectPtr")),
	"AsNumber"           , f_CPtr_AsNumber<UDRefl::SharedObject>,

	"__gc", details::wrap_dtor<UDRefl::SharedObject>(),
	"__index", &f_Ptr_index<UDRefl::SharedObject>,
	"__newindex",&f_Ptr_newindex<UDRefl::SharedObject>,
	"__tostring", &f_CPtr_tostring<UDRefl::SharedObject>,
	"__call", &f_meta<UDRefl::SharedObject, details::Meta::t_call, UDRefl::StrIDRegistry::MetaID::operator_call.GetValue()>,
	"__add", &f_meta<UDRefl::SharedObject, details::Meta::t_add, UDRefl::StrIDRegistry::MetaID::operator_add.GetValue(), 2>,
	"__band",&f_meta<UDRefl::SharedObject, details::Meta::t_band, UDRefl::StrIDRegistry::MetaID::operator_band.GetValue(), 2>,
	"__bnot",&f_meta<UDRefl::SharedObject, details::Meta::t_bnot, UDRefl::StrIDRegistry::MetaID::operator_bnot.GetValue(), 1>,
	"__bor",&f_meta<UDRefl::SharedObject, details::Meta::t_bor, UDRefl::StrIDRegistry::MetaID::operator_bor.GetValue(), 2>,
	"__div",&f_meta<UDRefl::SharedObject, details::Meta::t_div, UDRefl::StrIDRegistry::MetaID::operator_div.GetValue(), 2>,
	"__eq",&f_meta<UDRefl::SharedObject, details::Meta::t_eq, UDRefl::StrIDRegistry::MetaID::operator_eq.GetValue(), 2, true, bool>,
	"__le",&f_meta<UDRefl::SharedObject, details::Meta::t_le, UDRefl::StrIDRegistry::MetaID::operator_le.GetValue(), 2, true, bool>,
	"__lt",&f_meta<UDRefl::SharedObject, details::Meta::t_lt, UDRefl::StrIDRegistry::MetaID::operator_lt.GetValue(), 2, true, bool>,
	"__mod",&f_meta<UDRefl::SharedObject, details::Meta::t_mod, UDRefl::StrIDRegistry::MetaID::operator_mod.GetValue(), 2>,
	"__mul",&f_meta<UDRefl::SharedObject, details::Meta::t_mul, UDRefl::StrIDRegistry::MetaID::operator_mul.GetValue(), 2>,
	"__pow", &f_meta<UDRefl::SharedObject, details::Meta::t_pow, UDRefl::StrIDRegistry::MetaID::operator_bxor.GetValue(), 2>,
	"__shl",&f_meta<UDRefl::SharedObject, details::Meta::t_shl, UDRefl::StrIDRegistry::MetaID::operator_lshift.GetValue(), 2>,
	"__shr",&f_meta<UDRefl::SharedObject, details::Meta::t_shr, UDRefl::StrIDRegistry::MetaID::operator_rshift.GetValue(), 2>,
	"__sub",&f_meta<UDRefl::SharedObject, details::Meta::t_sub, UDRefl::StrIDRegistry::MetaID::operator_sub.GetValue(), 2>,
	"__unm", &f_meta<UDRefl::SharedObject, details::Meta::t_unm, UDRefl::StrIDRegistry::MetaID::operator_minus.GetValue(), 2>,

	"__assign",& f_meta<UDRefl::SharedObject, details::CppMeta::t_op_assign, UDRefl::StrIDRegistry::MetaID::operator_assign.GetValue(), 2>,
	"__deref", &f_meta<UDRefl::SharedObject, details::CppMeta::t_op_deref, UDRefl::StrIDRegistry::MetaID::operator_deref.GetValue(), 1>,
	"__pre_inc", &f_meta<UDRefl::SharedObject, details::CppMeta::t_op_pre_inc, UDRefl::StrIDRegistry::MetaID::operator_pre_inc.GetValue(), 1>,
	"__pre_dec", &f_meta<UDRefl::SharedObject, details::CppMeta::t_op_pre_dec, UDRefl::StrIDRegistry::MetaID::operator_pre_dec.GetValue(), 1>,
	"__post_inc", &f_meta<UDRefl::SharedObject, details::CppMeta::t_op_post_inc, UDRefl::StrIDRegistry::MetaID::operator_post_inc.GetValue(), 1>,
	"__post_dec", &f_meta<UDRefl::SharedObject, details::CppMeta::t_op_post_dec, UDRefl::StrIDRegistry::MetaID::operator_post_dec.GetValue(), 1>,

	NULL                 , NULL
};

static int f_ReflMngr_MakeShared(lua_State* L_) {
	LuaStateView L{ L_ };
	const int L_argnum = L.gettop();
	if (L_argnum <= 0)
		return L.error("%s::MakeShared : The number of arguments is invalid.", type_name<UDRefl::ReflMngr>().Data());

	TypeID ID;
	int type = L.type(1);
	switch (type)
	{
	case LUA_TSTRING:
		ID = TypeID{ details::get_arg<std::string_view>(L, 1) };
		break;
	case LUA_TNUMBER:
		ID = TypeID{ static_cast<std::size_t>(L.checkinteger(1)) };
		break;
	case LUA_TUSERDATA:
		ID = *static_cast<TypeID*>(L.checkudata(1, type_name<TypeID>().Data()));
		break;
	default:
		break;
	}

	const int argnum = L_argnum - 1;

	if (argnum > UDRefl::MaxArgNum) {
		return L.error("%s::MakeShared : The number of arguments (%d) is greater than UDRefl::MaxArgNum (%d).",
			type_name<UDRefl::ReflMngr>().Data(), argnum, static_cast<lua_Integer>(UDRefl::MaxArgNum));
	}

	void* argptr_buffer[UDRefl::MaxArgNum];
	std::size_t argTypeID_buffer[UDRefl::MaxArgNum];
	std::uint64_t copied_args_buffer[UDRefl::MaxArgNum];
	std::size_t num_copied_args = 0;

	for (std::size_t i{ 0 }; i < argnum; i++) {
		int arg = static_cast<int>(i) + L_argnum - argnum + 1;
		int type = L.type(arg);
		switch (type)
		{
		case LUA_TBOOLEAN:
		{
			auto arg_buffer = &copied_args_buffer[num_copied_args++];
			argptr_buffer[i] = arg_buffer;
			argTypeID_buffer[i] = TypeID_of<bool>.GetValue();
			UDRefl::buffer_as<bool>(arg_buffer) = static_cast<bool>(L.toboolean(arg));
		}
		break;
		case LUA_TNUMBER:
			if (L.isinteger(arg)) {
				auto arg_buffer = &copied_args_buffer[num_copied_args++];
				argptr_buffer[i] = arg_buffer;
				argTypeID_buffer[i] = TypeID_of<lua_Integer>.GetValue();
				UDRefl::buffer_as<lua_Integer>(arg_buffer) = static_cast<lua_Integer>(L.tointeger(arg));
			}
			else if (L.isnumber(arg)) {
				auto arg_buffer = &copied_args_buffer[num_copied_args++];
				argptr_buffer[i] = arg_buffer;
				argTypeID_buffer[i] = TypeID_of<lua_Number>.GetValue();
				UDRefl::buffer_as<lua_Number>(arg_buffer) = static_cast<lua_Number>(L.tonumber(arg));
			}
			else
				assert(false);
			break;
		case LUA_TSTRING:
			assert(false); // TODO
			break;
		case LUA_TUSERDATA:
			if (void* udata = L.testudata(-1, type_name<UDRefl::ConstObjectPtr>().Data())) {
				const auto& rhs = *static_cast<UDRefl::ConstObjectPtr*>(udata);
				auto ref_rhs = rhs.AddConstLValueReference();
				argptr_buffer[i] = const_cast<void*>(ref_rhs.GetPtr());
				argTypeID_buffer[i] = ref_rhs.GetID().GetValue();
			}
			else if (void* udata = L.testudata(-1, type_name<UDRefl::ObjectPtr>().Data())) {
				const auto& rhs = *static_cast<UDRefl::ObjectPtr*>(udata);
				auto ref_rhs = rhs.AddLValueReference();
				argptr_buffer[i] = const_cast<void*>(ref_rhs.GetPtr());
				argTypeID_buffer[i] = ref_rhs.GetID().GetValue();
			}
			else if (void* udata = L.testudata(-1, type_name<UDRefl::SharedConstObject>().Data())) {
				const auto& rhs = *static_cast<UDRefl::SharedConstObject*>(udata);
				auto ref_rhs = rhs->AddConstLValueReference();
				argptr_buffer[i] = const_cast<void*>(ref_rhs.GetPtr());
				argTypeID_buffer[i] = ref_rhs.GetID().GetValue();
			}
			else if (void* udata = L.testudata(-1, type_name<UDRefl::SharedObject>().Data())) {
				const auto& rhs = *static_cast<UDRefl::SharedObject*>(udata);
				auto ref_rhs = rhs->AddLValueReference();
				argptr_buffer[i] = const_cast<void*>(ref_rhs.GetPtr());
				argTypeID_buffer[i] = ref_rhs.GetID().GetValue();
			}
			else if (!L.getmetatable(arg)) {
				argptr_buffer[i] = L.touserdata(arg);
				argTypeID_buffer[i] = TypeID_of<void*>.GetValue();
			}
			break;
		default:
			break;
		}
	}

	UDRefl::SharedObject obj = UDRefl::Mngr->MakeShared(
		ID,
		std::span<const TypeID>{reinterpret_cast<TypeID*>(argTypeID_buffer), static_cast<std::size_t>(argnum)},
		static_cast<UDRefl::ArgPtrBuffer>(argptr_buffer)
	);

	if (!obj.Valid())
		return L.error("%s::MakeShared : Fail.", type_name<UDRefl::ReflMngr>().Data());
	
	auto* buffer = L.newuserdata(sizeof(UDRefl::SharedObject));
	new(buffer)UDRefl::SharedObject{ std::move(obj) };

	L.getmetatable(type_name<UDRefl::SharedObject>().Data());
	L.setmetatable(-2);

	return 1;
}

static const struct luaL_Reg lib_ReflMngr[] = {
	"MakeShared", f_ReflMngr_MakeShared,
	NULL        , NULL
};

static void init_CallHandle(lua_State* L_) {
	LuaStateView L{ L_ };

	L.newmetatable(type_name<details::CallHandle>().Data());
	L.pushcfunction(&f_meta<details::CallHandle, details::Meta::t_call>);
	L.setfield(-2, "__call");

	L.pop(1);
}

static int luaopen_StrID(lua_State* L_) {
	LuaStateView L{ L_ };
	L.newmetatable(type_name<StrID>().Data());
	L.pushvalue(-1); // duplicate the metatable
	L.setfield(-2, "__index"); // mt.__index = mt
	L.setfuncs(lib_StrID, 0);
	L.newlib(lib_StrID);
	return 1;
}

static int luaopen_TypeID(lua_State* L_) {
	LuaStateView L{ L_ };
	L.newmetatable(type_name<TypeID>().Data());
	L.pushvalue(-1); // duplicate the metatable
	L.setfield(-2, "__index"); // mt.__index = mt
	L.setfuncs(lib_TypeID, 0);
	L.newlib(lib_TypeID);
	return 1;
}

static int luaopen_ConstObjectPtr(lua_State* L_) {
	LuaStateView L{ L_ };
	L.newmetatable(type_name<UDRefl::ConstObjectPtr>().Data());
	L.setfuncs(meta_ConstObjectPtr, 0);
	L.newlib(lib_ConstObjectPtr);
	return 1;
}

static int luaopen_ObjectPtr(lua_State* L_) {
	LuaStateView L{ L_ };
	L.newmetatable(type_name<UDRefl::ObjectPtr>().Data());
	L.setfuncs(meta_ObjectPtr, 0);
	L.newlib(lib_ObjectPtr);
	return 1;
}

static int luaopen_SharedConstObject(lua_State* L_) {
	LuaStateView L{ L_ };
	L.newmetatable(type_name<UDRefl::SharedConstObject>().Data());
	L.setfuncs(meta_SharedConstObject, 0);
	L.newlib(lib_SharedConstObject);
	return 1;
}

static int luaopen_SharedObject(lua_State* L_) {
	LuaStateView L{ L_ };
	L.newmetatable(type_name<UDRefl::SharedObject>().Data());
	L.setfuncs(meta_SharedObject, 0);
	L.newlib(lib_SharedObject);
	return 1;
}

static int luaopen_ReflMngr(lua_State* L_) {
	LuaStateView L{ L_ };
	L.newlib(lib_ReflMngr);
	return 1;
}

static const luaL_Reg UDRefl_libs[] = {
  {"StrID"            , luaopen_StrID},
  {"TypeID"           , luaopen_TypeID},
  {details::LibName<UDRefl::ConstObjectPtr   >::value, luaopen_ConstObjectPtr},
  {details::LibName<UDRefl::ObjectPtr        >::value, luaopen_ObjectPtr},
  {details::LibName<UDRefl::SharedConstObject>::value, luaopen_SharedConstObject},
  {details::LibName<UDRefl::SharedObject     >::value, luaopen_SharedObject},
  {"ReflMngr"         , luaopen_ReflMngr},
  {NULL               , NULL}
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
