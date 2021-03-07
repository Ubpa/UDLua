#include <UDLua/DLua.h>

#include <ULuaW/ULuaW.h>

#include <UDRefl/UDRefl.h>
#include <UDRefl_ext/Bootstrap.h>

#include <stdexcept>

using namespace Ubpa;

// Meta
namespace Ubpa::details {
	namespace Meta {
		static constexpr auto operator_add = TSTR("__add");
		static constexpr auto operator_band = TSTR("__band");
		static constexpr auto operator_bnot = TSTR("__bnot");
		static constexpr auto operator_bor = TSTR("__bor");
		static constexpr auto operator_bxor = TSTR("__bxor");
		static constexpr auto operator_call = TSTR("__call");
		static constexpr auto operator_div = TSTR("__div");
		static constexpr auto operator_eq = TSTR("__eq");
		static constexpr auto operator_le = TSTR("__le");
		static constexpr auto operator_lt = TSTR("__lt");
		static constexpr auto operator_mod = TSTR("__mod");
		static constexpr auto operator_mul = TSTR("__mul");
		static constexpr auto operator_shl = TSTR("__shl");
		static constexpr auto operator_shr = TSTR("__shr");
		static constexpr auto operator_sub = TSTR("__sub");
		static constexpr auto operator_assignment = TSTR("__assignment");
		static constexpr auto operator_subscript = TSTR("__subscript");

		static constexpr auto container_begin = TSTR("__begin");
		static constexpr auto container_end = TSTR("__end");

		using t_operator_add = decltype(operator_add);
		using t_operator_band = decltype(operator_band);
		using t_operator_bnot = decltype(operator_bnot);
		using t_operator_bor = decltype(operator_bor);
		using t_operator_bxor = decltype(operator_bxor);
		using t_operator_call = decltype(operator_call);
		using t_operator_div = decltype(operator_div);
		using t_operator_eq = decltype(operator_eq);
		using t_operator_le = decltype(operator_le);
		using t_operator_lt = decltype(operator_lt);
		using t_operator_mod = decltype(operator_mod);
		using t_operator_mul = decltype(operator_mul);
		using t_operator_shl = decltype(operator_shl);
		using t_operator_shr = decltype(operator_shr);
		using t_operator_sub = decltype(operator_sub);
		using t_operator_assignment = decltype(operator_assignment);
		using t_operator_subscript = decltype(operator_subscript);

		using t_container_begin = decltype(container_begin);
		using t_container_end = decltype(container_end);
	}


	namespace ObjectMeta {
		static constexpr auto GetTypeFieldVars = TSTR("GetTypeFieldVars");
		static constexpr auto GetVars = TSTR("GetVars");

		using t_GetTypeFieldVars = decltype(GetTypeFieldVars);
		using t_GetVars = decltype(GetVars);
	}
}

//static void stack_dump(lua_State* L) {
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
//			//����
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

namespace Ubpa::details {
	static constexpr const char UnsyncRsrc[] = "UDRefl::UnsyncRsrc";

	class LuaStackPopGuard {
	public:
		LuaStackPopGuard(LuaStateView L, int n) : L{ L }, n { n } {}
		~LuaStackPopGuard() { L.pop(n); }
	private:
		LuaStateView L;
		int n;
	};

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
	constexpr lua_CFunction wrap_dtor() {
		return [](lua_State* L_) -> int {
			LuaStateView L{ L_ };
			auto* ptr = (T*)L.checkudata(1, type_name<T>().Data());
			ptr->~T();
			// std::cout << "[debug] dtor: " << type_name<T>().View() << std::endl;
			return 0;
		};
	}

	UDRefl::ObjectView* safe_get_Object(LuaStateView L, int idx) {
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

	template<typename T>
	T* auto_get_userdata_ptr(LuaStateView L, int idx) {
		if constexpr (std::is_same_v<T, UDRefl::ObjectView>)
			return safe_get_Object(L, idx);
		else
			return (T*)L.checkudata(idx, type_name<T>().Data());
	}

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
			return nullptr;
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
			return *auto_get_userdata_ptr<T>(L, idx);
	}

