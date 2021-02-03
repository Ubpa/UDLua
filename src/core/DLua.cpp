#include <UDLua/DLua.h>

#include <ULuaW/LuaStateView.h>

#include <UDRefl/UDRefl.h>

#include <lua.hpp>

using namespace Ubpa;

namespace Ubpa::details {
	namespace Meta {
		static constexpr auto add = TSTR("add");
		static constexpr auto band = TSTR("band");
		static constexpr auto bnot = TSTR("bnot");
		static constexpr auto bor = TSTR("bor");
		static constexpr auto bxor = TSTR("bxor");
		static constexpr auto call = TSTR("call");
		static constexpr auto concat = TSTR("concat");
		static constexpr auto div = TSTR("div");
		static constexpr auto eq = TSTR("eq");
		static constexpr auto gc = TSTR("gc");
		static constexpr auto idiv = TSTR("idiv");
		static constexpr auto index = TSTR("index");
		static constexpr auto le = TSTR("le");
		static constexpr auto len = TSTR("len");
		static constexpr auto lt = TSTR("lt");
		static constexpr auto metatable = TSTR("metatable");
		static constexpr auto mod = TSTR("mod");
		static constexpr auto mode = TSTR("mode");
		static constexpr auto mul = TSTR("mul");
		static constexpr auto name = TSTR("name");
		static constexpr auto newindex = TSTR("newindex");
		static constexpr auto pairs = TSTR("pairs");
		static constexpr auto pow = TSTR("pow");
		static constexpr auto shl = TSTR("shl");
		static constexpr auto shr = TSTR("shr");
		static constexpr auto sub = TSTR("sub");
		static constexpr auto tostring = TSTR("tostring");
		static constexpr auto unm = TSTR("unm");

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
		static constexpr auto deref = TSTR("deref");
		static constexpr auto pre_inc = TSTR("pre_inc");

