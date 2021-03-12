-- Manual registration
local vec = UDRefl.RegisterType({
    type = "vec",
    fields = {
        { type = "float32", name = "x" },
        { type = "float32", name = "y" },
    },
    unowned_fields = {
        { type = "int64", name = "dim", init_args = { 2 } }
    }
})

for iter in ObjectView.new(vec):GetMethods():__range() do
    local name, info = iter:__indirection():__tuple_bind()
    print(name:GetView())
end

local v = SharedObject.new(vec)
v.x = 3
v.y = 4

for iter in v:GetVars():__range() do
    local name, var = iter:__indirection():__tuple_bind()
    print(name:GetView() .. ": ".. var)
end

-- clear
v = nil
collectgarbage()
ObjectView.ReflMngr.typeinfos:erase(vec)
