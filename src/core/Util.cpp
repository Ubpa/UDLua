#include "Util.h"

using namespace Ubpa;

//void Ubpa::details::stack_dump(lua_State* L) {
//	printf("\n------ stack dump begin ------\n");
//	for (int i = 1; i <= lua_gettop(L); ++i) {
//		int t = lua_type(L, i);
//		switch (t) {
//		case LUA_TNONE: {
//			printf("LUA_TNONE\n");
//		}break;
//
//		case LUA_TNIL: {
//			printf("LUA_TNIL\n");
//		}break;
//
//		case LUA_TBOOLEAN: {
//			printf("LUA_TBOOLEAN : %s\n", lua_toboolean(L, i) ? "true" : "false");
//		}break;
//
//		case LUA_TLIGHTUSERDATA: {
//			printf("LUA_TLIGHTUSERDATA\n");
//		}break;
//
//		case LUA_TNUMBER: {
//			if (lua_isinteger(L, i)) {
//				printf("LUA_TNUMBER integer : %lld \n", lua_tointeger(L, i));
//			}
//			else if (lua_isnumber(L, i)) {
//				printf("LUA_TNUMBER number: %g\n", lua_tonumber(L, i));
//			}
//		}break;
//
//		case LUA_TSTRING: {
//			printf("LUA_TSTRING : %s\n", lua_tostring(L, i));
//		}break;
//
//		case LUA_TTABLE: {
//			printf("LUA_TTABLE\n");
//		}break;
//
//		case LUA_TFUNCTION: {
//			printf("LUA_TFUNCTION\n");
//		}break;
//
//		case LUA_TUSERDATA: {
//			printf("LUA_TUSERDATA\n");
//		}break;
//
//		case LUA_TTHREAD: {
//			printf("LUA_TTHREAD\n");
//		}break;
//
//		case LUA_NUMTAGS: {
//			printf("LUA_NUMTAGS\n");
//		}break;
//
//		default: {
//			printf("%s\n", lua_typename(L, t));
//		}break;
//		}
//	}
//
//	std::cout << "------ stack dump end ------" << std::endl;
//}

UDRefl::ObjectView* Ubpa::details::safe_get_Object(LuaStateView L, int idx) {
	void* p = lua_touserdata(L, idx);
	if (p != nullptr) {  /* value is a userdata? */
		if (lua_getmetatable(L, idx)) {  /* does it have a metatable? */
			luaL_getmetatable(L, type_name<UDRefl::ObjectView>().Data());  /* get correct metatable */
			if (!lua_rawequal(L, -1, -2)) { /* not the same? */
				luaL_getmetatable(L, type_name<UDRefl::SharedObject>().Data());  /* get correct metatable */
				if (!lua_rawequal(L, -1, -3)) /* not the same? */
					p = nullptr;  /* value is a userdata with wrong metatable */
				lua_pop(L, 3);  /* remove all metatables */
				p = static_cast<UDRefl::ObjectView*>((UDRefl::SharedObject*)p);
			}
			else
				lua_pop(L, 2);  /* remove both metatables */
			return (UDRefl::ObjectView*)p;
		}
	}
	return nullptr;
}

int Ubpa::details::FillArgStack(LuaStateView L, ArgStack& stack, int begin, int cnt) {
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
			// TODO: speed up
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
			L.pushfstring("Ubpa::details::FillArgStack : ArgStack doesn't support %s.", L.typename_(arg));
			return 1;
		}
	}

	return 0;
}