	template<typename Ret>
	void push(LuaStateView L, Ret rst) {
		if constexpr (std::is_reference_v<Ret>) {
			if constexpr (std::is_const_v<std::remove_reference_t<Ret>>)
				push(L, static_cast<std::remove_cvref_t<Ret>>(rst));
			else
				static_assert(always_false<Ret>);
		}
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
				L.pushfstring("Ubpa::details::FillArgStack : ArgStack doesn't support %s.", L.typename_(arg) );
				return 1;
			}
		}

		return 0;
	}

	UDRefl::MethodPtr::Func LuaFuncToMethodPtrFunc(Type object_type, UDRefl::MethodFlag flag, Type result_type, LuaRef func_ref) {
		assert(UDRefl::enum_single(flag));
		Type ref_obj_type = flag == UDRefl::MethodFlag::Static ? Type{}
		: (flag == UDRefl::MethodFlag::Const ? UDRefl::Mngr.tregistry.RegisterAddConstLValueReference(object_type)
			: UDRefl::Mngr.tregistry.RegisterAddLValueReference(object_type));
		return
			[
				ref_obj_type,
				flag, result_type,
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

	int f_SharedObject_new_MethodPtr(lua_State* L_) {
		LuaStateView L{ L_ };

		int L_argnum = L.gettop();

		UDRefl::SharedObject methodptr_obj;
		LuaRef func_ref;
		Type result_type = Type_of<void>;
		Type object_type;
		UDRefl::ParamList list;

		// [object type, ]function[, result type = Type_of<void>][, ParamList = {}]

		constexpr auto GetParamList = [](LuaStateView L, int idx) -> UDRefl::ParamList {
			auto obj = auto_get<UDRefl::ObjectView>(L, idx).RemoveConst();
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
					result_type = auto_get<Type>(L, 2);
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
				object_type = auto_get<Type>(L, 1);
				func_ref = std::move(LuaRef{ L });
			}
			break;
		case 3: // object type + function + result type | object type + function + paramlist | function + result type + paramlist
			if (L.type(1) == LUA_TFUNCTION) { // function + result type + paramlist
				result_type = auto_get<Type>(L, 2);
				list = GetParamList(L, 3);
				L.pop(2);
				func_ref = std::move(LuaRef{ L });
			}
			else { // object type + function + result type | object type + function + paramlist
				L.checktype(2, LUA_TFUNCTION);
				object_type = auto_get<Type>(L, 1);
				if (L.testudata(3, type_name<Type>().Data())) // object type + function + result type
					result_type = auto_get<Type>(L, 3);
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
			object_type = auto_get<Type>(L, 1);
			result_type = auto_get<Type>(L, 3);
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
				LuaFuncToMethodPtrFunc(
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

	template<auto funcptr, typename Obj, std::size_t... Ns>
	void caller(LuaStateView L, Obj* obj, std::index_sequence<Ns...>) {
		using FuncObj = decltype(funcptr);
		using Traits = FuncTraits<FuncObj>;
		using ArgList = typename Traits::ArgList;
		using Ret = typename Traits::Return;
		if constexpr (std::is_void_v<Ret>)
			(obj->*funcptr)(auto_get<At_t<ArgList, Ns>>(L, 2 + Ns)...);
		else
			push<Ret>(L, (obj->*funcptr)(auto_get<At_t<ArgList, Ns>>(L, 2 + Ns)...));
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

				auto* obj = auto_get_userdata_ptr<Obj>(L, 1);
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
				return L.error("%s::%s : Call Ubpa::UDRefl::ObjectView::MInvoke (%s) failed.",
					type_name<Functor>().Data(),
					MetaName::Data(),
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

template<typename T, typename ObjectMeta, int LArgNum = -1, typename Ret = Ubpa::details::Invalid>
static int f_ReflMngr_meta(lua_State* L_) {
	LuaStateView L{ L_ };
	const int callargnum = L.gettop();
	auto obj = details::auto_get<UDRefl::ObjectView>(L, 1);
	L.rotate(1, -1);
	L.pop(1);
	L.pushcfunction(f_meta<UDRefl::ObjectView, ObjectMeta, LArgNum, Ret>);
	details::push(L, UDRefl::ObjectView{ UDRefl::MngrView });
	details::push(L, UDRefl::ObjectView{ Type_of<UDRefl::ObjectView>, &obj });
	L.rotate(1, 3);
	int n = L.gettop();
	int error = L.pcall(callargnum + 1, 1, 0);
	if (error) {
		return L.error("%s::%s: Call f_meta failed.\n%s",
			type_name<T>().Data(), ObjectMeta::Data(), L.tostring(-1));
	}
	return 1;
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

static int f_ObjectView_unbox(lua_State* L_) {
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
	default:
		return L.error("%s::__unbox : The type (%s) can't unbox.",
			type_name<UDRefl::ObjectView>().Data(),
			ptr.GetType().GetName().data());
		break;
	}
	
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

	if (L.getmetatable(1) && L.getfield(-1, key.GetView().data()))
		return 1; // the field is already on the stack, so return directly

	if (UDRefl::Mngr.ContainsMethod(Type_of<UDRefl::ObjectView>, key)) {
		auto* buffer = L.newuserdata(sizeof(details::CallHandle));
		new(buffer)details::CallHandle{ Type_of<UDRefl::ObjectView>, key };
		L.getmetatable(type_name<details::CallHandle>().Data());
		L.setmetatable(-2);
	}
	else if (UDRefl::Mngr.ContainsMethod(Type_of<UDRefl::SharedObject>, key)) {
		auto* buffer = L.newuserdata(sizeof(details::CallHandle));
		new(buffer)details::CallHandle{ Type_of<UDRefl::SharedObject>, key };
		L.getmetatable(type_name<details::CallHandle>().Data());
		L.setmetatable(-2);
	}
	else {
		auto ptr = details::auto_get<UDRefl::ObjectView>(L, 1);
		if (!ptr.GetType().Valid())
			return L.error("%s::__index : the type of object is invalid.", type_name<UDRefl::ObjectView>().Data());

		if (auto key_obj = ptr.Var(key); key_obj.GetPtr()) {
			auto* buffer = L.newuserdata(sizeof(UDRefl::ObjectView));
			new(buffer)UDRefl::ObjectView{ key_obj };
			L.getmetatable(type_name<UDRefl::ObjectView>().Data());
			L.setmetatable(-2);
		}
		else if (UDRefl::Mngr.ContainsMethod(ptr.GetType(), key)) {
			auto* buffer = L.newuserdata(sizeof(details::CallHandle));
			new(buffer)details::CallHandle{ ptr.GetType(), key };
			L.getmetatable(type_name<details::CallHandle>().Data());
			L.setmetatable(-2);
		}
		else if (key.Is("self")) {
			L.pushvalue(1);
			return 1;
		}
		else {
			return L.error("%s::__index : %s index %s failed.",
				type_name<UDRefl::ObjectView>().Data(),
				ptr.GetType().GetName().data(),
				key.GetView().data());
		}
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

static int f_SharedObject_box(lua_State* L_) {
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

static int f_UDRefl_RegisterType(lua_State* L_) {
	LuaStateView L{ L_ };
	if (L.gettop() != 1)
		return L.error("UDRefl::RegisterType : The number of arguments must be 1");
	L.checktype(1, LUA_TTABLE);

	Type type;

	std::vector<Type> bases;

	std::vector<Name> field_names;
	std::vector<Type> field_types;

	std::vector<Name> methodnames;
	std::vector<UDRefl::MethodPtr> methodptrs;

	{ // name
		L.getfield(1, "type");
		type = details::auto_get<Type>(L, -1);
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
		// L.pop(1); // pop bases
	} while (false);
	do { // fields
		auto type = L.getfield(1, "fields");
		if (type == LUA_TNIL)
			break;

		if (type != LUA_TTABLE)
			return L.error("UDRefl::RegisterType : table's fields must be a table");

		lua_Integer len = L.lenL(-1);
		field_types.reserve(len);
		field_names.reserve(len);
		for (lua_Integer i = 1; i <= len; i++) {
			if (L.geti(-1, i) != LUA_TTABLE)
				return L.error("UDRefl::RegisterType : element of table's fields must be a table");
			L.getfield(-1, "type");
			field_types.push_back(details::auto_get<Type>(L, -1));
			L.getfield(-2, "name");
			field_names.push_back(details::auto_get<Name>(L, -1));
			L.pop(3); // table, type, name
		}
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
		methodnames.reserve(mlen);
		methodptrs.reserve(mlen);
		for (lua_Integer i = 1; i <= mlen; i++) {
			if (L.geti(-1, i) != LUA_TTABLE)
				return L.error("UDRefl::RegisterType : element of table's methods must be a table");

			L.getfield(-1, "name");
			Name methodname = details::auto_get<Name>(L, -1);
			if (methodname == UDRefl::NameIDRegistry::Meta::ctor)
				contains_ctor = true;
			else if (methodname == UDRefl::NameIDRegistry::Meta::dtor)
				contains_dtor = true;

			methodnames.push_back(methodname);

			UDRefl::MethodFlag flag;
			if (L.getfield(-2, "flag") != LUA_TNIL) {
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
			if (L.getfield(-3, "result") != LUA_TNIL)
				result_type = details::auto_get<Type>(L, -1);
			else
				result_type = Type_of<void>;

			if (L.getfield(-4, "body") != LUA_TFUNCTION)
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
			L.pop(5);
		}
	} while (false);
	Type rst = UDRefl::Mngr.RegisterType(type, bases, field_types, field_names, !contains_ctor && !contains_dtor);
	if(!rst)
		return L.error("UDRefl::RegisterType : Call UDRefl::ReflMngr::RegisterType failed.");
	for (std::size_t i = 0; i < methodnames.size(); i++) {
		Name mrst = UDRefl::Mngr.AddMethod(type, methodnames[i], UDRefl::MethodInfo{ std::move(methodptrs[i]) });
		if (!mrst) {
			UDRefl::Mngr.typeinfos.erase(rst);
			return L.error("UDRefl::RegisterType : Call UDRefl::ReflMngr::AddMethod for %s failed.", methodnames[i].GetView().data());
		}
	}
	if (!contains_ctor)
		UDRefl::Mngr.AddTrivialCopyConstructor(type);
	if (!contains_dtor)
		UDRefl::Mngr.AddDestructor(type);
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
	"unbox", f_ObjectView_unbox,
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
	"__unbox", f_ObjectView_unbox,
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
	"box", f_SharedObject_box,
	"new_MethodPtr", details::f_SharedObject_new_MethodPtr,
	NULL, NULL
};

static const struct luaL_Reg lib_UDRefl[] = {
	"RegisterType", f_UDRefl_RegisterType,
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
		UDRefl::ext::Bootstrap();
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

void luaopen_UDRefl_libs(lua_State* L_) {
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
