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

for field_iter in ObjectView.ReflMngr:GetFields(vec):__range() do
    print(field_iter:__indirection().name:GetView())
end

local v = SharedObject.new(vec)
v.x = 3
v.y = 4

for iter in v:GetTypeFieldVars():__range() do
    local type, field, var = iter:__indirection():__tuple_bind()
    print(field.name:GetView() .. ": ".. var)
end

-- clear
v = nil
collectgarbage()
ObjectView.ReflMngr.typeinfos:erase(vec)
