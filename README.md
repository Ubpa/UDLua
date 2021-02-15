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

Ubpa Lua base on [UDRefl](https://github.com/ubpa/udrefl) 

# How to use

> the example code is [here](https://github.com/Ubpa/UDRefl/blob/master/src/test/00_readme/main.cpp), other examples is [here](https://github.com/Ubpa/UDRefl#other-example)

Suppose you need to use `struct Vec`

```c++
struct vec {
  float x;
  float y;
  float norm2() const { return x * x + y * y; }
};
```

## Manual registration

- do it on C++ [->](https://github.com/ubpa/udrefl#manual-registration) 

- do it on lua: `vec = require("vec")` ([vec.lua](bin/vec.lua))

### Iterate over members

```lua
for field_iter in ObjectView.ReflMngr:GetFields(vec):range() do
    local field = field_iter:__deref()
    print(field.name:GetView())
end

for method_iter in ObjectView.ReflMngr:GetMethods(vec):range() do
    local method = method_iter:__deref()
    print(method.name:GetView())
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
print("x: " .. tostring(v.x))
```

### Invoke Methods

```lua
print("norm2: " .. tostring(v:norm2()))
```

### Iterate over varables

TODO

## Compiler compatibility

- Clang/LLVM >= 10.0
- GCC >= 10.0
- MSVC >= 1926