UDRefl::MethodPtr::Func Ubpa::details::LuaFuncToMethodPtrFunc(Type object_type, UDRefl::MethodFlag flag, Type result_type, LuaRef func_ref) {
	assert(UDRefl::enum_single(flag));
	Type ref_obj_type = flag == UDRefl::MethodFlag::Static ? Type{}
		: (flag == UDRefl::MethodFlag::Const ? UDRefl::Mngr.tregistry.RegisterAddConstLValueReference(object_type)
			: UDRefl::Mngr.tregistry.RegisterAddLValueReference(object_type));
	return
		[
			ref_obj_type,
			result_type,
			fref = std::make_shared<LuaRef>(std::move(func_ref))
		] (void* obj, void* result_buffer, UDRefl::ArgsView args) mutable
	{
		auto L = fref->GetView();
		int top = L.gettop();
		fref->Get();
		const int n = static_cast<int>(args.Types().size());
		int callargnum;
		if (ref_obj_type.Valid()) {
			callargnum = n + 1;
			L.checkstack(callargnum);
			push(L, UDRefl::ObjectView{ ref_obj_type, obj });
		}
		else {
			callargnum = n;
			L.checkstack(callargnum);
		}

		for (std::size_t i = 0; i < n; i++)
			push(L, args[i]);
		int error = L.pcall(callargnum, LUA_MULTRET, 0);
		int result_construct_argnum = L.gettop() - top;
		details::LuaStackPopGuard popguard{ L, result_construct_argnum };
		if (error) {
			std::stringstream ss;
			ss << type_name<UDRefl::SharedObject>().View() << "::new_MethodPtr::lambda:\n" << auto_get<std::string_view>(L, -1);
			std::string str = ss.str();
			std::runtime_error except{ str.data() };
			L.pop(1);
			throw except;
		}

		if (!result_buffer || result_type.IsVoid())
			return;

		if (result_type.IsReference()) {
			if (result_construct_argnum != 1) {
				std::stringstream ss;
				ss
					<< type_name<UDRefl::SharedObject>().View()
					<< "::new_MethodPtr::lambda: The result type is reference, so the number ("
					<< result_construct_argnum
					<< ") of return values must be 1"
					;
				std::string str = ss.str();
				std::runtime_error except{ str.data() };
				throw except;
			}
			UDRefl::ObjectView return_obj;
			if (void* obj = L.testudata(-1, type_name<UDRefl::ObjectView>().Data()))
				return_obj = *reinterpret_cast<UDRefl::ObjectView*>(obj);
			else if (void* obj = L.testudata(-1, type_name<UDRefl::SharedObject>().Data())) {
				auto* sobj = reinterpret_cast<UDRefl::SharedObject*>(obj);
				return_obj = { sobj->GetType(), sobj->GetPtr() };
			}
			else {
				std::stringstream ss;
				ss
					<< type_name<UDRefl::SharedObject>().View()
					<< "::new_MethodPtr::lambda: The result type is reference, so the return type must be a ObjectView/SharedObject"
					;
				std::string str = ss.str();
				std::runtime_error except{ str.data() };
				throw except;
			}

			if (!UDRefl::is_ref_compatible(result_type, result_type)) {
				std::stringstream ss;
				ss
					<< type_name<UDRefl::SharedObject>().View()
					<< "::new_MethodPtr::lambda: The result type is reference, but result type ("
					<< result_type.GetName()
					<< ") is not compatible with return type ("
					<< return_obj.GetType().GetName()
					<< ")"
					;
				std::string str = ss.str();
				std::runtime_error except{ str.data() };
				throw except;
			}

			UDRefl::buffer_as<void*>(result_buffer) = return_obj.GetPtr();
			return;
		}

		ArgStack argstack;
		{ // fill argstack
			int error = details::FillArgStack(L, argstack, top + 1, result_construct_argnum);
			if (error) {
				std::stringstream ss;
				ss << type_name<UDRefl::SharedObject>().View() << "::new_MethodPtr::lambda:\n" << auto_get<std::string_view>(L, -1);
				std::string str = ss.str();
				std::runtime_error except{ str.data() };
				L.pop(1);
				throw except;
			}
		}

		{ // construct result
			bool success = UDRefl::Mngr.Construct(
				UDRefl::ObjectView{ result_type, result_buffer },
						{
							argstack.argptr_buffer,
							{reinterpret_cast<Type*>(argstack.argType_buffer), static_cast<std::size_t>(result_construct_argnum)}
						}
						);
			if (!success) {
				std::stringstream ss;
				ss << type_name<UDRefl::SharedObject>().View() << "::new_MethodPtr::lambda: Construct fail.";
				std::string str = ss.str();
				std::runtime_error except{ str.data() };
				L.pop(1);
				throw except;
			}
		}
	};
}