		using t_deref = decltype(deref);
		using t_pre_inc = decltype(pre_inc);
	}
}

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
			auto* buffer = L.newuserdata(sizeof(Ret*));
			new (buffer) Ret{ std::move(rst) };
			if (!L.getmetatable(type_name<Ret>().Data())) {
				assert(L.newmetatable(type_name<Ret>().Data()));
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
			LuaStateView L(L_);
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

template<typename ID>
static int f_ID_new(lua_State* L_) {
	LuaStateView L(L_);
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
	LuaStateView L(L_);
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
	LuaStateView L(L_);

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
static int f_CPtr_index(lua_State* L_) {
	LuaStateView L(L_);

	if (L.gettop() != 2)
		return L.error("%s::__index : The number of arguments is invalid. The function needs 2 argument (object + key).", type_name<CPtr>().Data());

	auto ptr = static_cast<UDRefl::ConstObjectPtr>(*(CPtr*)L.checkudata(1, type_name<CPtr>().Data()));

	size_t len;
	const char* key = L.checklstring(2, &len);
	StrID key_ID{ key };

	if (auto keyptr = ptr->RVar(key_ID); keyptr.Valid()) {
		auto* buffer = L.newuserdata(sizeof(UDRefl::ConstObjectPtr));
		new(buffer)UDRefl::ConstObjectPtr{ keyptr };
		L.getmetatable(type_name<UDRefl::ConstObjectPtr>().Data());
		L.setmetatable(-2);
	}
	else if (ptr.ContainsMethod(key_ID)) {
		auto* buffer = L.newuserdata(sizeof(details::CallHandle));
		new(buffer)details::CallHandle{ ptr.GetID(), key_ID };
		L.getmetatable(type_name<details::CallHandle>().Data());
		L.setmetatable(-2);
	}
	else {
		assert(L.gettop() == 2);
		int success = L.getmetatable(-2); // CPtr's metatable
		assert(success);
		int contain = L.getfield(-1, key);
		if (!contain) {
			std::string_view tname = UDRefl::Mngr->tregistry.Nameof(ptr.DereferenceAsConst().GetID());
			return L.error("%s::__index : %s not contain %s.",
				type_name<CPtr>().Data(),
				tname.data(),
				key);
		}
	}
	return 1;
}

template<typename Ptr>
static int f_Ptr_index(lua_State* L_) {
	LuaStateView L(L_);

	if (L.gettop() != 2)
		return L.error("%s::__index : The number of arguments is invalid. The function needs 2 argument (object + key).", type_name<Ptr>().Data());

	auto ptr = static_cast<UDRefl::ObjectPtr>(*(Ptr*)L.checkudata(1, type_name<Ptr>().Data()));

	size_t len;
	const char* key = L.checklstring(2, &len);
	StrID key_ID{ std::string_view{key, len} };

	if (auto keyptr = ptr.RWVar(key_ID); keyptr.Valid()) {
		auto* buffer = L.newuserdata(sizeof(UDRefl::ObjectPtr));
		new(buffer)UDRefl::ObjectPtr{ keyptr };
		L.getmetatable(type_name<UDRefl::ObjectPtr>().Data());
		L.setmetatable(-2);
	}
	else if (auto keyptr = ptr.RVar(key_ID); keyptr.Valid()) {
		auto* buffer = L.newuserdata(sizeof(UDRefl::ConstObjectPtr));
		new(buffer)UDRefl::ConstObjectPtr{ keyptr };
		L.getmetatable(type_name<UDRefl::ConstObjectPtr>().Data());
		L.setmetatable(-2);
	}
	else if (ptr.ContainsMethod(key_ID)) {
		auto* buffer = L.newuserdata(sizeof(details::CallHandle));
		new(buffer)details::CallHandle{ ptr.GetID(), key_ID };
		L.getmetatable(type_name<details::CallHandle>().Data());
		L.setmetatable(-2);
	}
	else {
		assert(L.gettop() == 2);
		int success = L.getmetatable(-2); // Ptr's metatable
		assert(success);
		int contain = L.getfield(-1, key);
		if (!contain) {
			std::string_view tname = UDRefl::Mngr->tregistry.Nameof(ptr.DereferenceAsConst().GetID());
			return L.error("%s::__index : %s not contain %s.",
				type_name<Ptr>().Data(),
				tname.data(),
				key);
		}
	}
	return 1;
}

template<typename CPtr>
static int f_CPtr_tostring(lua_State* L_) {
	LuaStateView L(L_);

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
static int f_Ptr_newindex(lua_State* L_) {
	LuaStateView L(L_);

	if (L.gettop() != 3) {
		return L.error("%s::__newindex : The number of arguments is invalid. The function needs 3 argument (object, key, value).",
			type_name<Ptr>().Data());
	}

	auto ptr = static_cast<UDRefl::ObjectPtr>(*(Ptr*)L.checkudata(1, type_name<Ptr>().Data()));

	if (!ptr.Valid())
		return L.error("%s::__newindex : The object is nil.", type_name<Ptr>().Data());
	
	size_t len;
	const char* key = L.checklstring(2, &len);
	std::string_view key_sv{ key, len };
	UDRefl::ObjectPtr member = key_sv == "this" ? ptr : ptr.RWVar(key_sv);

	if (!member.Valid()) {
		std::string_view tname = UDRefl::Mngr->tregistry.Nameof(ptr.DereferenceAsConst().GetID());
		return L.error("%s::__newindex : %s not contain writable %s.",
			type_name<Ptr>().Data(),
			tname.data(),
			key);
	}

	int type = L.type(-1);
	switch (type)
	{
	case LUA_TBOOLEAN:
		if (!member.IsInvocable<const bool&>(UDRefl::StrIDRegistry::MetaID::operator_assign)) {
			std::string_view tname = UDRefl::Mngr->tregistry.Nameof(ptr.DereferenceAsConst().GetID());
			return L.error("%s::__newindex : %s::%s can't be assign with bool.",
				type_name<Ptr>().Data(),
				tname.data(),
				key);
		}
		member = static_cast<const bool&>(L.toboolean(-1));
		break;
	case LUA_TNUMBER:
		if (L.isinteger(-1)) {
			if (!member.IsInvocable<const lua_Integer&>(UDRefl::StrIDRegistry::MetaID::operator_assign)) {
				std::string_view tname = UDRefl::Mngr->tregistry.Nameof(ptr.DereferenceAsConst().GetID());
				return L.error("%s::__newindex : %s::%s can't be assign with lua_Integer.",
					type_name<Ptr>().Data(),
					tname.data(),
					key);
			}
			member = static_cast<const lua_Integer&>(L.tointeger(-1));
		}
		else if (L.isnumber(-1)) {
			if (!member.IsInvocable<const lua_Number&>(UDRefl::StrIDRegistry::MetaID::operator_assign)) {
				std::string_view tname = UDRefl::Mngr->tregistry.Nameof(ptr.DereferenceAsConst().GetID());
				return L.error("%s::__newindex : %s::%s can't be assign with lua_Number.",
					type_name<Ptr>().Data(),
					tname.data(),
					key);
			}
			member = static_cast<const lua_Number&>(L.tonumber(-1));
		}
		else
			assert(false);
		break;
	case LUA_TUSERDATA:
		if (void* udata = L.testudata(-1, type_name<UDRefl::ConstObjectPtr>().Data())) {
			const auto& rhs = *static_cast<UDRefl::ConstObjectPtr*>(udata);
			member.AInvoke<void>(UDRefl::StrIDRegistry::MetaID::operator_assign, rhs);
		}
		else if (void* udata = L.testudata(-1, type_name<UDRefl::ObjectPtr>().Data())) {
			const auto& rhs = *static_cast<UDRefl::ObjectPtr*>(udata);
			member.AInvoke<void>(UDRefl::StrIDRegistry::MetaID::operator_assign, rhs);
		}
		else if (void* udata = L.testudata(-1, type_name<UDRefl::SharedConstObject>().Data())) {
			const auto& rhs = *static_cast<UDRefl::SharedConstObject*>(udata);
			member.AInvoke<void>(UDRefl::StrIDRegistry::MetaID::operator_assign, rhs);
		}
		else if (void* udata = L.testudata(-1, type_name<UDRefl::SharedObject>().Data())) {
			const auto& rhs = *static_cast<UDRefl::SharedObject*>(udata);
			member.AInvoke<void>(UDRefl::StrIDRegistry::MetaID::operator_assign, rhs);
		}
		else if (!L.getmetatable(-1)) {
			if (!member.IsInvocable<void*>(UDRefl::StrIDRegistry::MetaID::operator_assign)) {
				std::string_view tname = UDRefl::Mngr->tregistry.Nameof(ptr.DereferenceAsConst().GetID());
				return L.error("%s::__newindex : %s::%s can't be assign with void*.",
					type_name<Ptr>().Data(),
					tname.data(),
					key);
			}
			member = L.touserdata(-1);
		}
		break;
	case LUA_TSTRING:
		if (member.IsInvocable<const char*>(UDRefl::StrIDRegistry::MetaID::operator_assign))
			member = L.tostring(-1);
		else if (member.IsInvocable<std::string_view>(UDRefl::StrIDRegistry::MetaID::operator_assign)) {
			size_t len;
			const char* str = L.tolstring(-1, &len);
			member = std::string_view{ str, len };
		}
		else if (member.IsInvocable<std::string>(UDRefl::StrIDRegistry::MetaID::operator_assign)) {
			size_t len;
			const char* str = L.tolstring(-1, &len);
			member = std::string{ str, len };
		}
		else {
			std::string_view tname = UDRefl::Mngr->tregistry.Nameof(ptr.DereferenceAsConst().GetID());
			return L.error("%s::__newindex : %s::%s can't be assign with a string.",
				type_name<Ptr>().Data(),
				tname.data(),
				key);
		}
		break;
	default:
		break;
	}

	return 0;
}

template<typename Functor, typename Ptr>
static int f_Functor_call(lua_State* L_) {
	LuaStateView L(L_);

	int L_argnum = L.gettop();
	if (L_argnum == 0) {
		return L.error("%s::__call : The number of arguments is invalid. The function needs >= 1 argument (functor, args...).",
			type_name<Functor>().Data());
	}
	int argnum = L_argnum - 1;

	const auto& functor = *(Functor*)L.checkudata(1, type_name<Functor>().Data());

	Ptr ptr;
	StrID methodID;

	if constexpr (UDRefl::IsObjectOrPtr_v<Functor>) {
		ptr = functor;
		methodID = UDRefl::StrIDRegistry::MetaID::operator_call;
	}
	else {
		methodID = functor.methodID;
		if (L_argnum >= 2) {
			if (void* udata = L.testudata(2, type_name<UDRefl::ConstObjectPtr>().Data())) {
				const auto& obj = *static_cast<UDRefl::ConstObjectPtr*>(udata);
				if (obj.GetID() != functor.typeID)
					ptr = Ptr{ functor.typeID };
				else {
					ptr = UDRefl::ConstCast(obj.AddConstLValueReference());
					--argnum;
				}
			}
			else if (void* udata = L.testudata(2, type_name<UDRefl::ObjectPtr>().Data())) {
				const auto& obj = *static_cast<UDRefl::ObjectPtr*>(udata);
				if (obj.GetID() != functor.typeID)
					ptr = Ptr{ functor.typeID };
				else {
					ptr = obj.AddLValueReference();
					--argnum;
				}
			}
			else if (void* udata = L.testudata(2, type_name<UDRefl::SharedConstObject>().Data())) {
				const auto& obj = *static_cast<UDRefl::SharedConstObject*>(udata);
				if (obj.GetID() != functor.typeID)
					ptr = Ptr{ functor.typeID };
				else {
					ptr = UDRefl::ConstCast(obj->AddConstLValueReference());
					--argnum;
				}
			}
			else if (void* udata = L.testudata(2, type_name<UDRefl::SharedObject>().Data())) {
				const auto& obj = *static_cast<UDRefl::SharedObject*>(udata);
				if (obj.GetID() != functor.typeID)
					ptr = Ptr{ functor.typeID };
				else {
					ptr = obj->AddLValueReference();
					--argnum;
				}
			}
			else
				ptr = Ptr{ functor.typeID };
		}
	}

	if (argnum > UDRefl::MaxArgNum) {
		return L.error("%s::__call : The number of arguments (%d) is greater than UDRefl::MaxArgNum (%d).",
			type_name<Functor>().Data(), argnum, static_cast<lua_Integer>(UDRefl::MaxArgNum));
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

	auto rst = ptr.MInvoke(
		methodID,
		std::span<const TypeID>{reinterpret_cast<TypeID*>(argTypeID_buffer), static_cast<std::size_t>(argnum)},
		static_cast<UDRefl::ArgPtrBuffer>(argptr_buffer)
	);

	if(!rst.GetID().Valid())
		return L.error("%s::__call : Invoke fail.", type_name<Functor>().Data());

	if (rst.GetID().Is<void>())
		return 0;

	void* buffer = L.newuserdata(sizeof(UDRefl::SharedObject));
	new(buffer)UDRefl::SharedObject{ std::move(rst) };
	L.getmetatable(type_name<UDRefl::SharedObject>().Data());
	L.setmetatable(-2);

	return 1;
}

template<typename Ptr, typename MetaName, std::size_t ID>
static int f_Ptr_unary_operator(lua_State* L_) {
	LuaStateView L(L_);

	int L_argnum = L.gettop();
	if (L_argnum != 1) {
		return L.error("%s::__%s : The number of arguments is invalid. The function needs 1 argument (object).",
			type_name<Ptr>().Data(), MetaName::Data());
	}

	const auto& ptr = *(Ptr*)L.checkudata(1, type_name<Ptr>().Data());

	auto rst = ptr->MInvoke(StrID{ ID });

	if (!rst.GetID().Valid()) {
		return L.error("%s::__%s : The function isn't invocable.",
			type_name<Ptr>().Data(),
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

template<typename Ptr, typename MetaName, std::size_t ID>
static int f_Ptr_binary_operator(lua_State* L_) {
	LuaStateView L(L_);

	int L_argnum = L.gettop();
	if (L_argnum != 2) {
		return L.error("%s::__%s : The number of arguments is invalid. The function needs 2 argument (lhs, rhs).",
			type_name<Ptr>().Data(), MetaName::Data());
	}

	const auto& ptr = *(Ptr*)L.checkudata(1, type_name<Ptr>().Data());

	void* argptr;
	std::aligned_storage_t<sizeof(void*)> copied_arg;
	TypeID argTypeID;

	const int arg = 2;
	const int type = L.type(arg);
	switch (type)
	{
	case LUA_TBOOLEAN:
	{
		UDRefl::buffer_as<bool>(&copied_arg) = static_cast<bool>(L.toboolean(arg));
		argptr = &copied_arg;
		argTypeID = TypeID_of<bool>;
	}
	break;
	case LUA_TNUMBER:
		if (L.isinteger(arg)) {
			UDRefl::buffer_as<lua_Integer>(&copied_arg) = static_cast<lua_Integer>(L.tointeger(arg));
			argptr = &copied_arg;
			argTypeID = TypeID_of<lua_Integer>;
		}
		else if (L.isnumber(arg)) {
			UDRefl::buffer_as<lua_Number>(&copied_arg) = static_cast<lua_Number>(L.tointeger(arg));
			argptr = &copied_arg;
			argTypeID = TypeID_of<lua_Number>;
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
			argptr = const_cast<void*>(ref_rhs.GetPtr());
			argTypeID = ref_rhs.GetID();
		}
		else if (void* udata = L.testudata(-1, type_name<UDRefl::ObjectPtr>().Data())) {
			const auto& rhs = *static_cast<UDRefl::ObjectPtr*>(udata);
			auto ref_rhs = rhs.AddLValueReference();
			argptr = ref_rhs.GetPtr();
			argTypeID = ref_rhs.GetID();
		}
		else if (void* udata = L.testudata(-1, type_name<UDRefl::SharedConstObject>().Data())) {
			const auto& rhs = *static_cast<UDRefl::SharedConstObject*>(udata);
			auto ref_rhs = rhs->AddConstLValueReference();
			argptr = const_cast<void*>(ref_rhs.GetPtr());
			argTypeID = ref_rhs.GetID();
		}
		else if (void* udata = L.testudata(-1, type_name<UDRefl::SharedObject>().Data())) {
			const auto& rhs = *static_cast<UDRefl::SharedObject*>(udata);
			auto ref_rhs = rhs->AddLValueReference();
			argptr = ref_rhs.GetPtr();
			argTypeID = ref_rhs.GetID();
		}
		else if (!L.getmetatable(arg)) {
			argptr = L.touserdata(arg);
			argTypeID = TypeID_of<void*>;
		}
		break;
	default:
		break;
	}

	std::span<const TypeID> argTypeIDs{ &argTypeID, 1 };
	UDRefl::ArgPtrBuffer argptr_buffer = &argptr;

	auto rst = ptr->MInvoke(
		StrID{ ID },
		argTypeIDs,
		argptr_buffer
	);

	if (!rst.GetID().Valid()) {
		return L.error("%s::__%s : The function isn't invocable with %s.",
			type_name<Ptr>().Data(),
			MetaName::Data(),
			UDRefl::Mngr->tregistry.Nameof(argTypeID).data()
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

template<typename Ptr, typename MetaName, std::size_t ID>
static int f_Ptr_compare_operator(lua_State* L_) {
	LuaStateView L(L_);

	int L_argnum = L.gettop();
	if (L_argnum != 2) {
		return L.error("%s::__%s : The number of arguments is invalid. The function needs 2 argument (lhs, rhs).",
			type_name<Ptr>().Data(), MetaName::Data());
	}

	const auto& ptr = *(Ptr*)L.checkudata(1, type_name<Ptr>().Data());

	void* argptr;
	std::aligned_storage_t<sizeof(void*)> copied_arg;
	TypeID argTypeID;

	const int arg = 2;
	const int type = L.type(arg);
	switch (type)
	{
	case LUA_TBOOLEAN:
	{
		UDRefl::buffer_as<bool>(&copied_arg) = static_cast<bool>(L.toboolean(arg));
		argptr = &copied_arg;
		argTypeID = TypeID_of<bool>;
	}
	break;
	case LUA_TNUMBER:
		if (L.isinteger(arg)) {
			UDRefl::buffer_as<lua_Integer>(&copied_arg) = static_cast<lua_Integer>(L.tointeger(arg));
			argptr = &copied_arg;
			argTypeID = TypeID_of<lua_Integer>;
		}
		else if (L.isnumber(arg)) {
			UDRefl::buffer_as<lua_Number>(&copied_arg) = static_cast<lua_Number>(L.tointeger(arg));
			argptr = &copied_arg;
			argTypeID = TypeID_of<lua_Number>;
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
			argptr = const_cast<void*>(ref_rhs.GetPtr());
			argTypeID = ref_rhs.GetID();
		}
		else if (void* udata = L.testudata(-1, type_name<UDRefl::ObjectPtr>().Data())) {
			const auto& rhs = *static_cast<UDRefl::ObjectPtr*>(udata);
			auto ref_rhs = rhs.AddLValueReference();
			argptr = ref_rhs.GetPtr();
			argTypeID = ref_rhs.GetID();
		}
		else if (void* udata = L.testudata(-1, type_name<UDRefl::SharedConstObject>().Data())) {
			const auto& rhs = *static_cast<UDRefl::SharedConstObject*>(udata);
			auto ref_rhs = rhs->AddConstLValueReference();
			argptr = const_cast<void*>(ref_rhs.GetPtr());
			argTypeID = ref_rhs.GetID();
		}
		else if (void* udata = L.testudata(-1, type_name<UDRefl::SharedObject>().Data())) {
			const auto& rhs = *static_cast<UDRefl::SharedObject*>(udata);
			auto ref_rhs = rhs->AddLValueReference();
			argptr = ref_rhs.GetPtr();
			argTypeID = ref_rhs.GetID();
		}
		else if (!L.getmetatable(arg)) {
			argptr = L.touserdata(arg);
			argTypeID = TypeID_of<void*>;
		}
		break;
	default:
		break;
	}

	std::span<const TypeID> argTypeIDs{ &argTypeID, 1 };
	UDRefl::ArgPtrBuffer argptr_buffer = &argptr;

	auto rst = ptr->MInvoke(
		StrID{ ID },
		argTypeIDs,
		argptr_buffer
	);

	if (!rst.GetID().Valid()) {
		return L.error("%s::__%s : The function isn't invocable with %s.",
			type_name<Ptr>().Data(),
			MetaName::Data(),
			UDRefl::Mngr->tregistry.Nameof(argTypeID).data()
		);
	}

	L.pushboolean(static_cast<bool>(rst));

	return 1;
}

static const struct luaL_Reg lib_StrID[] = {
	"new"     , f_ID_new<StrID>,
	"GetValue", details::wrap<&StrID::GetValue, StrID>(TSTR("GetValue")),
	"Valid"   , details::wrap<&StrID::Valid,    StrID>(TSTR("Valid"   )),
	"Is"      , details::wrap<&StrID::Is,       StrID>(TSTR("Is"      )),
	NULL      , NULL
};

static const struct luaL_Reg lib_TypeID[] = {
	"new"     , f_ID_new<TypeID>,
	"GetValue", details::wrap<&TypeID::GetValue, TypeID>(TSTR("GetValue")),
	"Valid"   , details::wrap<&TypeID::Valid,    TypeID>(TSTR("Valid")),
	"Is"      , details::wrap<&IDBase::Is,       TypeID>(TSTR("Is")),
	NULL      , NULL
};

static const struct luaL_Reg lib_ConstObjectPtr[] = {
	"new"     , f_Ptr_new<UDRefl::ConstObjectPtr>,
	"GetID"   , details::wrap<&UDRefl::ConstObjectPtr::GetID,    UDRefl::ConstObjectPtr>(TSTR("GetID")),
	"Valid"   , details::wrap<&UDRefl::ConstObjectPtr::Valid,    UDRefl::ConstObjectPtr>(TSTR("Valid")),
	"GetType" , details::wrap<&UDRefl::ConstObjectPtr::GetType,  UDRefl::ConstObjectPtr>(TSTR("GetType")),
	"TypeName", details::wrap<&UDRefl::ConstObjectPtr::TypeName, UDRefl::ConstObjectPtr>(TSTR("TypeName")),
	"AsNumber", f_CPtr_AsNumber<UDRefl::ConstObjectPtr>,

	"__deref", &f_Ptr_unary_operator<UDRefl::ConstObjectPtr, details::CppMeta::t_deref, UDRefl::StrIDRegistry::MetaID::operator_deref.GetValue()>,
	"__pre_inc", &f_Ptr_unary_operator<UDRefl::ConstObjectPtr, details::CppMeta::t_pre_inc, UDRefl::StrIDRegistry::MetaID::operator_pre_inc.GetValue()>,

	"cbegin", details::wrap<&UDRefl::ConstObjectPtr::cbegin, UDRefl::ConstObjectPtr>(TSTR("cbegin")),
	"cend", details::wrap<&UDRefl::ConstObjectPtr::cend, UDRefl::ConstObjectPtr>(TSTR("cend")),
	"crbegin", details::wrap<&UDRefl::ConstObjectPtr::crbegin, UDRefl::ConstObjectPtr>(TSTR("crbegin")),
	"crend", details::wrap<&UDRefl::ConstObjectPtr::crend, UDRefl::ConstObjectPtr>(TSTR("crend")),
	"empty", details::wrap<&UDRefl::ConstObjectPtr::empty, UDRefl::ConstObjectPtr>(TSTR("empty")),
	"size", details::wrap<&UDRefl::ConstObjectPtr::size, UDRefl::ConstObjectPtr>(TSTR("size")),
	"capacity", details::wrap<&UDRefl::ConstObjectPtr::size, UDRefl::ConstObjectPtr>(TSTR("capacity")),
	"key_comp", details::wrap<&UDRefl::ConstObjectPtr::size, UDRefl::ConstObjectPtr>(TSTR("key_comp")),
	"value_comp", details::wrap<&UDRefl::ConstObjectPtr::size, UDRefl::ConstObjectPtr>(TSTR("value_comp")),
	"hash_function", details::wrap<&UDRefl::ConstObjectPtr::size, UDRefl::ConstObjectPtr>(TSTR("hash_function")),
	"key_eq", details::wrap<&UDRefl::ConstObjectPtr::size, UDRefl::ConstObjectPtr>(TSTR("key_eq")),
	"get_allocator", details::wrap<&UDRefl::ConstObjectPtr::size, UDRefl::ConstObjectPtr>(TSTR("get_allocator")),

	"begin", details::wrap<&UDRefl::ConstObjectPtr::begin, UDRefl::ConstObjectPtr>(TSTR("begin")),
	"end_", details::wrap<&UDRefl::ConstObjectPtr::end, UDRefl::ConstObjectPtr>(TSTR("end_")),
	"rbegin", details::wrap<&UDRefl::ConstObjectPtr::rbegin, UDRefl::ConstObjectPtr>(TSTR("rbegin")),
	"rend", details::wrap<&UDRefl::ConstObjectPtr::rend, UDRefl::ConstObjectPtr>(TSTR("rend")),
	"front", details::wrap<&UDRefl::ConstObjectPtr::front, UDRefl::ConstObjectPtr>(TSTR("front")),
	"back", details::wrap<&UDRefl::ConstObjectPtr::back, UDRefl::ConstObjectPtr>(TSTR("back")),
	"data", details::wrap<&UDRefl::ConstObjectPtr::data, UDRefl::ConstObjectPtr>(TSTR("data")),

	NULL      , NULL
};

static const struct luaL_Reg lib_ObjectPtr[] = {
	"new"     , f_Ptr_new<UDRefl::ObjectPtr>,
	"GetID"   , details::wrap<&UDRefl::ObjectPtr::GetID,    UDRefl::ObjectPtr>(TSTR("GetID")),
	"Valid"   , details::wrap<&UDRefl::ObjectPtr::Valid,    UDRefl::ObjectPtr>(TSTR("Valid")),
	"GetType" , details::wrap<&UDRefl::ObjectPtr::GetType,  UDRefl::ObjectPtr>(TSTR("GetType")),
	"TypeName", details::wrap<&UDRefl::ObjectPtr::TypeName, UDRefl::ObjectPtr>(TSTR("TypeName")),
	"AsNumber", f_CPtr_AsNumber<UDRefl::ObjectPtr>,

	"__deref", &f_Ptr_unary_operator<UDRefl::ObjectPtr, details::CppMeta::t_deref, UDRefl::StrIDRegistry::MetaID::operator_deref.GetValue()>,
	"__pre_inc", &f_Ptr_unary_operator<UDRefl::ObjectPtr, details::CppMeta::t_pre_inc, UDRefl::StrIDRegistry::MetaID::operator_pre_inc.GetValue()>,

	"cbegin", details::wrap<&UDRefl::ObjectPtr::cbegin, UDRefl::ObjectPtr>(TSTR("cbegin")),
	"cend", details::wrap<&UDRefl::ObjectPtr::cend, UDRefl::ObjectPtr>(TSTR("cend")),
	"crbegin", details::wrap<&UDRefl::ObjectPtr::crbegin, UDRefl::ObjectPtr>(TSTR("crbegin")),
	"crend", details::wrap<&UDRefl::ObjectPtr::crend, UDRefl::ObjectPtr>(TSTR("crend")),
	"empty", details::wrap<&UDRefl::ObjectPtr::empty, UDRefl::ObjectPtr>(TSTR("empty")),
	"size", details::wrap<&UDRefl::ObjectPtr::size, UDRefl::ObjectPtr>(TSTR("size")),
	"capacity", details::wrap<&UDRefl::ObjectPtr::size, UDRefl::ObjectPtr>(TSTR("capacity")),
	"bucket_count", details::wrap<&UDRefl::ObjectPtr::size, UDRefl::ObjectPtr>(TSTR("bucket_count")),
	"key_comp", details::wrap<&UDRefl::ObjectPtr::size, UDRefl::ObjectPtr>(TSTR("key_comp")),
	"value_comp", details::wrap<&UDRefl::ObjectPtr::size, UDRefl::ObjectPtr>(TSTR("value_comp")),
	"hash_function", details::wrap<&UDRefl::ObjectPtr::size, UDRefl::ObjectPtr>(TSTR("hash_function")),
	"key_eq", details::wrap<&UDRefl::ObjectPtr::size, UDRefl::ObjectPtr>(TSTR("key_eq")),
	"get_allocator", details::wrap<&UDRefl::ObjectPtr::size, UDRefl::ObjectPtr>(TSTR("get_allocator")),

	"begin", details::wrap<&UDRefl::ObjectPtr::begin, UDRefl::ObjectPtr>(TSTR("begin")),
	"end_", details::wrap<&UDRefl::ObjectPtr::end, UDRefl::ObjectPtr>(TSTR("end_")),
	"rbegin", details::wrap<&UDRefl::ObjectPtr::rbegin, UDRefl::ObjectPtr>(TSTR("rbegin")),
	"rend", details::wrap<&UDRefl::ObjectPtr::rend, UDRefl::ObjectPtr>(TSTR("rend")),
	"front", details::wrap<&UDRefl::ObjectPtr::front, UDRefl::ObjectPtr>(TSTR("front")),
	"back", details::wrap<&UDRefl::ObjectPtr::back, UDRefl::ObjectPtr>(TSTR("back")),
	"data", details::wrap<&UDRefl::ObjectPtr::data, UDRefl::ObjectPtr>(TSTR("data")),

	NULL      , NULL
};

static const struct luaL_Reg lib_SharedConstObject[] = {
	"new"        , f_Ptr_new<UDRefl::SharedConstObject>,
	"GetID"      , details::wrap<&UDRefl::SharedConstObject::GetID,       UDRefl::SharedConstObject>(TSTR("GetID")),
	"Valid"      , details::wrap<&UDRefl::SharedConstObject::Valid,       UDRefl::SharedConstObject>(TSTR("Valid")),
	"AsObjectPtr", details::wrap<&UDRefl::SharedConstObject::AsObjectPtr, UDRefl::SharedConstObject>(TSTR("GetType")),
	"AsNumber"   , f_CPtr_AsNumber<UDRefl::SharedConstObject>,

	"__deref", &f_Ptr_unary_operator<UDRefl::SharedConstObject, details::CppMeta::t_deref, UDRefl::StrIDRegistry::MetaID::operator_deref.GetValue()>,
	"__pre_inc", &f_Ptr_unary_operator<UDRefl::SharedConstObject, details::CppMeta::t_pre_inc, UDRefl::StrIDRegistry::MetaID::operator_pre_inc.GetValue()>,

	NULL         , NULL
};

static const struct luaL_Reg lib_SharedObject[] = {
	"new"                , f_Ptr_new<UDRefl::SharedObject>,
	"GetID"              , details::wrap<&UDRefl::SharedObject::GetID,               UDRefl::SharedObject>(TSTR("GetID")),
	"Valid"              , details::wrap<&UDRefl::SharedObject::Valid,               UDRefl::SharedObject>(TSTR("Valid")),
	"AsSharedConstObject", details::wrap<&UDRefl::SharedObject::AsSharedConstObject, UDRefl::SharedObject>(TSTR("AsSharedConstObject")),
	"AsObjectPtr"        , details::wrap<&UDRefl::SharedObject::AsObjectPtr,         UDRefl::SharedObject>(TSTR("AsObjectPtr")),
	"AsConstObjectPtr"   , details::wrap<&UDRefl::SharedObject::AsConstObjectPtr,    UDRefl::SharedObject>(TSTR("AsConstObjectPtr")),
	"AsNumber"           , f_CPtr_AsNumber<UDRefl::SharedObject>,

	"__deref", &f_Ptr_unary_operator<UDRefl::SharedObject, details::CppMeta::t_deref, UDRefl::StrIDRegistry::MetaID::operator_deref.GetValue()>,
	"__pre_inc", &f_Ptr_unary_operator<UDRefl::SharedObject, details::CppMeta::t_pre_inc, UDRefl::StrIDRegistry::MetaID::operator_pre_inc.GetValue()>,

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

void init_CallHandle(lua_State* L_) {
	LuaStateView L(L_);

	L.newmetatable(type_name<details::CallHandle>().Data());
	L.pushcfunction(&f_Functor_call<details::CallHandle, UDRefl::ObjectPtr>);
	L.setfield(-2, "__call");

	L.pop(1);
}

int luaopen_StrID(lua_State* L_) {
	LuaStateView L(L_);
	L.newmetatable(type_name<StrID>().Data());
	L.pushvalue(-1); // duplicate the metatable
	L.setfield(-2, "__index"); // mt.__index = mt
	L.setfuncs(lib_StrID, 0);
	L.newlib(lib_StrID);
	return 1;
}

int luaopen_TypeID(lua_State* L_) {
	LuaStateView L(L_);
	L.newmetatable(type_name<TypeID>().Data());
	L.pushvalue(-1); // duplicate the metatable
	L.setfield(-2, "__index"); // mt.__index = mt
	L.setfuncs(lib_TypeID, 0);
	L.newlib(lib_TypeID);
	return 1;
}

int luaopen_ConstObjectPtr(lua_State* L_) {
	LuaStateView L(L_);
	L.newmetatable(type_name<UDRefl::ConstObjectPtr>().Data());
	L.pushcfunction(&f_CPtr_index<UDRefl::ConstObjectPtr>);
	L.setfield(-2, "__index");
	L.pushcfunction(&f_CPtr_tostring<UDRefl::ConstObjectPtr>);
	L.setfield(-2, "__tostring");
	L.pushcfunction(&f_Functor_call<UDRefl::ConstObjectPtr, UDRefl::ConstObjectPtr>);
	L.setfield(-2, "__call");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ConstObjectPtr, details::Meta::t_add, UDRefl::StrIDRegistry::MetaID::operator_add.GetValue()>);
	L.setfield(-2, "__add");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ConstObjectPtr, details::Meta::t_band, UDRefl::StrIDRegistry::MetaID::operator_band.GetValue()>);
	L.setfield(-2, "__band");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ConstObjectPtr, details::Meta::t_bnot, UDRefl::StrIDRegistry::MetaID::operator_bnot.GetValue()>);
	L.setfield(-2, "__bnot");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ConstObjectPtr, details::Meta::t_bor, UDRefl::StrIDRegistry::MetaID::operator_bor.GetValue()>);
	L.setfield(-2, "__bor");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ConstObjectPtr, details::Meta::t_bxor, UDRefl::StrIDRegistry::MetaID::operator_bxor.GetValue()>);
	L.setfield(-2, "__bxor");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ConstObjectPtr, details::Meta::t_div, UDRefl::StrIDRegistry::MetaID::operator_div.GetValue()>);
	L.setfield(-2, "__div");
	L.pushcfunction(&f_Ptr_compare_operator<UDRefl::ConstObjectPtr, details::Meta::t_eq, UDRefl::StrIDRegistry::MetaID::operator_eq.GetValue()>);
	L.setfield(-2, "__eq");
	L.pushcfunction(&f_Ptr_compare_operator<UDRefl::ConstObjectPtr, details::Meta::t_le, UDRefl::StrIDRegistry::MetaID::operator_le.GetValue()>);
	L.setfield(-2, "__le");
	L.pushcfunction(&f_Ptr_compare_operator<UDRefl::ConstObjectPtr, details::Meta::t_lt, UDRefl::StrIDRegistry::MetaID::operator_lt.GetValue()>);
	L.setfield(-2, "__lt");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ConstObjectPtr, details::Meta::t_mod, UDRefl::StrIDRegistry::MetaID::operator_mod.GetValue()>);
	L.setfield(-2, "__mod");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ConstObjectPtr, details::Meta::t_mul, UDRefl::StrIDRegistry::MetaID::operator_mul.GetValue()>);
	L.setfield(-2, "__mul");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ConstObjectPtr, details::Meta::t_shl, UDRefl::StrIDRegistry::MetaID::operator_lshift.GetValue()>);
	L.setfield(-2, "__shl");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ConstObjectPtr, details::Meta::t_shr, UDRefl::StrIDRegistry::MetaID::operator_rshift.GetValue()>);
	L.setfield(-2, "__shr");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ConstObjectPtr, details::Meta::t_sub, UDRefl::StrIDRegistry::MetaID::operator_sub.GetValue()>);
	L.setfield(-2, "__sub");
	L.setfuncs(lib_ConstObjectPtr, 0);
	L.newlib(lib_ConstObjectPtr);
	return 1;
}

