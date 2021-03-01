-- Manual registration
local vec = require("vec")

-- Iterate over members
for field_iter in ObjectView.ReflMngr:GetFields(vec):__range() do
    print(field_iter:__indirection().name:GetView())
end

for method_iter in ObjectView.ReflMngr:GetMethods(vec):__range() do
    print(method_iter:__indirection().name:GetView())
end

-- Constructing types
local v = SharedObject.new(vec)
print(v:GetType():GetName()) -- prints "vec"

-- Set/get variables
v.x = 3
v.y = 4
print("x: " .. v.x)

-- Invoke Methods
print("norm2: " .. v:norm2())

-- Iterate over variables
for iter in v:GetTypeFieldVars():__range() do
    local type, field, var = iter:__indirection():__tuple_bind()
    print(field.name:GetView() .. ": ".. var)
end

-- clear
collectgarbage()
ObjectView.ReflMngr.typeinfos:erase(vec)
