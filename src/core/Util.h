#pragma

#include <ULuaW/ULuaW.h>
#include <UDRefl/UDRefl.h>
#include <UTemplate/TStr.h>

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

	//void stack_dump(lua_State* L);

	static constexpr const char UnsyncRsrc[] = "UDRefl::UnsyncRsrc";

	class LuaStackPopGuard {
	public:
		LuaStackPopGuard(LuaStateView L, int n) : L{ L }, n{ n } {}
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

	UDRefl::ObjectView* safe_get_Object(LuaStateView L, int idx);

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
	int FillArgStack(LuaStateView L, ArgStack& stack, int begin, int cnt);

	UDRefl::MethodPtr::Func LuaFuncToMethodPtrFunc(Type object_type, UDRefl::MethodFlag flag, Type result_type, LuaRef func_ref);

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
