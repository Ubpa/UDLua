-- Manual registration
local A = UDRefl.RegisterType({
    type = "A",
    fields = {
        { type = "float32", name = "a" }
    }
})
local B = UDRefl.RegisterType({
    type = "B",
    bases = { "A" },
    fields = {
        { type = "float32", name = "b" }
    }
})
local C = UDRefl.RegisterType({
    type = "C",
    bases = { "A" },
    fields = {
        { type = "float32", name = "c" }
    }
})
local D = UDRefl.RegisterType({
    type = "D",
    bases = { "B", "C" },
    fields = {
        { type = "float32", name = "d" }
    }
})

local v = SharedObject.new(D)

v:Var(B, "a").self = 1
v:Var(C, "a").self = 2
v.b = 3
v.c = 4
v.d = 5

for iter in v:GetTypeFieldVars():__range() do
    local type, field, var = iter:__indirection():__tuple_bind()
    print(type.type:GetName() .. "::" .. field.name:GetView() .. ": ".. var)
end

-- clear
v = nil
collectgarbage()
ObjectView.ReflMngr.typeinfos:erase(D)
ObjectView.ReflMngr.typeinfos:erase(C)
ObjectView.ReflMngr.typeinfos:erase(B)
ObjectView.ReflMngr.typeinfos:erase(A)
