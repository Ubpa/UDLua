```
 __    __   _______   __       __    __       ___      
|  |  |  | |       \ |  |     |  |  |  |     /   \     
|  |  |  | |  .--.  ||  |     |  |  |  |    /  ^  \    
|  |  |  | |  |  |  ||  |     |  |  |  |   /  /_\  \   
|  `--'  | |  '--'  ||  `----.|  `--'  |  /  _____  \  
 \______/  |_______/ |_______| \______/  /__/     \__\ 
```

[![repo-size](https://img.shields.io/github/languages/code-size/Ubpa/UDLua?style=flat)](https://github.com/Ubpa/UDRefl/archive/master.zip) [![tag](https://img.shields.io/github/v/tag/Ubpa/UDLua)](https://github.com/Ubpa/UDRefl/tags) [![license](https://img.shields.io/github/license/Ubpa/UDLua)](LICENSE) 

⭐ Star us on GitHub — it helps!

# UDLua

Ubpa Lua base on [UDRefl](https://github.com/ubpa/udrefl) (the "best" C++20 dynamic reflection library), which make it easy to use C++ in Lua.

## Documents

- [todo](doc/todo.md) 

## How to use

> the example code is [here](bin/README.lua) 

Suppose you need to use `struct vec` 

```c++
struct vec {
  float x;
  float y;
  float norm() const { return std::sqrt(x * x + y * y); }
};
```

### Manual registration on Lua

> if you want to do it on C++, check out this link [->](https://github.com/ubpa/udrefl#manual-registration).

```lua
local vec = UDRefl.RegisterType({
  type = "vec",
  fields = {
    { type = "float32", name = "x" },
    { type = "float32", name = "y" },
  },
  methods = {
    {
      name = "norm",
      result = "float32",
      body = function (p)
        return math.sqrt(UDRefl.unbox(p.x * p.x + p.y * p.y))
      end
    }
  }
})
```

### Iterate over members

```lua
for field_iter in ObjectView.ReflMngr:GetFields(vec):__range() do
  print(field_iter:__indirection().name:GetView())
end

for method_iter in ObjectView.ReflMngr:GetMethods(vec):__range() do
  print(method_iter:__indirection().name:GetView())
end
```

### Constructing types

```lua
v = SharedObject.new(vec)
print(v:GetType():GetName()) -- prints "vec"
```

### Set/get variables

```lua
v.x = 3
v.y = 4
print("x: " .. v.x) -- prints "3"
```

### Invoke Methods

```lua
print("norm: " .. v:norm()) -- prints "5"
```

### Iterate over varables

```lua
for iter in v:GetTypeFieldVars():__range() do
  local type, field, var = iter:__indirection():__tuple_bind()
  print(field.name:GetView() .. ": ".. var)
end
```

## Compiler compatibility

- Clang/LLVM >= 10.0
- GCC >= 10.0
- MSVC >= 1926

> Tested platforms:
>
> - Windows 10: VS2019 16.8.5
>
> - Ubuntu 20: GCC 10.2, Clang 11.0
>
> - MacOS 11.0 : GCC 10.2
>
>   > AppleClang 12 and Clang 11 is not supported

