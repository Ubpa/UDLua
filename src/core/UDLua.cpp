#include <UDLua/UDLua.h>

#include "Util.h"

#include <UDRefl_ext/Bootstrap.h>

#include <map>
#include <stdexcept>

using namespace Ubpa;

namespace Ubpa::details {
	struct Invalid {}; // for f_meta
}

template<typename Functor, typename MetaName, int LArgNum = -1, typename Ret = Ubpa::details::Invalid, bool Inversable = false>
static int f_meta(lua_State * L_) {
	static_assert(!Inversable || UDRefl::IsObjectOrView_v<Functor>);

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

	int functor_idx;
	if constexpr (Inversable) {
		if (details::safe_get_Object(L, 1))
			functor_idx = 1;
		else
			functor_idx = 2;
	}
	else
		functor_idx = 1;

	const auto& functor = details::auto_get<Functor>(L, functor_idx);

	if constexpr (UDRefl::IsObjectOrView_v<Functor>) {
		ptr = UDRefl::ObjectView{ functor.GetType(), functor.GetPtr() };
		method_name = { MetaName::View() };
	}
	else {
		ptr = UDRefl::ObjectView{ functor.type, nullptr };
		method_name = functor.method_name;
		if (L_argnum >= 2) {
			if (void* udata = L.testudata(2, type_name<UDRefl::ObjectView>().Data())) {
				UDRefl::ObjectView* obj = static_cast<UDRefl::ObjectView*>(udata);
				if (obj->GetType() == functor.type) {
					ptr = *obj;
					--argnum;
				}
				else if (functor.type == Type_of<UDRefl::ObjectView>) {
					ptr = { Type_of<UDRefl::ObjectView>, obj };
					--argnum;
				}
			}
			else if (auto obj = (UDRefl::SharedObject*)L.testudata(2, type_name<UDRefl::SharedObject>().Data())) {
				if (obj->GetType() == functor.type) {
					ptr = *obj;
					--argnum;
				}
				else if (functor.type.Is<UDRefl::ObjectView>()) {
					ptr = { Type_of<UDRefl::ObjectView>, static_cast<UDRefl::ObjectView*>(obj) };
					--argnum;
				}
				else if (functor.type.Is<UDRefl::SharedObject>()) {
					ptr = { Type_of<UDRefl::SharedObject>, obj };
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

	{ // fill argstack
		int error;
		if constexpr (Inversable) {
			if (functor_idx == 2)
				error = details::FillArgStack(L, argstack, 1, 1);
			else
				error = details::FillArgStack(L, argstack, L_argnum - argnum + 1, argnum);
		}
		else
			error = details::FillArgStack(L, argstack, L_argnum - argnum + 1, argnum);

		if (error) {
			return L.error("%s::new : \n%s",
				type_name<UDRefl::SharedObject>().Data(), L.tostring(-1));
		}
	}

	if constexpr (!std::is_same_v<Ret, details::Invalid>) {
		Type result_type = ptr.IsInvocable(method_name,
			std::span<const Type>{reinterpret_cast<Type*>(argstack.argType_buffer),
			static_cast<std::size_t>(argnum)});
		if (!result_type.Is<Ret>()) {
			return L.error("%s::%s : The function isn't invocable with arguments or it's return type isn't %s.",
				type_name<Functor>().Data(),
				MetaName::Data(),
				type_name<Ret>().Data()
			);
		}

		if constexpr (std::is_void_v<Ret>) {
			try {
				ptr.Invoke<void>(
					method_name,
					{
						argstack.argptr_buffer,
						{reinterpret_cast<Type*>(argstack.argType_buffer), static_cast<std::size_t>(argnum)}
					}
				);
			}
			catch (const std::runtime_error& e) {
				return L.error("%s::%s : Invoke exception.\n%s",
					type_name<Functor>().Data(),
					MetaName::Data(),
					e.what()
				);
			}
			catch (...) {
				return L.error("%s::%s : Invoke exception.\n",
					type_name<Functor>().Data(),
					MetaName::Data()
				);
			}
			
			return 0;
		}
		else {
			try {
				Ret rst = ptr.Invoke<Ret>(
					method_name,
					{
						argstack.argptr_buffer,
						{reinterpret_cast<Type*>(argstack.argType_buffer), static_cast<std::size_t>(argnum)}
					}
				);

				details::push<Ret>(L, std::move(rst));
			}
			catch (const std::runtime_error& e) {
				return L.error("%s::%s : Invoke exception.\n%s",
					type_name<Functor>().Data(),
					MetaName::Data(),
					e.what()
				);
			}
			catch (...) {
				return L.error("%s::%s : Invoke exception.\n",
					type_name<Functor>().Data(),
					MetaName::Data()
				);
			}

			return 1;
		}
	}
	else {
		{ // get unsync resource
			int success = L.getfield(LUA_REGISTRYINDEX, details::UnsyncRsrc);
			assert(success);
		}
		auto* rsrc = (std::pmr::unsynchronized_pool_resource*)L.touserdata(-1);
		try {
			UDRefl::SharedObject rst = ptr.MInvoke(
				method_name,
				rsrc,
				{
					argstack.argptr_buffer,
					{reinterpret_cast<Type*>(argstack.argType_buffer), static_cast<std::size_t>(argnum)}
				},
				UDRefl::MethodFlag::All,
				rsrc
			);

			if (!rst.GetType()) {
				return L.error("%s::%s : Call Ubpa::UDRefl::ObjectView::MInvoke (%s, %s) failed.",
					type_name<Functor>().Data(),
					MetaName::Data(),
					ptr.GetType().GetName().data(),
					method_name.GetView().data()
				);
			}

			if (rst.GetType().Is<void>())
				return 0;

			if (rst.IsObjectView())
				details::push<UDRefl::ObjectView>(L, rst);
			else {
				rst = { UDRefl::ObjectView{rst.GetType(), rst.GetPtr()},[rsrc_ref = LuaRef{L}, buffer = rst.GetBuffer()](void*){} };
				details::push(L, std::move(rst));
			}

			return 1;
		}
		catch (const std::runtime_error& e) {
			return L.error("%s::%s : Invoke exception.\n%s",
				type_name<Functor>().Data(),
				MetaName::Data(),
				e.what()
			);
		}
		catch (...) {
			return L.error("%s::%s : Invoke exception.\n",
				type_name<Functor>().Data(),
				MetaName::Data()
			);
		}
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
		case LUA_TNUMBER:
		{
			auto value = details::auto_get<std::size_t>(L, 1);
			T t;
			if constexpr (std::is_same_v<T, Name>) {
				NameID id{ value };
				std::string_view name = UDRefl::Mngr.nregistry.Viewof(id);
				if (name.empty()) {
					return L.error("%s::new: Not found name of ID (lua_Integer: %I)",
						type_name<T>().Data(),
						static_cast<lua_Integer>(value));
				}
				t = { name, id };
			}
			else {
				TypeID id{ value };
				std::string_view name = UDRefl::Mngr.tregistry.Viewof(id);
				if (name.empty()) {
					return L.error("%s::new: Not found name of ID (lua_Integer: %I)",
						type_name<T>().Data(),
						static_cast<lua_Integer>(value));
				}
				t = { name, id };
			}
			void* buffer = L.newuserdata(sizeof(T));
			new(buffer)T{ t };
			break;
		}
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
			new (buffer) T{ t };
			break;
		}
		default:
			return L.error("%s::new : The type of argument#1 is invalid. The function needs 0 argument / a string/integer.", type_name<T>().Data());
		}
	}
	else
		return L.error("%s::new : The number of arguments is invalid. The function needs 0 argument / a string/integer.", type_name<T>().Data());

	L.getmetatable(type_name<T>().Data());
	L.setmetatable(-2);
	return 1;
}

static int f_ObjectView_new(lua_State* L_) {
	LuaStateView L{ L_ };
	int size = L.gettop();
	if (size == 0)
		details::push(L, UDRefl::ObjectView{});
	else if (size == 1) {
		int type = L.type(-1);
		switch (type)
		{
		case LUA_TUSERDATA:
		{
			auto* id = (Type*)L.checkudata(1, type_name<Type>().Data());
			details::push(L, UDRefl::ObjectView{ *id });
			break;
		}
		default:
			return L.error("%s::new : The type of argument#1 is invalid. The function needs 0 argument / a Type.", type_name<UDRefl::ObjectView>().Data());
		}
	}
	else
		return L.error("%s::new : The number of arguments is invalid. The function needs 0 argument / a Type.", type_name<UDRefl::ObjectView>().Data());

	return 1;
}

static int f_ObjectView_tostring(lua_State* L_) {
	LuaStateView L{ L_ };

	if (L.gettop() != 1)
		return L.error("%s::__tostring : The number of arguments is invalid. The function needs 1 argument (object).", type_name<UDRefl::ObjectView>().Data());

	auto ptr = details::auto_get<UDRefl::ObjectView>(L, 1);

	if (!ptr.GetPtr())
		return L.error("%s::__tostring : The object is nil.", type_name<UDRefl::ObjectView>().Data());

	if (!ptr.IsInvocable<std::stringstream&>(UDRefl::NameIDRegistry::Meta::operator_shr)) {
		return L.error("%s::__tostring : The type (%s) can't convert to a string.",
			type_name<UDRefl::ObjectView>().Data(),
			ptr.GetType().GetName().data());
	}

	std::stringstream ss;
	try {
		ss << ptr;
	}
	catch (const std::runtime_error& e) {
		return L.error("%s::__tostring : Exception (<<).\n%s",
			type_name<UDRefl::ObjectView>().Data(),
			e.what());
	}
	catch (...) {
		return L.error("%s::__tostring : Exception (<<).",
			type_name<UDRefl::ObjectView>().Data());
	}
	auto str = ss.str();

	L.pushlstring(str.data(), str.size());

	return 1;
}

static int f_ObjectView_index(lua_State* L_) {
	LuaStateView L{ L_ };

	if (L.gettop() != 2)
		return L.error("%s::__index : The number of arguments is invalid. The function needs 2 argument (object + key).", type_name<UDRefl::ObjectView>().Data());

	int type = L.type(2);

	Name key;
	switch (type)
	{
	case LUA_TUSERDATA:
	{
		if (auto* pName = (Name*)L.testudata(2, type_name<Name>().Data()))
			key = *pName;
		else
			return f_meta<UDRefl::ObjectView, details::Meta::t_operator_subscript, 2>(L_);

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
		return f_meta<UDRefl::ObjectView, details::Meta::t_operator_subscript, 2>(L_);
	}

	if (!key)
		return L.error("%s::__index : key is empty.", type_name<UDRefl::ObjectView>().Data());

	// order
	// 1. metatable
	// 2. var
	// 3. method
	// 4. ObjectView's method
	// 5. self

	constexpr auto contains_method = [](UDRefl::ObjectView obj, Name name) {
		auto methods = UDRefl::MethodRange{ obj };
		return std::find_if(methods.begin(), methods.end(), [name](const auto& name_methodinfo) {
			return std::get<const Name>(name_methodinfo) == name;
		}) != methods.end();
	};

	if (L.getmetatable(1) && L.getfield(-1, key.GetView().data()))
		return 1; // the field is already on the stack, so return directly

	auto ptr = details::auto_get<UDRefl::ObjectView>(L, 1);
	if (!ptr.GetType().Valid())
		return L.error("%s::__index : the type of object is invalid.", type_name<UDRefl::ObjectView>().Data());

	if (auto key_obj = ptr.Var(key); key_obj.GetPtr()) {
		auto* buffer = L.newuserdata(sizeof(UDRefl::ObjectView));
		new(buffer)UDRefl::ObjectView{ key_obj };
		L.getmetatable(type_name<UDRefl::ObjectView>().Data());
		L.setmetatable(-2);
	}
	else if (contains_method(ptr, key)) {
		auto* buffer = L.newuserdata(sizeof(details::CallHandle));
		new(buffer)details::CallHandle{ ptr.GetType(), key };
		L.getmetatable(type_name<details::CallHandle>().Data());
		L.setmetatable(-2);
	}
	else if (key.Is("self")) {
		L.pushvalue(1);
		return 1;
	}
	else if (contains_method(UDRefl::ObjectView_of<UDRefl::ObjectView>, key)) {
		auto* buffer = L.newuserdata(sizeof(details::CallHandle));
		new(buffer)details::CallHandle{ Type_of<UDRefl::ObjectView>, key };
		L.getmetatable(type_name<details::CallHandle>().Data());
		L.setmetatable(-2);
	}
	else if (contains_method(UDRefl::ObjectView_of<UDRefl::SharedObject>, key)) {
		auto* buffer = L.newuserdata(sizeof(details::CallHandle));
		new(buffer)details::CallHandle{ Type_of<UDRefl::SharedObject>, key };
		L.getmetatable(type_name<details::CallHandle>().Data());
		L.setmetatable(-2);
	}
	else
	{
		return L.error("%s::__index : %s index \"%s\" failed.",
			type_name<UDRefl::ObjectView>().Data(),
			ptr.GetType().GetName().data(),
			key.GetView().data());
	}
	
	return 1;
}

static int f_ObjectView_newindex(lua_State* L_) {
	LuaStateView L{ L_ };
	if (L.gettop() != 3) {
		return L.error("%s::__newindex : The number of arguments is invalid. The function needs 3 argument (object, key, value).",
			type_name<UDRefl::ObjectView>().Data());
	}

	// stack : ptr, key, value
	L.getmetatable(type_name<UDRefl::ObjectView>().Data());
	L.getfield(-1, details::Meta::operator_assignment.Data());
	L.getfield(-2, "__index");
	L.rotate(1, -2);
	// stack : value, ..., __assignment, __index, ptr, key
	{
		int error = L.pcall(2, 1, 0);
		if (error) {
			return L.error("%s::__newindex: Call __index failed.\n%s",
				type_name<UDRefl::ObjectView>().Data(), L.tostring(-1));
		}
	}
	// stack : value, ..., __assignment, __index result (member ptr)
	L.rotate(1, -1);
	// stack : ..., __assignment, __index result (member ptr), value
	{
		int error = L.pcall(2, 1, 0);
		if (error) {
			return L.error("%s::__newindex: Call __assignment failed.\n%s",
				type_name<UDRefl::ObjectView>().Data(), L.tostring(-1));
		}
	}
	return 0;
}

static int f_ObjectView_range_next(lua_State* L_) {
	LuaStateView L{ L_ };
	const int argnum = L.gettop();

	if (argnum != 1 && argnum != 2) {
		return L.error("%s::range_next : The number of arguments is invalid. The function needs 1/2 argument (end_iter[, iter/nil]).",
			type_name<UDRefl::ObjectView>().Data());
	}

	auto ptr = details::auto_get< UDRefl::ObjectView>(L, 1);
	UDRefl::SharedObject end_iter = ptr.end();

	if (!end_iter.GetType()) {
		return L.error("%s::range_next : The type (%s) can't invoke end.",
			type_name<UDRefl::ObjectView>().Data(),
			ptr.GetType().GetName().data());
	}
	if (argnum == 1)
		L.pushnil();

	int type = L.type(2);
	try {
		switch (type)
		{
		case LUA_TNIL:
		{
			UDRefl::SharedObject iter = ptr.begin();
			if (!iter.GetType()) {
				return L.error("%s::range_next : The type (%s) can't invoke begin.",
					type_name<UDRefl::ObjectView>().Data(),
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
				return L.error("%s::range_next : The type (%s) can't invoke operator++().",
					type_name<UDRefl::ObjectView>().Data(),
					iter.GetType().GetName().data());
			}
			if (iter == end_iter)
				L.pushnil();
			return 1; // stack top is the iter / nil
		}
		default:
			return L.error("%s::range_next : The second arguments must be a nil/iter.",
				type_name<UDRefl::ObjectView>().Data());
		}
	}
	catch (const std::runtime_error& e) {
		return L.error("%s::tuple_bind : Exception.\n%s",
			type_name<UDRefl::ObjectView>().Data(),
			e.what()
		);
	}
	catch (...) {
		return L.error("%s::range_next : Exception.\n",
			type_name<UDRefl::ObjectView>().Data()
		);
	}
}

static int f_ObjectView_range(lua_State* L_) {
	LuaStateView L{ L_ };
	if (L.gettop() != 1) {
		return L.error("%s::range : The number of arguments is invalid. The function needs 1 argument (obj).",
			type_name<UDRefl::ObjectView>().Data());
	}
	details::auto_get<UDRefl::ObjectView>(L, 1);
	L.pushcfunction(f_ObjectView_range_next);
	L.pushvalue(1);
	L.pushnil();
	return 3;
}

static int f_ObjectView_concat(lua_State* L_) {
	LuaStateView L{ L_ };

	if (L.gettop() != 2) {
		return L.error("%s::__concat : The number of arguments is invalid. The function needs 2 argument.",
			type_name<UDRefl::ObjectView>().Data());
	}

	if (L.testudata(1, type_name<UDRefl::ObjectView>().Data()) || L.testudata(1, type_name<UDRefl::SharedObject>().Data())) {
		L.pushcfunction(f_ObjectView_tostring);
		L.rotate(1, -1);
		int error = L.pcall(1, 1, 0);
		if (error) {
			return L.error("%s::__concat : The object call __tostring failed.\n",
				type_name<UDRefl::ObjectView>().Data(), L.tostring(-1));
		}
		L.rotate(1, 1);
	}
	else {
		L.rotate(1, 1);
		L.pushcfunction(f_ObjectView_tostring);
		L.rotate(1, -1);
		int error = L.pcall(1, 1, 0);
		if (error) {
			return L.error("%s::__concat : The object call __tostring failed.\n",
				type_name<UDRefl::ObjectView>().Data(), L.tostring(-1));
		}
	}

	L.concat(2);
	return 1;
}

static int f_ObjectView_tuple_bind(lua_State* L_) {
	LuaStateView L{ L_ };
	auto obj = details::auto_get<UDRefl::ObjectView>(L, 1);
	try {
		std::size_t n = obj.tuple_size();
		for (std::size_t i{ 0 }; i < n; i++)
			details::push(L, obj.get(i));
		return static_cast<int>(n);
	}
	catch (const std::runtime_error& e) {
		return L.error("%s::tuple_bind : Exception.\n%s",
			type_name<UDRefl::ObjectView>().Data(),
			e.what()
		);
	}
	catch (...) {
		return L.error("%s::tuple_bind : Exception.\n",
			type_name<UDRefl::ObjectView>().Data()
		);
	}
}

static int f_SharedObject_new(lua_State* L_) {
	LuaStateView L{ L_ };
	const int L_argnum = L.gettop();
	if (L_argnum <= 0)
		return L.error("%s::new : The number of arguments is invalid.", type_name<UDRefl::SharedObject>().Data());

	Type type;
	int argtype = L.type(1);
	int argnum = 0;
	int arg_begin = 0;
	switch (argtype)
	{
	case LUA_TSTRING:
		type = Type{ details::auto_get<std::string_view>(L, 1) };
		argnum = L_argnum - 1;
		arg_begin = L_argnum - argnum + 1;
		break;
	case LUA_TUSERDATA:
		type = *static_cast<Type*>(L.checkudata(1, type_name<Type>().Data()));
		argnum = L_argnum - 1;
		arg_begin = L_argnum - argnum + 1;
		break;
	case LUA_TTABLE:
		// { type = "...", init_args = { args... } }
		if(L_argnum != 1)
			return L.error("%s::new : The 1st argument is table, so the number of arguments must be 1.", type_name<UDRefl::SharedObject>().Data());
		L.getfield(1, "type");
		type = details::auto_get<Type>(L, -1);
		if (auto init_args_type = L.getfield(1, "init_args")) {
			if(init_args_type != LUA_TTABLE)
				return L.error("%s::new : The type of init_args must be table.", type_name<UDRefl::SharedObject>().Data());
			int init_args_index = L.gettop();
			arg_begin = init_args_index + 1;
			argnum = static_cast<int>(L.lenL(init_args_index));
			L.checkstack(argnum);
			for (lua_Integer i = 1; i <= static_cast<lua_Integer>(argnum); i++)
				L.geti(init_args_index, i);
		}
		else {
			arg_begin = L_argnum - argnum + 1;
			argnum = 0;
		}
		
		break;
	default:
		return L.error("%s::new : The function doesn't support %s.",
			type_name<UDRefl::SharedObject>().Data(),
			L.typename_(1)
		);
	}

	details::ArgStack argstack;
	int error = details::FillArgStack(L, argstack, arg_begin, argnum);

	if (error) {
		return L.error("%s::new : FillArgStack Failed\n%s",
			type_name<UDRefl::SharedObject>().Data(), L.tostring(-1));
	}

	{ // get unsync resource
		int success = L.getfield(LUA_REGISTRYINDEX, details::UnsyncRsrc);
		assert(success);
	}
	auto* rsrc = (std::pmr::unsynchronized_pool_resource*)L.touserdata(-1);
	UDRefl::SharedObject obj;
	try {
		obj = UDRefl::Mngr.MMakeShared(
			type,
			rsrc,
			{
				argstack.argptr_buffer,
				{reinterpret_cast<Type*>(argstack.argType_buffer), static_cast<std::size_t>(argnum)}
			}
		);
	}
	catch (const std::runtime_error& e) {
		return L.error("%s::new : Fail.\n%s", type_name<UDRefl::SharedObject>().Data(), e.what());
	}
	catch (...) {
		return L.error("%s::new : Fail.", type_name<UDRefl::SharedObject>().Data());
	}

	if (!obj.GetType())
		return L.error("%s::new : Fail.", type_name<UDRefl::SharedObject>().Data());

	obj = { UDRefl::ObjectView{obj.GetType(), obj.GetPtr()},[rsrc_ref = LuaRef{L}, buffer = obj.GetBuffer()](void*){} };

	details::push(L, std::move(obj));

	return 1;
}

static int f_SharedObject_new_MethodPtr(lua_State* L_) {
	LuaStateView L{ L_ };

	int L_argnum = L.gettop();

	UDRefl::SharedObject methodptr_obj;
	LuaRef func_ref;
	Type result_type = Type_of<void>;
	Type object_type;
	UDRefl::ParamList list;

	// [object type, ]function[, result type = Type_of<void>][, ParamList = {}]

	constexpr auto GetParamList = [](LuaStateView L, int idx) -> UDRefl::ParamList {
		auto obj = details::auto_get<UDRefl::ObjectView>(L, idx).RemoveConst();
		if (!obj.GetType().Is<UDRefl::ParamList>()) {
			L.error(
				"%s::new_MethodPtr :"
				"The %dth arguments (%s) isn't ParamList.",
				type_name<UDRefl::SharedObject>().Data(), idx, obj.GetType().GetName().data());
			return {};
		}
		return obj.As<UDRefl::ParamList>();
	};

	switch (L_argnum) {
	case 1: // function
		L.checktype(1, LUA_TFUNCTION);
		func_ref = std::move(LuaRef{ L });
		break;
	case 2: // object type + function | function + result type | function + paramlist
		if (L.type(1) == LUA_TFUNCTION) { // function + result type | function + paramlist
			if (L.testudata(2, type_name<Type>().Data())) // function + result type
				result_type = details::auto_get<Type>(L, 2);
			else if (L.testudata(2, type_name<UDRefl::ParamList>().Data())) // function + paramlist
				list = GetParamList(L, 2);
			else {
				return L.error("%s::new_MethodPtr: The 2nd argument should be a Type/ParamList.",
					type_name<UDRefl::SharedObject>().Data());
			}
			L.pop(1);
			func_ref = std::move(LuaRef{ L });
		}
		else {
			// object type + function
			object_type = details::auto_get<Type>(L, 1);
			func_ref = std::move(LuaRef{ L });
		}
		break;
	case 3: // object type + function + result type | object type + function + paramlist | function + result type + paramlist
		if (L.type(1) == LUA_TFUNCTION) { // function + result type + paramlist
			result_type = details::auto_get<Type>(L, 2);
			list = GetParamList(L, 3);
			L.pop(2);
			func_ref = std::move(LuaRef{ L });
		}
		else { // object type + function + result type | object type + function + paramlist
			L.checktype(2, LUA_TFUNCTION);
			object_type = details::auto_get<Type>(L, 1);
			if (L.testudata(3, type_name<Type>().Data())) // object type + function + result type
				result_type = details::auto_get<Type>(L, 3);
			else if (L.testudata(3, type_name<UDRefl::ParamList>().Data())) // object type + function + paramlist
				list = GetParamList(L, 3);
			else {
				return L.error("%s::new_MethodPtr: The 2nd argument should be a Type/ParamList.",
					type_name<UDRefl::SharedObject>().Data());
			}
			L.pop(1);
			func_ref = std::move(LuaRef{ L });
		}
		break;
	case 4: // object type + function + result type + paramlist
		L.checktype(2, LUA_TFUNCTION);
		object_type = details::auto_get<Type>(L, 1);
		result_type = details::auto_get<Type>(L, 3);
		list = GetParamList(L, 4);
		L.pop(2);
		func_ref = std::move(LuaRef{ L });
		break;
	default:
		return L.error(
			"%s::new_MethodPtr :"
			"The number of arguments (%d) is invalid. The function needs 1~4 arguments([object type, ]function[, result type][, ParamList]).",
			type_name<UDRefl::SharedObject>().Data(), L_argnum);
	}

	assert(result_type);

	if (result_type.IsConst()) {
		return L.error(
			"%s::new_MethodPtr :"
			"The result type ($s) must be non-const.",
			type_name<UDRefl::SharedObject>().Data(), result_type.GetName().data());
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
			details::LuaFuncToMethodPtrFunc(
				UDRefl::Mngr.tregistry.RegisterAddLValueReference(object_type),
				flag,
				result_type,
				std::move(func_ref)
			),
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

static int f_UDRefl_box(lua_State* L_) {
	LuaStateView L{ L_ };
	const int L_argnum = L.gettop();
	if (L_argnum != 1)
		return L.error("%s::box : The number of arguments must be 1.", type_name<UDRefl::SharedObject>().Data());

	Type type;
	int argtype = L.type(1);
	switch (argtype)
	{
	case LUA_TNIL:
		details::push(L, UDRefl::Mngr.MakeShared(Type_of<std::nullptr_t>));
		break;
	case LUA_TBOOLEAN:
		details::push(L, UDRefl::Mngr.MakeShared(Type_of<bool>, UDRefl::TempArgsView{ details::auto_get<bool>(L, 1) }));
		break;
	case LUA_TLIGHTUSERDATA:
		details::push(L, UDRefl::Mngr.MakeShared(Type_of<void*>, UDRefl::TempArgsView{ details::auto_get<void*>(L, 1) }));
		break;
	case LUA_TNUMBER:
		if (L.isinteger(1))
			details::push(L, UDRefl::Mngr.MakeShared(Type_of<lua_Integer>, UDRefl::TempArgsView{ details::auto_get<lua_Integer>(L, 1) }));
		else {
			assert(L.isnumber(1));
			details::push(L, UDRefl::Mngr.MakeShared(Type_of<lua_Number>, UDRefl::TempArgsView{ details::auto_get<lua_Number>(L, 1) }));
		}
		break;
	case LUA_TSTRING:
		details::push(L, UDRefl::Mngr.MakeShared(Type_of<std::string>, UDRefl::TempArgsView{ details::auto_get<std::string_view>(L, 1) }));
		break;
	case LUA_TUSERDATA:
		if (L.testudata(1, type_name<Type>().Data()))
			details::push(L, UDRefl::Mngr.MakeShared(Type_of<Type>, UDRefl::TempArgsView{ details::auto_get<Type>(L, 1) }));
		else if (L.testudata(1, type_name<Name>().Data()))
			details::push(L, UDRefl::Mngr.MakeShared(Type_of<Name>, UDRefl::TempArgsView{ details::auto_get<Name>(L, 1) }));
		else {
			return L.error("%s::box : In userdata, only Type & Name can be boxed.",
				type_name<UDRefl::SharedObject>().Data()
			);
		}
		break;
	default:
		return L.error("%s::box : The type (%s) of the argument is not support.",
			type_name<UDRefl::SharedObject>().Data(),
			L.typename_(1)
		);
	}

	return 1;
}

static int f_UDRefl_unbox(lua_State* L_) {
	LuaStateView L{ L_ };

	if (L.gettop() != 1)
		return L.error("%s::__unbox : The number of arguments is invalid. The function needs 1 argument (object).", type_name<UDRefl::ObjectView>().Data());

	auto ptr = details::auto_get<UDRefl::ObjectView>(L, 1);

	if (!ptr.GetPtr())
		return L.error("%s::__unbox : The object is nil.", type_name<UDRefl::ObjectView>().Data());

	ptr = ptr.RemoveConstReference();

	switch (ptr.GetType().GetID().GetValue())
	{
	case TypeID_of<bool>.GetValue():
		L.pushboolean(ptr.As<bool>());
		break;
	case TypeID_of<std::int8_t>.GetValue():
		L.pushinteger(static_cast<lua_Integer>(ptr.As<std::int8_t>()));
		break;
	case TypeID_of<std::int16_t>.GetValue():
		L.pushinteger(static_cast<lua_Integer>(ptr.As<std::int16_t>()));
		break;
	case TypeID_of<std::int32_t>.GetValue():
		L.pushinteger(static_cast<lua_Integer>(ptr.As<std::int32_t>()));
		break;
	case TypeID_of<std::int64_t>.GetValue():
		L.pushinteger(static_cast<lua_Integer>(ptr.As<std::int64_t>()));
		break;
	case TypeID_of<std::uint8_t>.GetValue():
		L.pushinteger(static_cast<lua_Integer>(ptr.As<std::uint8_t>()));
		break;
	case TypeID_of<std::uint16_t>.GetValue():
		L.pushinteger(static_cast<lua_Integer>(ptr.As<std::uint16_t>()));
		break;
	case TypeID_of<std::uint32_t>.GetValue():
		L.pushinteger(static_cast<lua_Integer>(ptr.As<std::uint32_t>()));
		break;
	case TypeID_of<std::uint64_t>.GetValue():
		L.pushinteger(static_cast<lua_Integer>(ptr.As<std::uint64_t>()));
		break;
	case TypeID_of<float>.GetValue():
		L.pushnumber(static_cast<lua_Number>(ptr.As<float>()));
		break;
	case TypeID_of<double>.GetValue():
		L.pushnumber(static_cast<lua_Number>(ptr.As<double>()));
		break;
	case TypeID_of<void*>.GetValue():
		L.pushlightuserdata(ptr.As<void*>());
		break;
	case TypeID_of<std::nullptr_t>.GetValue():
		L.pushnil();
		break;
	case TypeID_of<Type>.GetValue():
		details::push(L, ptr.As<Type>());
		break;
	case TypeID_of<Name>.GetValue():
		details::push(L, ptr.As<Name>());
		break;
	case TypeID_of<const char*>.GetValue():
		L.pushstring(ptr.As<const char*>());
		break;
	case TypeID_of<char*>.GetValue():
		L.pushstring(ptr.As<char*>());
		break;
	case TypeID_of<std::string_view>.GetValue():
		details::push(L, ptr.As<std::string_view>());
		break;
	case TypeID_of<std::string>.GetValue():
		details::push<std::string_view>(L, ptr.As<std::string>());
		break;
	case TypeID_of<UDRefl::ObjectView>.GetValue():
		details::push<UDRefl::ObjectView>(L, ptr.As<UDRefl::ObjectView>());
		break;
	case TypeID_of<UDRefl::SharedObject>.GetValue():
		details::push<UDRefl::SharedObject>(L, ptr.As<UDRefl::SharedObject>());
		break;
	default:
		return L.error("%s::__unbox : The type (%s) can't unbox.",
			type_name<UDRefl::ObjectView>().Data(),
			ptr.GetType().GetName().data());
		break;
	}

	return 1;
}

static int f_UDRefl_RegisterType(lua_State* L_) {
	LuaStateView L{ L_ };
	if (L.gettop() != 1)
		return L.error("UDRefl::RegisterType : The number of arguments must be 1");
	L.checktype(1, LUA_TTABLE);

	Type type;
	UDRefl::AttrSet type_attrs;

	std::vector<Type> bases;

	std::vector<Name> field_names;
	std::vector<Type> field_types;
	std::map<std::size_t, UDRefl::AttrSet> fields_attrs;

	std::vector<Name> method_names;
	std::vector<UDRefl::MethodPtr> methodptrs;
	std::map<std::size_t, UDRefl::AttrSet> methods_attrs;

	std::vector<Name> unowned_field_names;
	std::vector<UDRefl::SharedObject> unowned_field_objs;
	std::map<std::size_t, UDRefl::AttrSet> unowned_fields_attrs;

	{ // name
		L.getfield(1, "type");
		type = details::auto_get<Type>(L, -1);
		if (int attrs_type = L.getfield(1, "attrs")) {
			if (attrs_type != LUA_TTABLE)
				return L.error("UDRefl::RegisterType : attrs must be a table");

			lua_Integer attr_num = L.lenL(-1);
			L.pushcfunction(f_SharedObject_new);
			for (lua_Integer i = 1; i <= attr_num; i++) {
				L.pushvalue(-1); // SharedObject.new
				L.geti(-3, i); // attrs[i]
				int error = L.pcall(1, 1, 0); // SharedObject.new({...})
				if (error) {
					return L.error("UDRefl::RegisterType: Call %s::new failed.\n%s",
						type_name<UDRefl::SharedObject>().Data(),
						L.tostring(-1));
				}
				auto attr = details::auto_get<UDRefl::SharedObject>(L, -1);
				auto target = type_attrs.find(attr);
				if(target != type_attrs.end())
					return L.error("UDRefl::RegisterType: Same attr (%s)of type.\n%s", attr.GetType().GetName());
				type_attrs.insert(target, attr);
				L.pop(1); // SharedObject
			}
			L.pop(1); // f_SharedObject_new
		}
		L.pop(2); // bases, attrs
	}

	do{ // bases
		auto type = L.getfield(1, "bases");
		if (type == LUA_TNIL)
			break;

		if (type != LUA_TTABLE)
			return L.error("UDRefl::RegisterType : table's bases must be a table");

		lua_Integer len = L.lenL(-1);
		for (lua_Integer i = 1; i <= len; i++) {
			L.geti(-1, i);
			bases.push_back(details::auto_get<Type>(L, -1));
			L.pop(1);
		}
		L.pop(1); // bases
	} while (false);

	do { // fields
		auto type = L.getfield(1, "fields");
		if (type == LUA_TNIL)
			break;

		if (type != LUA_TTABLE)
			return L.error("UDRefl::RegisterType : table's fields must be a table");
		int fields_index = L.gettop();
		lua_Integer len = L.lenL(fields_index);
		field_types.reserve(len);
		field_names.reserve(len);
		for (lua_Integer i = 1; i <= len; i++) {
			if (L.geti(fields_index, i) != LUA_TTABLE)
				return L.error("UDRefl::RegisterType : element of table's fields must be a table");
			int field_index = L.gettop();
			L.getfield(field_index, "type");
			field_types.push_back(details::auto_get<Type>(L, -1));
			L.getfield(field_index, "name");
			Name field_name = details::auto_get<Name>(L, -1);
			field_names.push_back(field_name);

			if (int attrs_type = L.getfield(field_index, "attrs")) {
				if (attrs_type != LUA_TTABLE)
					return L.error("UDRefl::RegisterType : attrs must be a table");

				lua_Integer attr_num = L.lenL(-1);
				L.pushcfunction(f_SharedObject_new);
				for (lua_Integer i = 1; i <= attr_num; i++) {
					L.pushvalue(-1); // SharedObject.new
					L.geti(-3, i); // attrs[i]
					int error = L.pcall(1, 1, 0); // SharedObject.new({...})
					if (error) {
						return L.error("UDRefl::RegisterType: Call %s::new failed.\n%s",
							type_name<UDRefl::SharedObject>().Data(),
							L.tostring(-1));
					}
					auto attr = details::auto_get<UDRefl::SharedObject>(L, -1);
					auto& field_i_attrs = fields_attrs[i-1];
					auto target = field_i_attrs.find(attr);
					if (target != field_i_attrs.end()) {
						return L.error("UDRefl::RegisterType: Same attr (%s)of field(%s).\n%s",
							attr.GetType().GetName(),
							field_name.GetView().data());
					}
					field_i_attrs.insert(target, attr);
					L.pop(1); // SharedObject
				}
				L.pop(1); // f_SharedObject_new
			}

			L.pop(4); // table, type, name, attrs
		}

		L.pop(1); // fields
	} while (false);

	bool contains_ctor = false;
	bool contains_dtor = false;
	do { // methods
		auto ftype = L.getfield(1, "methods");
		if (ftype == LUA_TNIL)
			break;

		if (ftype != LUA_TTABLE)
			return L.error("UDRefl::RegisterType : table's methods must be a table");

		lua_Integer mlen = L.lenL(-1);
		method_names.reserve(mlen);
		methodptrs.reserve(mlen);
		for (lua_Integer i = 1; i <= mlen; i++) {
			if (L.geti(-1, i) != LUA_TTABLE)
				return L.error("UDRefl::RegisterType : element of table's methods must be a table");
			int methodidx = L.gettop();
			L.getfield(methodidx, "name");
			Name method_name = details::auto_get<Name>(L, -1);
			if (method_name == UDRefl::NameIDRegistry::Meta::ctor)
				contains_ctor = true;
			else if (method_name == UDRefl::NameIDRegistry::Meta::dtor)
				contains_dtor = true;

			method_names.push_back(method_name);

			UDRefl::MethodFlag flag;
			if (L.getfield(methodidx, "flag") != LUA_TNIL) {
				auto flagname = details::auto_get<std::string_view>(L, -1);
				if (flagname == "Variable")
					flag = UDRefl::MethodFlag::Variable;
				else if (flagname == "Const")
					flag = UDRefl::MethodFlag::Const;
				else if (flagname == "Static")
					flag = UDRefl::MethodFlag::Static;
				else
					return L.error("UDRefl::RegisterType : flag(%s) of table's methods[%I] must be a Variable/Const/Static.", flagname.data(), i);
			}
			else
				flag = UDRefl::MethodFlag::Variable;

			Type result_type;
			if (L.getfield(methodidx, "result") != LUA_TNIL)
				result_type = details::auto_get<Type>(L, -1);
			else
				result_type = Type_of<void>;

			if (L.getfield(methodidx, "body") != LUA_TFUNCTION)
				return L.error("UDRefl::RegisterType : body of table's methods[%I] must be a function.", i);
			LuaRef func_ref{ L }; // pop

			UDRefl::ParamList params;
			if (auto fparamstype = L.getfield(-4, "params"); fparamstype != LUA_TNIL) {
				if (fparamstype != LUA_TTABLE)
					return L.error("UDRefl::RegisterType : params of table's methods[%I] must be a table", i);
				lua_Integer len = L.lenL(-1);
				params.reserve(len);
				for (lua_Integer i = 1; i <= len; i++) {
					L.geti(-1, i);
					params.push_back(details::auto_get<Type>(L, -1));
					L.pop(1);
				}
			}
			methodptrs.emplace_back(
				details::LuaFuncToMethodPtrFunc(
					UDRefl::Mngr.tregistry.RegisterAddLValueReference(type),
					flag,
					result_type,
					std::move(func_ref)
				),
				flag,
				result_type,
				std::move(params));

			if (int attrs_type = L.getfield(methodidx, "attrs")) {
				if (attrs_type != LUA_TTABLE)
					return L.error("UDRefl::RegisterType : attrs must be a table");

				lua_Integer attr_num = L.lenL(-1);
				L.pushcfunction(f_SharedObject_new);
				for (lua_Integer i = 1; i <= attr_num; i++) {
					L.pushvalue(-1); // SharedObject.new
					L.geti(-3, i); // attrs[i]
					int error = L.pcall(1, 1, 0); // SharedObject.new({...})
					if (error) {
						return L.error("UDRefl::RegisterType: Call %s::new failed.\n%s",
							type_name<UDRefl::SharedObject>().Data(),
							L.tostring(-1));
					}
					auto attr = details::auto_get<UDRefl::SharedObject>(L, -1);
					auto& method_i_attrs = methods_attrs[i - 1];
					auto target = method_i_attrs.find(attr);
					if (target != method_i_attrs.end()) {
						return L.error("UDRefl::RegisterType: Same attr (%s)of method(%s).\n%s",
							attr.GetType().GetName(),
							method_name.GetView().data());
					}
					method_i_attrs.insert(target, attr);
					L.pop(1); // SharedObject
				}
				L.pop(1); // f_SharedObject_new
			}

			L.pop(6); // method, name, flag, result, body, attrs
		}
	} while (false);

	do { // unowned_fields
		auto ftype = L.getfield(1, "unowned_fields");
		if (ftype == LUA_TNIL)
			break;

		if (ftype != LUA_TTABLE)
			return L.error("UDRefl::RegisterType : table's unowned_fields must be a table");

		int unowned_fields_index = L.gettop();

		{ // get unsync resource
			int success = L.getfield(LUA_REGISTRYINDEX, details::UnsyncRsrc);
			assert(success);
		}
		auto* rsrc = (std::pmr::unsynchronized_pool_resource*)L.touserdata(-1);

		lua_Integer flen = L.lenL(unowned_fields_index);
		unowned_field_names.reserve(flen);
		unowned_field_objs.reserve(flen);
		for (lua_Integer i = 1; i <= flen; i++) {
			if (L.geti(unowned_fields_index, i) != LUA_TTABLE)
				return L.error("UDRefl::RegisterType : element of table's unowned_fields must be a table");

			int unowned_field_index = L.gettop();

			L.getfield(unowned_field_index, "name");
			Name unowned_field_name = details::auto_get<Name>(L, -1);
			unowned_field_names.push_back(unowned_field_name);

			L.pushcfunction(f_SharedObject_new);
			L.pushvalue(unowned_field_index);
			int error = L.pcall(1, 1, 0); // SharedObject.new({...})
			if (error) {
				return L.error("UDRefl::RegisterType: Call %s::new failed.\n%s",
					type_name<UDRefl::SharedObject>().Data(),
					L.tostring(-1));
			}
			unowned_field_objs.push_back(details::auto_get<UDRefl::SharedObject>(L, -1));

			if (int attrs_type = L.getfield(unowned_field_index, "attrs")) {
				if (attrs_type != LUA_TTABLE)
					return L.error("UDRefl::RegisterType : attrs must be a table");

				lua_Integer attr_num = L.lenL(-1);
				L.pushcfunction(f_SharedObject_new);
				for (lua_Integer i = 1; i <= attr_num; i++) {
					L.pushvalue(-1); // SharedObject.new
					L.geti(-3, i); // attrs[i]
					int error = L.pcall(1, 1, 0); // SharedObject.new({...})
					if (error) {
						return L.error("UDRefl::RegisterType: Call %s::new failed.\n%s",
							type_name<UDRefl::SharedObject>().Data(),
							L.tostring(-1));
					}
					auto attr = details::auto_get<UDRefl::SharedObject>(L, -1);
					auto& unowned_field_i_attrs = unowned_fields_attrs[i - 1];
					auto target = unowned_field_i_attrs.find(attr);
					if (target != unowned_field_i_attrs.end()) {
						return L.error("UDRefl::RegisterType: Same attr (%s)of unowned_field(%s).\n%s",
							attr.GetType().GetName(),
							unowned_field_name.GetView().data());
					}
					unowned_field_i_attrs.insert(target, attr);
					L.pop(1); // SharedObject
				}
				L.pop(1); // f_SharedObject_new
			}

			L.pop(2); // table, name
		}
	} while (false);

	Type rst = UDRefl::Mngr.RegisterType(type, bases, field_types, field_names, !contains_ctor && !contains_dtor);
	if(!rst)
		return L.error("UDRefl::RegisterType : Call Ubpa::UDRefl::ReflMngr::RegisterType failed.");

	for (std::size_t i = 0; i < method_names.size(); i++) {
		Name mrst = UDRefl::Mngr.AddMethod(type, method_names[i], UDRefl::MethodInfo{ std::move(methodptrs[i]) });
		if (!mrst) {
			UDRefl::Mngr.typeinfos.erase(rst);
			return L.error("UDRefl::RegisterType : Call Ubpa::UDRefl::ReflMngr::AddMethod for %s failed.", method_names[i].GetView().data());
		}
	}

	if (!contains_ctor)
		UDRefl::Mngr.AddDefaultConstructor(type);
	if (!contains_dtor)
		UDRefl::Mngr.AddDestructor(type);

	for (std::size_t i = 0; i < unowned_field_names.size(); i++) {
		Name frst = UDRefl::Mngr.AddField(type, unowned_field_names[i], UDRefl::FieldInfo{ UDRefl::FieldPtr{std::move(unowned_field_objs[i])} });
		if (!frst) {
			UDRefl::Mngr.typeinfos.erase(rst);
			return L.error("UDRefl::RegisterType : Call Ubpa::UDRefl::ReflMngr::AddField for unowned %s failed.", unowned_field_names[i].GetView().data());
		}
	}

	for (auto attr : type_attrs)
		UDRefl::Mngr.AddTypeAttr(type, attr);

	for (const auto& [idx, attrs] : fields_attrs) {
		for (auto attr : attrs)
			UDRefl::Mngr.AddFieldAttr(type, field_names[idx], attr);
	}

	for (const auto& [idx, attrs] : methods_attrs) {
		for (auto attr : attrs)
			UDRefl::Mngr.AddMethodAttr(type, method_names[idx], attr);
	}

	for (const auto& [idx, attrs] : unowned_fields_attrs) {
		for (auto attr : attrs)
			UDRefl::Mngr.AddFieldAttr(type, unowned_field_names[idx], attr);
	}

	details::push(L, rst);
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
	"new", f_ObjectView_new,
	"range", f_ObjectView_range,
	"tuple_bind", f_ObjectView_tuple_bind,
	NULL, NULL
};

static const struct luaL_Reg meta_ObjectView[] = {
	"__add", &f_meta<UDRefl::ObjectView, details::Meta::t_operator_add, 2, details::Invalid, true>,
	"__band", &f_meta<UDRefl::ObjectView, details::Meta::t_operator_band, 2, details::Invalid, true>,
	"__bnot", &f_meta<UDRefl::ObjectView, details::Meta::t_operator_bnot, 1>,
	"__bor", &f_meta<UDRefl::ObjectView, details::Meta::t_operator_bor, 2, details::Invalid, true>,
	"__bxor", &f_meta<UDRefl::ObjectView, details::Meta::t_operator_bxor, 2, details::Invalid, true>,
	"__call",& f_meta<UDRefl::ObjectView, details::Meta::t_operator_call>,
	"__concat", f_ObjectView_concat,
	"__div", &f_meta<UDRefl::ObjectView, details::Meta::t_operator_div, 2>,
	"__eq", &f_meta<UDRefl::ObjectView, details::Meta::t_operator_eq, 2, bool, true>,
	"__index", f_ObjectView_index,
	"__le", &f_meta<UDRefl::ObjectView, details::Meta::t_operator_le, 2, bool>,
	"__lt", &f_meta<UDRefl::ObjectView, details::Meta::t_operator_lt, 2, bool>,
	"__mod", &f_meta<UDRefl::ObjectView, details::Meta::t_operator_mod, 2>,
	"__mul", &f_meta<UDRefl::ObjectView, details::Meta::t_operator_mul, 2, details::Invalid, true>,
	"__newindex", f_ObjectView_newindex,
	// ^ is bxor in c++
	"__pow",& f_meta<UDRefl::ObjectView, details::Meta::t_operator_bxor, 2, details::Invalid, true>,
	"__shl", &f_meta<UDRefl::ObjectView, details::Meta::t_operator_shl, 2>,
	"__shr", &f_meta<UDRefl::ObjectView, details::Meta::t_operator_shr, 2>,
	"__sub", &f_meta<UDRefl::ObjectView, details::Meta::t_operator_sub, 2>,
	"__unbox", f_UDRefl_unbox,
	"__tostring", f_ObjectView_tostring,
	"__unm", &f_meta<UDRefl::ObjectView, details::Meta::t_operator_sub, 1>,

	"__assignment",& f_meta<UDRefl::ObjectView, details::Meta::t_operator_assignment, 2>,
	"__begin",& f_meta<UDRefl::ObjectView, details::Meta::t_container_begin, 1>,
	"__end",& f_meta<UDRefl::ObjectView, details::Meta::t_container_end, 1>,
	"__range", f_ObjectView_range,
	"__tuple_bind", f_ObjectView_tuple_bind,

	NULL      , NULL
};

static const struct luaL_Reg lib_SharedObject[] = {
	"new", f_SharedObject_new,
	"new_MethodPtr", f_SharedObject_new_MethodPtr,
	NULL, NULL
};

static const struct luaL_Reg lib_UDRefl[] = {
	"RegisterType", f_UDRefl_RegisterType,
	"box", f_UDRefl_box,
	"unbox", f_UDRefl_unbox,
	NULL, NULL
};

static const struct luaL_Reg* meta_SharedObject = meta_ObjectView;

static void init_CallHandle(lua_State* L_) {
	LuaStateView L{ L_ };

	L.newmetatable(type_name<details::CallHandle>().Data());
	L.pushcfunction(&f_meta<details::CallHandle, details::Meta::t_operator_call>);
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
		Ubpa_UDRefl_ext_Bootstrap();
		void* buffer = L.newuserdata(sizeof(UDRefl::ObjectView));
		new(buffer) UDRefl::ObjectView{ UDRefl::MngrView };
		L.setmetatable(type_name<UDRefl::ObjectView>().Data());
		L.setfield(-2, "ReflMngr");
	}
	return 1;
}

static int luaopen_SharedObject(lua_State* L_) {
	LuaStateView L{ L_ };
	L.newmetatable(type_name<UDRefl::SharedObject>().Data());
	L.pushcfunction(details::wrap_dtor<UDRefl::SharedObject>());
	L.setfield(-2, "__gc");
	L.setfuncs(meta_SharedObject, 0);
	L.newlib(lib_SharedObject);
	return 1;
}

static int luaopen_UDRefl(lua_State* L_) {
	LuaStateView L{ L_ };
	L.newlib(lib_UDRefl);
	return 1;
}

static const luaL_Reg UDRefl_libs[] = {
	{"Name", luaopen_Name},
	{"Type", luaopen_Type},
	{"ObjectView", luaopen_ObjectView},
	{"SharedObject", luaopen_SharedObject},
	{"UDRefl", luaopen_UDRefl},
	{NULL, NULL}
};

UDLua_core_API void luaopen_UDLua(lua_State* L_) {
	init_CallHandle(L_);

	LuaStateView L{ L_ };

	void* buffer = L.newuserdata(sizeof(std::pmr::unsynchronized_pool_resource));
	new (buffer) std::pmr::unsynchronized_pool_resource{};
	int success = L.newmetatable(type_name<std::pmr::unsynchronized_pool_resource>().Data());
	L.pushcfunction(details::wrap_dtor<std::pmr::unsynchronized_pool_resource>());
	L.setfield(-2, "__gc");
	L.setmetatable(-2);
	L.setfield(LUA_REGISTRYINDEX, details::UnsyncRsrc);
	L.pop(1);

	const luaL_Reg* lib;
	// "require" functions from 'UDRefl_libs' and set results to global table
	for (lib = UDRefl_libs; lib->func; lib++) {
		L.requiref(lib->name, lib->func, 1);
		lua_pop(L, 1); // remove lib
	}
}
