#include <ULuaW/ULuaW.h>
#include <UDLua/UDLua.h>
#include <UDRefl/UDRefl.h>

#include <iostream>
#include <cmath>
#include <array>
#include <map>

using namespace Ubpa;

struct vec {
	float x, y;

	float norm() const {
		return std::sqrt(x * x + y * y);
	}

	vec operator+(const vec& rhs) const {
		return { x + rhs.x, y + rhs.y };
	}

	vec operator-(const vec& rhs) const {
		return { x - rhs.x, y - rhs.y };
	}

	vec operator*(const vec& rhs) const {
		return { x * rhs.x, y * rhs.y };
	}

	vec operator/(const vec& rhs) const {
		return { x / rhs.x, y / rhs.y };
	}

	friend bool operator==(const vec& lhs, const vec& rhs) {
		return lhs.x == rhs.x && lhs.y == rhs.y;
	}

	vec& offset(float v) {
		x += v;
		y += v;
		return *this;
	}

	std::array<float, 2>& to_array() { return *reinterpret_cast<std::array<float, 2>*>(this); }

	template<typename Out>
	friend Out& operator<<(Out& o, const vec& v) {
		o << v.x << " " << v.y;
		return o;
	}
};

int main() {
	std::map<int, int>::iterator t;
	UDRefl::Mngr.RegisterType<vec>();
	UDRefl::Mngr.AddConstructor<vec, float, float>();
	UDRefl::Mngr.AddField<&vec::x>("x");
	UDRefl::Mngr.AddField<&vec::y>("y");
	UDRefl::Mngr.AddMethod<&vec::norm>("norm");
	UDRefl::Mngr.AddMethod<&vec::offset>("offset");
	UDRefl::Mngr.AddMethod<&vec::to_array>("to_array");

	UDRefl::Mngr.RegisterType<std::array<float, 2>>();

	/*
	print(ObjectView.Global:GetType():GetName())
	v = SharedObject.new("vec", 2, 3)
	v2 = v + v
	v2:offset(2)
	arr = v2:to_array()
	iter = arr:cbegin()
	while iter ~= arr:cend() do print(iter:__deref()); iter:__pre_inc(); end
	for iter in arr:range() do print(iter:__deref()); end
	*/

	char buffer[256];
	int error;
	LuaState L;
	L.openlibs();
	luaopen_UDRefl_libs(L);
	while (std::fgets(buffer, sizeof(buffer), stdin) != nullptr) {
		error = L.loadstring(buffer) || L.pcall(0, 0, 0);
		if (error) {
			std::cerr << L.tostring(-1) << std::endl;
			L.pop(1);
		}
	}
	return 0;
}