int luaopen_ObjectPtr(lua_State* L_) {
	LuaStateView L(L_);
	L.newmetatable(type_name<UDRefl::ObjectPtr>().Data());
	L.pushcfunction(&f_Ptr_index<UDRefl::ObjectPtr>);
	L.setfield(-2, "__index");
	L.pushcfunction(&f_Ptr_newindex<UDRefl::ObjectPtr>);
	L.setfield(-2, "__newindex");
	L.pushcfunction(&f_CPtr_tostring<UDRefl::ObjectPtr>);
	L.setfield(-2, "__tostring");
	L.pushcfunction(&f_Functor_call<UDRefl::ObjectPtr, UDRefl::ObjectPtr>);
	L.setfield(-2, "__call");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ObjectPtr, details::Meta::t_add, UDRefl::StrIDRegistry::MetaID::operator_add.GetValue()>);
	L.setfield(-2, "__add");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ObjectPtr, details::Meta::t_band, UDRefl::StrIDRegistry::MetaID::operator_band.GetValue()>);
	L.setfield(-2, "__band");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ObjectPtr, details::Meta::t_bnot, UDRefl::StrIDRegistry::MetaID::operator_bnot.GetValue()>);
	L.setfield(-2, "__bnot");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ObjectPtr, details::Meta::t_bor, UDRefl::StrIDRegistry::MetaID::operator_bor.GetValue()>);
	L.setfield(-2, "__bor");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ObjectPtr, details::Meta::t_bxor, UDRefl::StrIDRegistry::MetaID::operator_bxor.GetValue()>);
	L.setfield(-2, "__bxor");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ObjectPtr, details::Meta::t_div, UDRefl::StrIDRegistry::MetaID::operator_div.GetValue()>);
	L.setfield(-2, "__div");
	L.pushcfunction(&f_Ptr_compare_operator<UDRefl::ObjectPtr, details::Meta::t_eq, UDRefl::StrIDRegistry::MetaID::operator_eq.GetValue()>);
	L.setfield(-2, "__eq");
	L.pushcfunction(&f_Ptr_compare_operator<UDRefl::ObjectPtr, details::Meta::t_le, UDRefl::StrIDRegistry::MetaID::operator_le.GetValue()>);
	L.setfield(-2, "__le");
	L.pushcfunction(&f_Ptr_compare_operator<UDRefl::ObjectPtr, details::Meta::t_lt, UDRefl::StrIDRegistry::MetaID::operator_lt.GetValue()>);
	L.setfield(-2, "__lt");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ObjectPtr, details::Meta::t_mod, UDRefl::StrIDRegistry::MetaID::operator_mod.GetValue()>);
	L.setfield(-2, "__mod");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ObjectPtr, details::Meta::t_mul, UDRefl::StrIDRegistry::MetaID::operator_mul.GetValue()>);
	L.setfield(-2, "__mul");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ObjectPtr, details::Meta::t_shl, UDRefl::StrIDRegistry::MetaID::operator_lshift.GetValue()>);
	L.setfield(-2, "__shl");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ObjectPtr, details::Meta::t_shr, UDRefl::StrIDRegistry::MetaID::operator_rshift.GetValue()>);
	L.setfield(-2, "__shr");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::ObjectPtr, details::Meta::t_sub, UDRefl::StrIDRegistry::MetaID::operator_sub.GetValue()>);
	L.setfield(-2, "__sub");
	L.setfuncs(lib_ObjectPtr, 0);
	L.newlib(lib_ObjectPtr);
	return 1;
}

