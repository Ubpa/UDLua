#include <ULuaW/ULuaW.h>
#include <UDLua/UDLua.h>
#include <UDRefl/UDRefl.h>

#include <iostream>
#include <cmath>

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

	template<typename Out>
	friend Out& operator<<(Out& o, const vec& v) {
		o << v.x << " " << v.y;
		return o;
	}
};

int main() {
	UDRefl::Mngr->RegisterType<vec>();
	UDRefl::Mngr->AddConstructor<vec, float, float>();
	UDRefl::Mngr->AddField<&vec::x>("x");
	UDRefl::Mngr->AddField<&vec::y>("y");
	UDRefl::Mngr->AddMethod<&vec::norm>("norm");
	UDRefl::Mngr->AddMethod<&vec::offset>("offset");

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
