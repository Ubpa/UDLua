-- Manual registration
local A = UDRefl.RegisterType({
    type = "A",
    fields = {
        { type = "float32", name = "a" }
    }
})
local B = UDRefl.RegisterType({
    type = "B",
    fields = {
        { type = "float32", name = "b" }
    }
})
local C = UDRefl.RegisterType({
    type = "C",
    bases = { "A", "B" },
    fields = {
        { type = "float32", name = "c" }
    }
})

-- Iterate over members
for field_iter in ObjectView.ReflMngr:GetFields(C):__range() do
    print(field_iter:__indirection().name:GetView())
end

for method_iter in ObjectView.ReflMngr:GetMethods(C):__range() do
    print(method_iter:__indirection().name:GetView())
end

-- Constructing types
local v = SharedObject.new(C)
print(v:GetType():GetName()) -- prints "C"

-- Set/get variables
v.a = 3
v.b = 4
v.c = 5
print(v.a,v.b,v.c)

-- Iterate over variables
for iter in v:GetTypeFieldVars():__range() do
    local type, field, var = iter:__indirection():__tuple_bind()
    print(field.name:GetView() .. ": ".. var)
end

-- clear
collectgarbage()
ObjectView.ReflMngr.typeinfos:erase(C)
ObjectView.ReflMngr.typeinfos:erase(B)
ObjectView.ReflMngr.typeinfos:erase(A)