int luaopen_SharedConstObject(lua_State* L_) {
	LuaStateView L(L_);
	L.newmetatable(type_name<UDRefl::SharedConstObject>().Data());
	L.pushcfunction(&f_CPtr_index<UDRefl::SharedConstObject>);
	L.setfield(-2, "__index");
	L.pushcfunction(&f_CPtr_tostring<UDRefl::SharedConstObject>);
	L.setfield(-2, "__tostring");
	L.pushcfunction(details::wrap_dtor<UDRefl::SharedConstObject>());
	L.setfield(-2, "__gc");
	L.pushcfunction(&f_Functor_call<UDRefl::SharedConstObject, UDRefl::ConstObjectPtr>);
	L.setfield(-2, "__call");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedConstObject, details::Meta::t_add, UDRefl::StrIDRegistry::MetaID::operator_add.GetValue()>);
	L.setfield(-2, "__add");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedConstObject, details::Meta::t_band, UDRefl::StrIDRegistry::MetaID::operator_band.GetValue()>);
	L.setfield(-2, "__band");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedConstObject, details::Meta::t_bnot, UDRefl::StrIDRegistry::MetaID::operator_bnot.GetValue()>);
	L.setfield(-2, "__bnot");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedConstObject, details::Meta::t_bor, UDRefl::StrIDRegistry::MetaID::operator_bor.GetValue()>);
	L.setfield(-2, "__bor");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedConstObject, details::Meta::t_bxor, UDRefl::StrIDRegistry::MetaID::operator_bxor.GetValue()>);
	L.setfield(-2, "__bxor");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedConstObject, details::Meta::t_div, UDRefl::StrIDRegistry::MetaID::operator_div.GetValue()>);
	L.setfield(-2, "__div");
	L.pushcfunction(&f_Ptr_compare_operator<UDRefl::SharedConstObject, details::Meta::t_eq, UDRefl::StrIDRegistry::MetaID::operator_eq.GetValue()>);
	L.setfield(-2, "__eq");
	L.pushcfunction(&f_Ptr_compare_operator<UDRefl::SharedConstObject, details::Meta::t_le, UDRefl::StrIDRegistry::MetaID::operator_le.GetValue()>);
	L.setfield(-2, "__le");
	L.pushcfunction(&f_Ptr_compare_operator<UDRefl::SharedConstObject, details::Meta::t_lt, UDRefl::StrIDRegistry::MetaID::operator_lt.GetValue()>);
	L.setfield(-2, "__lt");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedConstObject, details::Meta::t_mod, UDRefl::StrIDRegistry::MetaID::operator_mod.GetValue()>);
	L.setfield(-2, "__mod");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedConstObject, details::Meta::t_mul, UDRefl::StrIDRegistry::MetaID::operator_mul.GetValue()>);
	L.setfield(-2, "__mul");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedConstObject, details::Meta::t_shl, UDRefl::StrIDRegistry::MetaID::operator_lshift.GetValue()>);
	L.setfield(-2, "__shl");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedConstObject, details::Meta::t_shr, UDRefl::StrIDRegistry::MetaID::operator_rshift.GetValue()>);
	L.setfield(-2, "__shr");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedConstObject, details::Meta::t_sub, UDRefl::StrIDRegistry::MetaID::operator_sub.GetValue()>);
	L.setfield(-2, "__sub");
	L.setfuncs(lib_SharedConstObject, 0);
	L.newlib(lib_SharedConstObject);
	return 1;
}

