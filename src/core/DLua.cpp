#include <UDLua/DLua.h>

#include <ULuaW/LuaStateView.h>

#include <UDRefl/UDRefl.h>
#include <USRefl/USRefl.h>

#include <lua.hpp>

using namespace Ubpa;

template<>
struct Ubpa::USRefl::TypeInfo<Ubpa::IDBase> :
	TypeInfoBase<Ubpa::IDBase>
{
	static constexpr AttrList attrs = {};
	static constexpr FieldList fields = {
		Field {TSTR("InvalidValue"), &Type::InvalidValue},
		Field {TSTR("GetValue"), &Type::GetValue},
		Field {TSTR("Valid"), &Type::Valid},
		Field {TSTR("Is"), &Type::Is},
		Field {TSTR("Reset"), &Type::Reset},
		Field {TSTR("operator bool"), &Type::operator bool},
	};
};

template<>
struct Ubpa::USRefl::TypeInfo<Ubpa::StrID> :
	TypeInfoBase<Ubpa::StrID, Base<IDBase>>
{
	static constexpr AttrList attrs = {};
	static constexpr FieldList fields = {};
};

template<>
struct Ubpa::USRefl::TypeInfo<Ubpa::TypeID> :
	TypeInfoBase<Ubpa::TypeID, Base<IDBase>>
{
	static constexpr AttrList attrs = {};
	static constexpr FieldList fields = {
	};
};

template<>
struct Ubpa::USRefl::TypeInfo<Ubpa::UDRefl::SharedObjectBase> :
	TypeInfoBase<Ubpa::UDRefl::SharedObjectBase>
{
	static constexpr AttrList attrs = {};
	static constexpr FieldList fields = {
		Field {TSTR("GetID"), &Type::GetID},
		Field {TSTR("Valid"), &Type::Valid},
	};
};

template<>
struct Ubpa::USRefl::TypeInfo<Ubpa::UDRefl::SharedConstObject> :
	TypeInfoBase<Ubpa::UDRefl::SharedConstObject, Base<Ubpa::UDRefl::SharedObjectBase>>
{
	static constexpr AttrList attrs = {};
	static constexpr FieldList fields = {
		Field {TSTR("AsObjectPtr"), &Type::AsObjectPtr},
	};
};

template<>
struct Ubpa::USRefl::TypeInfo<Ubpa::UDRefl::SharedObject> :
	TypeInfoBase<Ubpa::UDRefl::SharedObject, Base<Ubpa::UDRefl::SharedObjectBase>>
{
	static constexpr AttrList attrs = {};
	static constexpr FieldList fields = {
		Field {TSTR("AsSharedConstObject"), &Type::AsSharedConstObject},
		Field {TSTR("AsObjectPtr"), &Type::AsObjectPtr},
		Field {TSTR("AsConstObjectPtr"), &Type::AsConstObjectPtr},
	};
};

template<>
struct Ubpa::USRefl::TypeInfo<Ubpa::UDRefl::ObjectPtrBase> :
	TypeInfoBase<Ubpa::UDRefl::ObjectPtrBase>
{
	static constexpr AttrList attrs = {};
	static constexpr FieldList fields = {
		Field {TSTR("GetID"), &Type::GetID},
		Field {TSTR("Valid"), &Type::Valid},
		Field {TSTR("GetType"), &Type::GetType},
		Field {TSTR("TypeName"), &Type::TypeName},
	};
};

template<>
struct Ubpa::USRefl::TypeInfo<Ubpa::UDRefl::ConstObjectPtr> :
	TypeInfoBase<Ubpa::UDRefl::ConstObjectPtr, Base<UDRefl::ObjectPtrBase>>
{
	static constexpr AttrList attrs = {};
	static constexpr FieldList fields = {};
};

template<>
struct Ubpa::USRefl::TypeInfo<Ubpa::UDRefl::ObjectPtr> :
	TypeInfoBase<Ubpa::UDRefl::ObjectPtr, Base<UDRefl::ObjectPtrBase>>
{
	static constexpr AttrList attrs = {};
	static constexpr FieldList fields = {};
};

namespace Ubpa::details {
	template<auto fieldptr>
	struct FieldName;
}

template<typename Obj, typename Func, Func Obj::* fieldptr>
struct Ubpa::details::FieldName<fieldptr> {
	static constexpr auto get() {
		constexpr auto& field = USRefl_ElemList_GetByValue(Ubpa::USRefl::TypeInfo<Obj>::fields, fieldptr);
		return typename std::decay_t<decltype(field)>::TName{};
	}
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

	template<auto funcptr, typename CustomObj = void>
	constexpr lua_CFunction wrap() noexcept {
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
						FieldName<funcptr>::get().Data(),
						static_cast<lua_Integer>(Length_v<ArgList>));