int luaopen_SharedObject(lua_State* L_) {
	LuaStateView L(L_);
	L.newmetatable(type_name<UDRefl::SharedObject>().Data());
	L.pushcfunction(&f_Ptr_index<UDRefl::SharedObject>);
	L.setfield(-2, "__index");
	L.pushcfunction(&f_Ptr_newindex<UDRefl::SharedObject>);
	L.setfield(-2, "__newindex");
	L.pushcfunction(&f_CPtr_tostring<UDRefl::SharedObject>);
	L.setfield(-2, "__tostring");
	L.pushcfunction(details::wrap_dtor<UDRefl::SharedObject>());
	L.setfield(-2, "__gc");
	L.pushcfunction(&f_Functor_call<UDRefl::SharedObject, UDRefl::ObjectPtr>);
	L.setfield(-2, "__call");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedObject, details::Meta::t_add, UDRefl::StrIDRegistry::MetaID::operator_add.GetValue()>);
	L.setfield(-2, "__add");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedObject, details::Meta::t_band, UDRefl::StrIDRegistry::MetaID::operator_band.GetValue()>);
	L.setfield(-2, "__band");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedObject, details::Meta::t_bnot, UDRefl::StrIDRegistry::MetaID::operator_bnot.GetValue()>);
	L.setfield(-2, "__bnot");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedObject, details::Meta::t_bor, UDRefl::StrIDRegistry::MetaID::operator_bor.GetValue()>);
	L.setfield(-2, "__bor");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedObject, details::Meta::t_bxor, UDRefl::StrIDRegistry::MetaID::operator_bxor.GetValue()>);
	L.setfield(-2, "__bxor");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedObject, details::Meta::t_div, UDRefl::StrIDRegistry::MetaID::operator_div.GetValue()>);
	L.setfield(-2, "__div");
	L.pushcfunction(&f_Ptr_compare_operator<UDRefl::SharedObject, details::Meta::t_eq, UDRefl::StrIDRegistry::MetaID::operator_eq.GetValue()>);
	L.setfield(-2, "__eq");
	L.pushcfunction(&f_Ptr_compare_operator<UDRefl::SharedObject, details::Meta::t_le, UDRefl::StrIDRegistry::MetaID::operator_le.GetValue()>);
	L.setfield(-2, "__le");
	L.pushcfunction(&f_Ptr_compare_operator<UDRefl::SharedObject, details::Meta::t_lt, UDRefl::StrIDRegistry::MetaID::operator_lt.GetValue()>);
	L.setfield(-2, "__lt");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedObject, details::Meta::t_mod, UDRefl::StrIDRegistry::MetaID::operator_mod.GetValue()>);
	L.setfield(-2, "__mod");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedObject, details::Meta::t_mul, UDRefl::StrIDRegistry::MetaID::operator_mul.GetValue()>);
	L.setfield(-2, "__mul");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedObject, details::Meta::t_shl, UDRefl::StrIDRegistry::MetaID::operator_lshift.GetValue()>);
	L.setfield(-2, "__shl");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedObject, details::Meta::t_shr, UDRefl::StrIDRegistry::MetaID::operator_rshift.GetValue()>);
	L.setfield(-2, "__shr");
	L.pushcfunction(&f_Ptr_binary_operator<UDRefl::SharedObject, details::Meta::t_sub, UDRefl::StrIDRegistry::MetaID::operator_sub.GetValue()>);
	L.setfield(-2, "__sub");
	L.setfuncs(lib_SharedObject, 0);
	L.newlib(lib_SharedObject);
	return 1;
}

int luaopen_ReflMngr(lua_State* L_) {
	LuaStateView L(L_);
	L.newlib(lib_ReflMngr);
	return 1;
}

static const luaL_Reg UDRefl_libs[] = {
  {"StrID"            , luaopen_StrID},
  {"TypeID"           , luaopen_TypeID},
  {"ConstObjectPtr"   , luaopen_ConstObjectPtr},
  {"ObjectPtr"        , luaopen_ObjectPtr},
  {"SharedConstObject", luaopen_SharedConstObject},
  {"SharedObject"     , luaopen_SharedObject},
  {"ReflMngr"         , luaopen_ReflMngr},
  {NULL               , NULL}
};

void luaopen_UDRefl_libs(lua_State* L_) {
	init_CallHandle(L_);

	LuaStateView L(L_);
	const luaL_Reg* lib;
	// "require" functions from 'UDRefl_libs' and set results to global table
	for (lib = UDRefl_libs; lib->func; lib++) {
		L.requiref(lib->name, lib->func, 1);
		lua_pop(L, 1); // remove lib
	}
}