				auto* obj = (Obj*)L.checkudata(1, type_name<Obj>().Data());
				caller<funcptr>(L, obj, std::make_index_sequence<Length_v<ArgList>>{});
			}
			else if constexpr (is_function_pointer_v<FuncPtr>) {
				using Obj = CustomObj;
				if (n != Length_v<ArgList>)
					return L.error("%s::%s : The number of arguments is invalid. The function needs %I arguments (no object).",
						type_name<Obj>().Data(),
						FieldName<funcptr>::get().Data(),
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

	std::pmr::vector<void*> argsbuffer{ std::pmr::vector<void*>::allocator_type{ UDRefl::Mngr->GetTemporaryResource() } };
	std::pmr::vector<std::pmr::vector<std::uint8_t>> copied_args
		{ std::pmr::vector<std::pmr::vector<std::uint8_t>>::allocator_type{ UDRefl::Mngr->GetTemporaryResource() } };
	std::pmr::vector<TypeID> typeIDs{ std::pmr::vector<TypeID>::allocator_type{ UDRefl::Mngr->GetTemporaryResource() } };
	argsbuffer.resize(argnum);
	typeIDs.resize(argnum);
	
	for (std::size_t i{ 0 }; i < argnum; i++) {
		int arg = static_cast<int>(i) + L_argnum - argnum + 1;
		int type = L.type(arg);
		switch (type)
		{
		case LUA_TBOOLEAN:
		{
			std::pmr::vector<std::uint8_t> buffer{ std::pmr::vector<std::uint8_t>::allocator_type{  UDRefl::Mngr->GetTemporaryResource() } };
			buffer.resize(sizeof(bool));
			UDRefl::buffer_as<bool>(buffer.data()) = static_cast<bool>(L.toboolean(arg));
			argsbuffer[i] = buffer.data();
			typeIDs[i] = TypeID_of<bool>;
			copied_args.push_back(std::move(buffer));
		}
			break;
		case LUA_TNUMBER:
			if (L.isinteger(arg)) {
				std::pmr::vector<std::uint8_t> buffer{ std::pmr::vector<std::uint8_t>::allocator_type{  UDRefl::Mngr->GetTemporaryResource() } };
				buffer.resize(sizeof(lua_Integer));
				UDRefl::buffer_as<lua_Integer>(buffer.data()) = static_cast<lua_Integer>(L.tointeger(arg));
				argsbuffer[i] = buffer.data();
				typeIDs[i] = TypeID_of<lua_Integer>;
				copied_args.push_back(std::move(buffer));
			}
			else if (L.isnumber(arg)) {
				std::pmr::vector<std::uint8_t> buffer{ std::pmr::vector<std::uint8_t>::allocator_type{  UDRefl::Mngr->GetTemporaryResource() } };
				buffer.resize(sizeof(lua_Number));
				UDRefl::buffer_as<lua_Number>(buffer.data()) = static_cast<lua_Number>(L.tointeger(arg));
				argsbuffer[i] = buffer.data();
				typeIDs[i] = TypeID_of<lua_Number>;
				copied_args.push_back(std::move(buffer));
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
				argsbuffer[i] = const_cast<void*>(ref_rhs.GetPtr());
				typeIDs[i] = ref_rhs.GetID();
			}
			else if (void* udata = L.testudata(-1, type_name<UDRefl::ObjectPtr>().Data())) {
				const auto& rhs = *static_cast<UDRefl::ObjectPtr*>(udata);
				auto ref_rhs = rhs.AddLValueReference();
				argsbuffer[i] = ref_rhs.GetPtr();
				typeIDs[i] = ref_rhs.GetID();
			}
			else if (void* udata = L.testudata(-1, type_name<UDRefl::SharedConstObject>().Data())) {
				const auto& rhs = *static_cast<UDRefl::SharedConstObject*>(udata);
				auto ref_rhs = rhs->AddConstLValueReference();
				argsbuffer[i] = const_cast<void*>(ref_rhs.GetPtr());
				typeIDs[i] = ref_rhs.GetID();
			}
			else if (void* udata = L.testudata(-1, type_name<UDRefl::SharedObject>().Data())) {
				const auto& rhs = *static_cast<UDRefl::SharedObject*>(udata);
				auto ref_rhs = rhs->AddLValueReference();
				argsbuffer[i] = ref_rhs.GetPtr();
				typeIDs[i] = ref_rhs.GetID();
			}
			else if (!L.getmetatable(arg)) {
				argsbuffer[i] = L.touserdata(arg);
				typeIDs[i] = TypeID_of<void*>;
			}
			break;
		default:
			break;
		}
	}

	auto rst = ptr.MInvoke(
		methodID,
		std::span<const TypeID>{typeIDs.data(), typeIDs.size()},
		static_cast<UDRefl::ArgsBuffer>(argsbuffer.data())
	);

	if(!rst.GetID().Valid())
		return L.error("%s::__call : Fail.", type_name<Functor>().Data());

	if (rst.GetID().Is<void>())
		return 0;

	void* buffer = L.newuserdata(sizeof(UDRefl::SharedObject));
	new(buffer)UDRefl::SharedObject{ std::move(rst) };
	L.getmetatable(type_name<UDRefl::SharedObject>().Data());
	L.setmetatable(-2);

	return 1;
}

template<typename Ptr>
static int f_Ptr_operator_add(lua_State* L_) {
	LuaStateView L(L_);

	int L_argnum = L.gettop();
	if (L_argnum != 2) {
		return L.error("%s::__add : The number of arguments is invalid. The function needs 2 argument (lhs, rhs).",
			type_name<Ptr>().Data());
	}

	const auto& ptr = *(Ptr*)L.checkudata(1, type_name<Ptr>().Data());

	void* argsbuffer;
	std::pmr::vector<std::uint8_t> copied_arg{ std::pmr::vector<std::uint8_t>::allocator_type{ UDRefl::Mngr->GetTemporaryResource() } };
	TypeID argTypeID;

	const int arg = 2;
	const int type = L.type(arg);
	switch (type)
	{
	case LUA_TBOOLEAN:
	{
		copied_arg.resize(sizeof(bool));
		UDRefl::buffer_as<bool>(copied_arg.data()) = static_cast<bool>(L.toboolean(arg));
		argsbuffer = copied_arg.data();
		argTypeID = TypeID_of<bool>;
	}
	break;
	case LUA_TNUMBER:
		if (L.isinteger(arg)) {
			copied_arg.resize(sizeof(lua_Integer));
			UDRefl::buffer_as<lua_Integer>(copied_arg.data()) = static_cast<lua_Integer>(L.tointeger(arg));
			argsbuffer = copied_arg.data();
			argTypeID = TypeID_of<lua_Integer>;
		}
		else if (L.isnumber(arg)) {
			copied_arg.resize(sizeof(lua_Number));
			UDRefl::buffer_as<lua_Number>(copied_arg.data()) = static_cast<lua_Number>(L.tointeger(arg));
			argsbuffer = copied_arg.data();
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
			argsbuffer = const_cast<void*>(ref_rhs.GetPtr());
			argTypeID = ref_rhs.GetID();
		}
		else if (void* udata = L.testudata(-1, type_name<UDRefl::ObjectPtr>().Data())) {
			const auto& rhs = *static_cast<UDRefl::ObjectPtr*>(udata);
			auto ref_rhs = rhs.AddLValueReference();
			argsbuffer = ref_rhs.GetPtr();
			argTypeID = ref_rhs.GetID();
		}
		else if (void* udata = L.testudata(-1, type_name<UDRefl::SharedConstObject>().Data())) {
			const auto& rhs = *static_cast<UDRefl::SharedConstObject*>(udata);
			auto ref_rhs = rhs->AddConstLValueReference();
			argsbuffer = const_cast<void*>(ref_rhs.GetPtr());
			argTypeID = ref_rhs.GetID();
		}
		else if (void* udata = L.testudata(-1, type_name<UDRefl::SharedObject>().Data())) {
			const auto& rhs = *static_cast<UDRefl::SharedObject*>(udata);
			auto ref_rhs = rhs->AddLValueReference();
			argsbuffer = ref_rhs.GetPtr();
			argTypeID = ref_rhs.GetID();
		}
		else if (!L.getmetatable(arg)) {
			argsbuffer = L.touserdata(arg);
			argTypeID = TypeID_of<void*>;
		}
		break;
	default:
		break;
	}

	auto rst = ptr->MInvoke(
		UDRefl::StrIDRegistry::MetaID::operator_add,
		std::span<const TypeID>{&argTypeID, 1},
		static_cast<UDRefl::ArgsBuffer>(&argsbuffer)
	);

	if (!rst.GetID().Valid())
		return L.error("%s::__add : Fail.", type_name<Ptr>().Data());

	if (rst.GetID().Is<void>())
		return 0;

	void* buffer = L.newuserdata(sizeof(UDRefl::SharedObject));
	new(buffer)UDRefl::SharedObject{ std::move(rst) };
	L.getmetatable(type_name<UDRefl::SharedObject>().Data());
	L.setmetatable(-2);

	return 1;
}

static const struct luaL_Reg lib_StrID[] = {
	"new"     , f_ID_new<StrID>,
	"GetValue", details::wrap<&StrID::GetValue, StrID>(),
	"Valid"   , details::wrap<&StrID::Valid,    StrID>(),
	"Is"      , details::wrap<&StrID::Is,       StrID>(),
	NULL      , NULL
};

static const struct luaL_Reg lib_TypeID[] = {
	"new"     , f_ID_new<TypeID>,
	"GetValue", details::wrap<&TypeID::GetValue, TypeID>(),
	"Valid"   , details::wrap<&TypeID::Valid,    TypeID>(),
	"Is"      , details::wrap<&IDBase::Is,       TypeID>(),
	NULL      , NULL
};

static const struct luaL_Reg lib_ConstObjectPtr[] = {
	"new"     , f_Ptr_new<UDRefl::ConstObjectPtr>,
	"GetID"   , details::wrap<&UDRefl::ConstObjectPtr::GetID,    UDRefl::ConstObjectPtr>(),
	"Valid"   , details::wrap<&UDRefl::ConstObjectPtr::Valid,    UDRefl::ConstObjectPtr>(),
	"GetType" , details::wrap<&UDRefl::ConstObjectPtr::GetType,  UDRefl::ConstObjectPtr>(),
	"TypeName", details::wrap<&UDRefl::ConstObjectPtr::TypeName, UDRefl::ConstObjectPtr>(),
	"AsNumber", f_CPtr_AsNumber<UDRefl::ConstObjectPtr>,
	NULL      , NULL
};

static const struct luaL_Reg lib_ObjectPtr[] = {
	"new"     , f_Ptr_new<UDRefl::ObjectPtr>,
	"GetID"   , details::wrap<&UDRefl::ObjectPtr::GetID,    UDRefl::ObjectPtr>(),
	"Valid"   , details::wrap<&UDRefl::ObjectPtr::Valid,    UDRefl::ObjectPtr>(),
	"GetType" , details::wrap<&UDRefl::ObjectPtr::GetType,  UDRefl::ObjectPtr>(),
	"TypeName", details::wrap<&UDRefl::ObjectPtr::TypeName, UDRefl::ObjectPtr>(),
	"AsNumber", f_CPtr_AsNumber<UDRefl::ObjectPtr>,
	NULL      , NULL
};

static const struct luaL_Reg lib_SharedConstObject[] = {
	"new"        , f_Ptr_new<UDRefl::SharedConstObject>,
	"GetID"      , details::wrap<&UDRefl::SharedConstObject::GetID,       UDRefl::SharedConstObject>(),
	"Valid"      , details::wrap<&UDRefl::SharedConstObject::Valid,       UDRefl::SharedConstObject>(),
	"AsObjectPtr", details::wrap<&UDRefl::SharedConstObject::AsObjectPtr, UDRefl::SharedConstObject>(),
	"AsNumber"   , f_CPtr_AsNumber<UDRefl::SharedConstObject>,
	NULL         , NULL
};

static const struct luaL_Reg lib_SharedObject[] = {
	"new"                , f_Ptr_new<UDRefl::SharedObject>,
	"GetID"              , details::wrap<&UDRefl::SharedObject::GetID,               UDRefl::SharedObject>(),
	"Valid"              , details::wrap<&UDRefl::SharedObject::Valid,               UDRefl::SharedObject>(),
	"AsSharedConstObject", details::wrap<&UDRefl::SharedObject::AsSharedConstObject, UDRefl::SharedObject>(),
	"AsObjectPtr"        , details::wrap<&UDRefl::SharedObject::AsObjectPtr,         UDRefl::SharedObject>(),
	"AsConstObjectPtr"   , details::wrap<&UDRefl::SharedObject::AsConstObjectPtr,    UDRefl::SharedObject>(),
	"AsNumber"           , f_CPtr_AsNumber<UDRefl::SharedObject>,
	NULL                 , NULL
};

static int f_ReflMngr_MakeShared(lua_State* L_) {
	LuaStateView L{ L_ };
	int size = L.gettop();
	if (size == 0)
		return L.error("%s::MakeShared : The number of arguments is invalid.", type_name<UDRefl::ReflMngr>().Data());
	auto typeID = *(TypeID*)L.checkudata(1, type_name<TypeID>().Data());

	UDRefl::SharedObject obj;

	if (size == 1)
		obj = UDRefl::Mngr->MakeShared(typeID);
	else
		assert(false); // TODO

	if (!obj.Valid())
		return L.error("%s::MakeShared : fail.", type_name<UDRefl::ReflMngr>().Data());
	
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
	L.pushcfunction(&f_Ptr_operator_add<UDRefl::ConstObjectPtr>);
	L.setfield(-2, "__add");
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
	L.pushcfunction(&f_Ptr_operator_add<UDRefl::ObjectPtr>);
	L.setfield(-2, "__add");
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
	L.pushcfunction(&f_Ptr_operator_add<UDRefl::SharedConstObject>);
	L.setfield(-2, "__add");
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
	L.pushcfunction(&f_Ptr_operator_add<UDRefl::SharedObject>);
	L.setfield(-2, "__add");
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
